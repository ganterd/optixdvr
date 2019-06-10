#pragma once

#include <cuda.h>
#include <optix.h>
#include <optixu/optixpp.h>

#include "brickpool.hpp"

class OptixVolumeBrickPool : public VolumeBrickPool
{
public:
    RTformat mOptixFormat;
    optix::Context* mContext = nullptr;
    optix::Buffer mOptixBuffer;
    optix::TextureSampler mTextureSampler;
    optix::Buffer mPageTableBuffer;
    optix::TextureSampler mPageTableTexture;

    OptixVolumeBrickPool();

    void allocate();
    void uploadBrick(VolumeBrick &brick);
    size_t upload();
    void allocatePageTable();
    void uploadPageTable();
};