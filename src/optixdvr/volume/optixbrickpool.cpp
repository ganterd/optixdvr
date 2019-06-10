#include "optixbrickpool.hpp"
#include "../optixdvr_instance.hpp"

OptixVolumeBrickPool::OptixVolumeBrickPool()
{
    mContext = &OptixInstance::get()->m_context;
    mPageTableBuffer = nullptr;
}

void OptixVolumeBrickPool::allocatePageTable()
{
    if(mPageTableBuffer)
    {
        mPageTableTexture->destroy();
        mPageTableBuffer->destroy();
    }
    mPageTableBuffer = (*mContext)->createBuffer(
        RT_BUFFER_INPUT,
        RT_FORMAT_UNSIGNED_SHORT4,
        mNumBricks.x, mNumBricks.y, mNumBricks.z
    );

    mPageTableTexture = (*mContext)->createTextureSampler();
    mPageTableTexture->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
    mPageTableTexture->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
    mPageTableTexture->setWrapMode(2, RT_WRAP_CLAMP_TO_EDGE);
    mPageTableTexture->setFilteringModes(
        RT_FILTER_NEAREST,
        RT_FILTER_NEAREST,
        RT_FILTER_NEAREST
    );
    mPageTableTexture->setIndexingMode(RT_TEXTURE_INDEX_ARRAY_INDEX);
    mPageTableTexture->setReadMode(RT_TEXTURE_READ_ELEMENT_TYPE);
    mPageTableTexture->setBuffer(0, 0, mPageTableBuffer);
}

void OptixVolumeBrickPool::allocate()
{
    if(mContext == nullptr)
    {
        std::cerr << "==BrickPool== No context assigned!" << std::endl;
    }

    size_t freeSpace;
    size_t totalSpace;
    size_t memusagepercent = 70;
    cudaMemGetInfo(&freeSpace, &totalSpace);

    size_t memportion = (freeSpace * memusagepercent) / 100;

    const size_t maxmem = 1024UL * 1024UL * 1024UL * 4UL;
    if(memportion > maxmem)
    {
        memportion = maxmem;
    }

    cudaDeviceProp deviceProperties;
    cudaGetDeviceProperties(&deviceProperties, 0);

    //mVolume = volume;
    mBytesPerVoxel = mVolume->bytesPerVoxel;
    size_t voxelSpace = memportion / mBytesPerVoxel;

    // IMPORTANT NOTE:
    // Deep textures (in z) work better than wide and high (x/y) textures
    // See previous texture latency work to understand why (page sizes and
    // layout).
    mDataDimensions.x = 1024;
    mDataDimensions.y = 1024;
    size_t layer = mDataDimensions.x * mDataDimensions.y;
    size_t availableLayers = voxelSpace / layer;
    mDataDimensions.z = min(
        deviceProperties.maxTexture3D[2],
        availableLayers
    );

    mPoolBrickSlots.x = 0;
    mPoolBrickSlots.y = 0;
    mPoolBrickSlots.z = 0;
    mTotalPoolBrickSlots = 0;

    switch(mVolume->dataType)
    {
    case Volume::CHAR:
        mOptixFormat = RT_FORMAT_BYTE;
        break;
    case Volume::UCHAR:
        mOptixFormat = RT_FORMAT_UNSIGNED_BYTE;
        break;
    case Volume::USHORT:
        mOptixFormat = RT_FORMAT_UNSIGNED_SHORT;
        break;
    case Volume::UINT:
        mOptixFormat = RT_FORMAT_UNSIGNED_INT;
        break;
    case Volume::FLOAT:
        mOptixFormat = RT_FORMAT_FLOAT;
        break;
    }

    mOptixBuffer = (*mContext)->createBuffer(
        RT_BUFFER_INPUT,
        mOptixFormat,
        mDataDimensions.x, mDataDimensions.y, mDataDimensions.z
    );

    mTextureSampler = (*mContext)->createTextureSampler();
    mTextureSampler->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
    mTextureSampler->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
    mTextureSampler->setWrapMode(2, RT_WRAP_CLAMP_TO_EDGE);
    mTextureSampler->setFilteringModes(
        RT_FILTER_LINEAR,
        RT_FILTER_LINEAR,
        RT_FILTER_LINEAR
    );
    mTextureSampler->setIndexingMode(RT_TEXTURE_INDEX_ARRAY_INDEX);
    mTextureSampler->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);
    mTextureSampler->setBuffer(0, 0, mOptixBuffer);

    mNextUploadSlot = 0;
};

