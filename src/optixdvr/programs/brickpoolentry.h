#pragma once

#ifdef __CUDACC__
#include <optix.h>
#include <optix_world.h>
rtTextureSampler<ushort4, 3> pageTableTexture;
rtDeclareVariable(float3, poolSlots, , );
rtDeclareVariable(float3, poolDataRegionSize, , );
rtDeclareVariable(float3, poolSampleRegionSize, , );
#endif

struct PageTableEntry
{
    unsigned short x, y, z, flags;

};
#define PageTableEntryNotPaged 0
#define PageTableEntryPaged 1
#define PageTableEntryPaging 2

inline __device__ void reportCacheMiss(const struct PageTableEntry &entry)
{
    // TODO: This
}