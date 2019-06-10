#pragma once

#include <limits>
#include <iomanip>
#include "volume.hpp"
#include "transferfunction.hpp"
#include "brickpool.hpp"
#include "../utils/stats.hpp"

struct AccelerationLeaf
{
public:
    float mMinTFValue = +std::numeric_limits<float>::infinity();
    float mMaxTFValue = -std::numeric_limits<float>::infinity();
    bool mActive = false;
};

class BrickedVolume
{
public:
    Stats mStats;
    //std::vector<VolumeBrick> mBricks;
    std::vector<AccelerationLeaf> mLeaves;
    //vec3f mSubdivisions;
    vec3f mNumLeaves;
    vec3f mVoxelsPerBrick;
    //VolumeBrickPool* mPool;
    Volume* mVolume;

    struct Cluster
    {
        vec3f mStart;
        vec3f mSize;
    };
    bool mCluster = false;
    std::vector<struct Cluster> mClusters;

    size_t mTotalLeaves = 0;
    size_t mTotalActiveLeaves = 0;
    size_t m_total_subdivisions = 0;
    size_t m_active_subdivisions = 0;

    BrickedVolume();

    void reset();
    void set_volume(Volume* volume);
    virtual void set_brick_size(const vec3size_t& bricksize);
    vec3size_t get_brick_size(){ return mVoxelsPerBrick; };

    virtual size_t testbricks(const TransferFunction& tf);
    void cluster();

    inline AccelerationLeaf& leaf(int x, int y, int z)
    {
        return mLeaves[x + (int)mNumLeaves.x * y + (int)mNumLeaves.x * (int)mNumLeaves.y * z];
    }

    inline bool leafActive(int x, int y, int z)
    {
        return leaf(x, y, z).mActive;
    }

private:
    AccelerationLeaf scanBrick(Volume* volume, int bx, int by, int bz);
};