size_t OptixVolumeBrickPool::upload()
{
    /* Iterate throught the bricks and upload the unpaged ones */
    int uploadedbricks = 0;
    for(int i = 0; i < mBricks.size(); ++i)
    {
        if(mBricks[i].mActive)
        {
            if(!mBricks[i].mPaged)
            {
                uploadBrick(mBricks[i]);
                uploadedbricks++;
            }
        }
    }
    mStats.set("numnewuploadedbricks", uploadedbricks);

    /* If the page table has bee updated, we need to upload it */
    if(uploadedbricks > 0)
    {
        uploadPageTable();
    }

    (*mContext)["volumeTexture"]->set(mTextureSampler);

    return uploadedbricks;
}

void OptixVolumeBrickPool::uploadBrick(VolumeBrick &brick)
{
    if(mNextUploadSlot >= mTotalPoolBrickSlots)
    {
        std::cerr << "==BrickPool== Out of brick upload slots" << std::endl;
        return;
    }

    size_t uploadslot = mNextUploadSlot++;
    vec3size_t uploadbrick;
    uploadbrick.z = uploadslot / (mPoolBrickSlots.x * mPoolBrickSlots.y);
    uploadbrick.y = (uploadslot % (mPoolBrickSlots.x * mPoolBrickSlots.y))
        / mPoolBrickSlots.x;
    uploadbrick.x = uploadslot % mPoolBrickSlots.x;

    char* mappedBuffer = (char*)mOptixBuffer->map(0, RT_BUFFER_MAP_WRITE);
    for(size_t z = 0; z < mActualDataSize.z; ++z)
    {
        for(size_t y = 0; y < mActualDataSize.y; ++y)
        {
            size_t dst_x = uploadbrick.x * mActualDataSize.x;
            size_t dst_y = (y + uploadbrick.y * mActualDataSize.y)
                * mDataDimensions.x;
            size_t dst_z = (z + uploadbrick.z * mActualDataSize.z)
                * mDataDimensions.y
                * mDataDimensions.x;

            size_t src_x = 0;
            size_t src_y = y * brick.mActualDimensions.x;
            size_t src_z = z
                * brick.mActualDimensions.y
                * brick.mActualDimensions.x;

            size_t src_start = mBytesPerVoxel * (src_x + src_y + src_z);
            size_t dst_start = mBytesPerVoxel * (dst_x + dst_y + dst_z);

            memcpy(
                &mappedBuffer[dst_start],
                &brick.mData[src_start],
                mBytesPerVoxel * mActualDataSize.x
            );
        }
    }
    mOptixBuffer->unmap();
    brick.mPaged = true;

    /* Update the page table */
    size_t brickIndex = brick.mBrickIndex.x;
    brickIndex += brick.mBrickIndex.y * mNumBricks.x;
    brickIndex += brick.mBrickIndex.z * mNumBricks.x * mNumBricks.y;
    struct PageTableEntry pageTableEntry;
    pageTableEntry.x = uploadbrick.x;
    pageTableEntry.y = uploadbrick.y;
    pageTableEntry.z = uploadbrick.z;
    pageTableEntry.flags = PageTableEntryPaged;
    mPageTableData[brickIndex] = pageTableEntry;
};

void OptixVolumeBrickPool::uploadPageTable()
{
    /* Upload the current page table to the GPU */
    //std::cout << "==BrickPool== Uploading page table to GPU" << std::endl;
    struct PageTableEntry* map = (struct PageTableEntry*)mPageTableBuffer->map();
    memcpy(map, &mPageTableData[0], mPageTableMemoryUsage);
    mPageTableBuffer->unmap();

    /* Set Optix variables */
    (*mContext)["pageTableTexture"]->set(mPageTableTexture);

    (*mContext)["poolSlots"]->setFloat(
        mPoolBrickSlots.x,
        mPoolBrickSlots.y,
        mPoolBrickSlots.z
    );

    (*mContext)["poolDataRegionSize"]->setFloat(
        (float)mActualDataSize.x,// / (float)mDataDimensions.x,
        (float)mActualDataSize.y,// / (float)mDataDimensions.y,
        (float)mActualDataSize.z// / (float)mDataDimensions.z
    );

    (*mContext)["poolSampleRegionSize"]->setFloat(
        (float)mBrickSize.x,// / (float)mDataDimensions.x,
        (float)mBrickSize.y,// / (float)mDataDimensions.y,
        (float)mBrickSize.z// / (float)mDataDimensions.z
    );
}