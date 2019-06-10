#pragma once

#include <limits>
#include <iomanip>
#include "volume.hpp"
#include "transferfunction.hpp"
#include "../programs/brickpoolentry.h"
#include "../utils/stats.hpp"

class VolumeBrick
{
public:
    char* mData = nullptr;
    RTformat mOptixFormat;
    size_t mDataTotal;
    float minValue;
    float maxValue;
    bool mActive = false;
    bool mPaged = false;
    vec3size_t mBrickIndex;
    vec3size_t mDataDimensions;
    vec3size_t mActualDimensions;
    vec3size_t mPadMin;
    vec3size_t mPadMax;

    VolumeBrick()
    {
        minValue = +std::numeric_limits<float>::infinity();
        maxValue = -std::numeric_limits<float>::infinity();

        mPadMin = vec3size_t(0);
        mPadMax = vec3size_t(1);
    }

    void free()
    {
        if(mData)
            delete[] mData;
    }
};

class VolumeBrickPool
{
public:
    Stats mStats;

    std::vector<VolumeBrick> mBricks;

    vec3size_t mDataDimensions;
    vec3size_t mPoolBrickSlots;
    vec3size_t mBrickSize;
    vec3size_t mNumBricks;
    vec3size_t mActualDataSize;
    vec3f mNormalizedRegionSize;
    size_t mBytesPerVoxel;
    size_t mTotalPoolBrickSlots;
    size_t mNextUploadSlot;
    Volume* mVolume = nullptr;

    size_t mPageTableMemoryUsage = 0;
    std::vector<struct PageTableEntry> mPageTableData;

    VolumeBrickPool();

    void volume(Volume *v);
    void set_brick_size(const vec3size_t &bricksize, const vec3size_t &padding = vec3size_t(1));
    virtual void allocate() = 0;

    /**
     * Upload brick data to the next available slot in the GPU
     * texture. Note that this assumes data is the same size
     * and data type provided in constructor.
     */

    inline VolumeBrick& brick(int x, int y, int z)
    {
        return mBricks[x + (int)mNumBricks.x * y + (int)mNumBricks.x * (int)mNumBricks.y * z];
    }

    VolumeBrick pullBrick(int bx, int by, int bz);

    size_t testBricks(const TransferFunction &tf);

    virtual size_t upload() = 0;
    virtual void allocatePageTable() = 0;
    virtual void uploadPageTable() = 0;
};