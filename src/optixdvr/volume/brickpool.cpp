
#include "brickpool.hpp"
#include "brickedvolume.hpp"

VolumeBrickPool::VolumeBrickPool(){
    mBrickSize = vec3size_t(32);
    mActualDataSize = vec3size_t(33);
};

void VolumeBrickPool::set_brick_size(
    const vec3size_t &bricksize,
    const vec3size_t &padding
){
    mBrickSize = bricksize;
    mActualDataSize = bricksize + padding;

    mPoolBrickSlots.x = mDataDimensions.x / mActualDataSize.x;
    mPoolBrickSlots.y = mDataDimensions.y / mActualDataSize.y;
    mPoolBrickSlots.z = mDataDimensions.z / mActualDataSize.z;

    mTotalPoolBrickSlots =
        mPoolBrickSlots.x * mPoolBrickSlots.y * mPoolBrickSlots.z;

    mNormalizedRegionSize.x = (float)mActualDataSize.x / (float)mDataDimensions.x;
    mNormalizedRegionSize.y = (float)mActualDataSize.y / (float)mDataDimensions.y;
    mNormalizedRegionSize.z = (float)mActualDataSize.z / (float)mDataDimensions.z;

    mNextUploadSlot = 0;

    if(mVolume == nullptr)
    {
        return;
    }

    if(mBricks.size())
    {
        #pragma omp parallel for collapse(3)
        for(int z = 0; z < (int)mNumBricks.z; ++z)
        {
            for(int y = 0; y < (int)mNumBricks.y; ++y)
            {
                for(int x = 0; x < (int)mNumBricks.x; ++x)
                {
                    brick(x, y, z).free();
                }
            }
        }
    }
    mNumBricks.x = ceilf(mVolume->dataDimensions.x / (float)mBrickSize.x);
    mNumBricks.y = ceilf(mVolume->dataDimensions.y / (float)mBrickSize.y);
    mNumBricks.z = ceilf(mVolume->dataDimensions.z / (float)mBrickSize.z);
    size_t totalNumBricks =
        (size_t)mNumBricks.x *
        (size_t)mNumBricks.y *
        (size_t)mNumBricks.z;

    mBricks.resize(totalNumBricks);
    mStats.set("numbricks", totalNumBricks);

    mPageTableMemoryUsage = mNumBricks.x * mNumBricks.y * mNumBricks.z * sizeof(struct PageTableEntry);
    mStats.set("pagetablememory", mPageTableMemoryUsage);

    mPageTableData.resize(mNumBricks.x * mNumBricks.y * mNumBricks.z);
    for(size_t i = 0; i < mNumBricks.x * mNumBricks.y * mNumBricks.z; ++i)
    {
        struct PageTableEntry pagetableEntry;
        pagetableEntry.x = 0;
        pagetableEntry.y = 0;
        pagetableEntry.z = 0;
        pagetableEntry.flags = PageTableEntryNotPaged;
        mPageTableData[i] = pagetableEntry;
    }

    allocatePageTable();

    utils::Timer timer;
    timer.start();
    #pragma omp parallel for collapse(3)
    for(int z = 0; z < (int)mNumBricks.z; ++z)
    {
        for(int y = 0; y < (int)mNumBricks.y; ++y)
        {
            for(int x = 0; x < (int)mNumBricks.x; ++x)
            {
                VolumeBrick b = pullBrick(x, y, z);
                brick(x, y, z) = b;
            }
        }
    }
    timer.stop();
    mStats.set("loadtime", timer.getTime());
}

void VolumeBrickPool::volume(Volume *v)
{
    mVolume = v;
    allocate();
    mNextUploadSlot = 0;
    set_brick_size(mBrickSize);
}

size_t VolumeBrickPool::testBricks(const TransferFunction& tf)
{
    utils::Timer timer;
    size_t activeBricks = 0;
    size_t changes = 0;

    /* Iterate through all bricks and test against tf */
    timer.start();
    for(int i = 0; i < mNumBricks.x * mNumBricks.y * mNumBricks.z; ++i)
    {
        bool active = tf.rangeActive(mBricks[i].minValue, mBricks[i].maxValue);

        if(active != mBricks[i].mActive)
            changes++;

        mBricks[i].mActive = active;
        activeBricks += active ? 1 : 0;
    }
    timer.stop();
    mStats.set("numactivebricks", activeBricks);
    mStats.set("brickchanges", changes);
    mStats.set("tftesttime", timer.getTime());

    return changes;
}

VolumeBrick VolumeBrickPool::pullBrick(int bx, int by, int bz)
{
    VolumeBrick brick;
    brick.mBrickIndex = vec3size_t(bx, by, bz);
    brick.mDataDimensions = mBrickSize;
    brick.mActualDimensions = brick.mDataDimensions + brick.mPadMin + brick.mPadMax;
    size_t bpv = mVolume->bytesPerVoxel;
    brick.mDataTotal =
        bpv
        * brick.mActualDimensions.x
        * brick.mActualDimensions.y
        * brick.mActualDimensions.z;
    brick.mData = new char[brick.mDataTotal];

    int rowSize = brick.mActualDimensions.x;
    int rowStart = bx * brick.mDataDimensions.x;
    int rowEnd = rowStart + rowSize;
    int volumeLimit = mVolume->dataLimits.x;
    int rowLimit = 0;
    if(volumeLimit <= rowEnd)
    {
        rowLimit = rowEnd - volumeLimit;
    }
    rowSize -= rowLimit;
    size_t brickStride = bpv * (rowSize);
    for(size_t z = 0; z < brick.mActualDimensions.z; ++z)
    {
        for(size_t y = 0; y < brick.mActualDimensions.y; ++y)
        {
            size_t src_x = bx * brick.mDataDimensions.x;
            size_t src_y = by * brick.mDataDimensions.y + y;
            size_t src_z = bz * brick.mDataDimensions.z + z;

            size_t dst_x = 0;
            size_t dst_y = y * brick.mActualDimensions.x;
            size_t dst_z = z * brick.mActualDimensions.x * brick.mActualDimensions.y;
            size_t dst = bpv * (dst_x + dst_y + dst_z);

            vec3f p;
            p.x = src_x;
            p.y = src_y;
            p.z = src_z;
            p = min(p, mVolume->dataDimensions - vec3f(1));

            memcpy(&brick.mData[dst], mVolume->voxeladdress(p), brickStride);

            for(size_t x = 0; x < brick.mActualDimensions.x; ++x)
            {
                p.x = bx * brick.mDataDimensions.x + x;
                float v = mVolume->GetNormalisedVoxel(p);
                brick.minValue = fmin(v, brick.minValue);
                brick.maxValue = fmax(v, brick.maxValue);
            }
        }
    }
    return brick;
}