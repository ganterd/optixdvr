#include "brickedvolume.hpp"

BrickedVolume::BrickedVolume() :
    mVolume(nullptr),
    mNumLeaves(0.0f),
    //mPool(nullptr),
    mVoxelsPerBrick(16)
{ }

void BrickedVolume::reset()
{
}

void BrickedVolume::set_volume(Volume* volume)
{
    mVolume = volume;
    set_brick_size(mVoxelsPerBrick);
}

void BrickedVolume::set_brick_size(const vec3size_t& bricksize)
{
    // Clear current resources before
    utils::Timer timer;
    timer.start();


    mVoxelsPerBrick.x = bricksize.x;
    mVoxelsPerBrick.y = bricksize.y;
    mVoxelsPerBrick.z = bricksize.z;
    if(mVolume)
    {
        mNumLeaves.x = ceilf(mVolume->dataDimensions.x / (float)bricksize.x);
        mNumLeaves.y = ceilf(mVolume->dataDimensions.y / (float)bricksize.y);
        mNumLeaves.z = ceilf(mVolume->dataDimensions.z / (float)bricksize.z);

        m_total_subdivisions =
            (size_t)mNumLeaves.x *
            (size_t)mNumLeaves.y *
            (size_t)mNumLeaves.z;
        mLeaves.resize(m_total_subdivisions);
        mStats.set("numbricks", m_total_subdivisions);

        //std::cout << "==BrickedVolume== Pulling brick data... ";
        #pragma omp parallel for collapse(3)
        for(int z = 0; z < (int)mNumLeaves.z; ++z)
        {
            for(int y = 0; y < (int)mNumLeaves.y; ++y)
            {
                for(int x = 0; x < (int)mNumLeaves.x; ++x)
                {
                    AccelerationLeaf b = scanBrick(mVolume, x, y, z);
                    leaf(x, y, z) = b;
                }
            }
        }

        //mPool->set_brick_size(bricksize, mSubdivisions);
    }
    timer.stop();
    mStats.set("subdivisiontime", timer.getTime());
}

size_t BrickedVolume::testbricks(const TransferFunction& tf)
{
    utils::Timer timer;
    m_active_subdivisions = 0;
    size_t changes = 0;

    /* Iterate through all bricks and test against tf */
    timer.start();
    for(int i = 0; i < m_total_subdivisions; ++i)
    {
        bool active = tf.rangeActive(mLeaves[i].mMinTFValue, mLeaves[i].mMaxTFValue);

        if(active != mLeaves[i].mActive)
            changes++;

        mLeaves[i].mActive = active;
        m_active_subdivisions += active ? 1 : 0;
    }
    timer.stop();
    mStats.set("numactivebricks", m_active_subdivisions);
    mStats.set("brickchanges", changes);
    mStats.set("tftesttime", timer.getTime());

    /* If clustering is enabled, run clustering */
    if(mCluster)
    {
        timer.start();
        cluster();
        timer.stop();
        mStats.set("clusteringtime", timer.getTime());
    }

    return changes;
}

void BrickedVolume::cluster()
{
    /* Create an active bricks table which we use to keep track of */
    /* yet unclustered bricks.                                     */
    std::vector<bool> activeBricks(m_total_subdivisions, false);
    for(int i = 0; i < m_total_subdivisions; ++i)
    {
        activeBricks[i] = mLeaves[i].mActive;
    }

    /* Cast to size_t vector */
    vec3size_t subdivisions = mNumLeaves;

    /* Generate list of cluster sizes in descending order of size */
    std::vector<int> clustersizes;
    int sizestride = 1;
    for(int i = 64; i > 1; i = i - sizestride)
    {
        if(vec3size_t(i) > subdivisions)
            continue;
        else
            clustersizes.push_back(i);
    }

    /* Always cluster with size 1, i.e: single brick */
    clustersizes.push_back(1);

    /* Begin the (very very unoptimised) clustering */
    mClusters.clear();
    for(int clusterSize : clustersizes)
    {
        if(vec3size_t(clusterSize) > subdivisions)
            continue;

        size_t numClusters = 0;
        std::vector<vec3size_t> clusters;
        for(size_t z = 0; z < subdivisions.z - clusterSize; ++z)
        {
            for(size_t y = 0; y < subdivisions.y - clusterSize; ++y)
            {
                for(size_t x = 0; x < subdivisions.x - clusterSize; ++x)
                {
                    bool cancluster = true;
                    for(size_t cz = z; (cz < (z + clusterSize)) && cancluster; ++cz)
                    {
                        for(size_t cy = y; (cy < (y + clusterSize)) && cancluster; ++cy)
                        {
                            for(size_t cx = x; (cx < (x + clusterSize)) && cancluster; ++cx)
                            {
                                cancluster = activeBricks[cx + subdivisions.x * cy + subdivisions.x * subdivisions.y * cz];
                            }
                        }
                    }
                    if(cancluster)
                    {
                        struct Cluster c;
                        c.mStart = vec3f(x, y, z);
                        c.mSize = vec3f(clusterSize);
                        mClusters.push_back(c);
                        numClusters++;
                        for(size_t cz = z; (cz < (z + clusterSize)) && cancluster; ++cz)
                        {
                            for(size_t cy = y; (cy < (y + clusterSize)) && cancluster; ++cy)
                            {
                                for(size_t cx = x; (cx < (x + clusterSize)) && cancluster; ++cx)
                                {
                                    activeBricks[cx + subdivisions.x * cy + subdivisions.x * subdivisions.y * cz] = false;
                                }
                            }
                        }
                    }
                }
            }
        }
        std::stringstream ss;
        ss << "numclusters_" << clusterSize;
        mStats.set(ss.str(), numClusters);
    }
    mStats.set("clustersizes", clustersizes.size());
    mStats.set("numclusters", mClusters.size());
}

AccelerationLeaf BrickedVolume::scanBrick(Volume* volume, int bx, int by, int bz)
{
    AccelerationLeaf brick;
    vec3size_t padMin(0);
    vec3size_t padMax(1);
    vec3size_t actualDimensions = mVoxelsPerBrick + padMin + padMax;

    int rowSize = actualDimensions.x;
    int rowStart = bx * mVoxelsPerBrick.x;
    int rowEnd = rowStart + rowSize;
    int volumeLimit = volume->dataLimits.x;
    int rowLimit = 0;
    if(volumeLimit <= rowEnd)
    {
        rowLimit = rowEnd - volumeLimit;
    }
    rowSize -= rowLimit;
    for(size_t z = 0; z < actualDimensions.z; ++z)
    {
        for(size_t y = 0; y < actualDimensions.y; ++y)
        {
            for(size_t x = 0; x < actualDimensions.x; ++x)
            {
                vec3f p;
                p.x = bx * mVoxelsPerBrick.x + x;
                p.y = by * mVoxelsPerBrick.y + y;
                p.z = bz * mVoxelsPerBrick.z + z;
                p = min(p, volume->dataDimensions - vec3f(1));

                float v = volume->GetNormalisedVoxel(p);
                brick.mMinTFValue = fmin(v, brick.mMinTFValue);
                brick.mMaxTFValue = fmax(v, brick.mMaxTFValue);
            }
        }
    }
    return brick;
}