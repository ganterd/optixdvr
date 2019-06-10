#include <optix_world.h>
#include "prd.h"
#include "brickpoolentry.h"

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(PerRayData, prd, rtPayload, );

rtDeclareVariable(int, dontSample, , );
rtTextureSampler<float, 3> volumeTexture;
rtTextureSampler<float4, 1> transferFunction;
rtDeclareVariable(float3, brickSizeVolumeSpace, , );
rtDeclareVariable(float3, volumeMin, , );
rtDeclareVariable(float3, volumeSize, , );

inline __device__ void accumulate(
    const optix::Ray &ray_in,
    float entryDistance,
    float exitDistance,
    vec4f &accumulation
){
    const vec3f rayDirection = vec3f(ray.direction);
    const vec3f rayOrigin = vec3f(ray.origin);

    /* Round up and down to the nearest entry/exit samples */
    int startSample = (int)ceilf(entryDistance / prd.in.worldSpaceStepSize);
    int endSample = (int)floorf(exitDistance / prd.in.worldSpaceStepSize);
    exitDistance = (float)endSample * prd.in.worldSpaceStepSize;
    prd.hit.exitDistance = exitDistance + prd.in.worldSpaceStepSize;
    const int steps = endSample - startSample;

    /* Get the world space entry point */
    const vec3f worldSpaceEntry = rayDirection * entryDistance + rayOrigin;

    /* Convert this to volume space */
    const vec3f brickEntryPoint = (worldSpaceEntry - volumeMin) / volumeSize;

    /* Ray-stepping loop */
    vec3f p = brickEntryPoint;
    vec4f a = accumulation;
    const vec3f step = prd.in.volumeSpaceStep;
    const float opacityCorrection = prd.in.opacityCorrection;
    vec3f pageTableIndex;
    vec3f prevPageTableIndex(-1.0f);
    vec3f poolOffset;
    vec3f brickBegin;
    const vec3f brickSizeInv = vec3f(1.0f) / brickSizeVolumeSpace;
    int ptaccesses = 0;
    for(int i = 0; i < steps && a.w < 0.99f; ++i)
    {

        pageTableIndex.x = floorf(p.x * brickSizeInv.x);
        pageTableIndex.y = floorf(p.y * brickSizeInv.y);
        pageTableIndex.z = floorf(p.z * brickSizeInv.z);

        /* If we've moved to a new brick, need to fetch page table info */
        if(pageTableIndex != prevPageTableIndex)
        {
            ushort4 pageTableEntry = tex3D(pageTableTexture, pageTableIndex.x, pageTableIndex.y, pageTableIndex.z);
            if(pageTableEntry.w == PageTableEntryNotPaged)
            {
                p += step;
                continue;
            }
            brickBegin = pageTableIndex * brickSizeVolumeSpace;

            /* For some reason, have to push the offset half a voxel for visual fix? */
            poolOffset.x = (float)pageTableEntry.x * poolDataRegionSize.x + 0.5f;
            poolOffset.y = (float)pageTableEntry.y * poolDataRegionSize.y + 0.5f;
            poolOffset.z = (float)pageTableEntry.z * poolDataRegionSize.z + 0.5f;
            prevPageTableIndex = pageTableIndex;
            ptaccesses++;
        }

        /* Convert p from normalized volume space to pool data space */
        vec3f voxelAddress;
        voxelAddress.x = poolOffset.x + ((p.x - brickBegin.x) * brickSizeInv.x) * poolSampleRegionSize.x;
        voxelAddress.y = poolOffset.y + ((p.y - brickBegin.y) * brickSizeInv.y) * poolSampleRegionSize.y;
        voxelAddress.z = poolOffset.z + ((p.z - brickBegin.z) * brickSizeInv.z) * poolSampleRegionSize.z;

        /* Sample the volume */
        float value = tex3D(volumeTexture, voxelAddress.x, voxelAddress.y, voxelAddress.z);

        /* Tranform from voxel intesity to colour */
        vec4f colour = tex1D(transferFunction, value);

        /* Apply opacity correction and accumulate colour */
        colour.w = 1.0f - powf(1.0f - colour.w, opacityCorrection);
        float opacity = 1.0f - a.w;
        colour.w *= opacity;
        a.x = colour.x * colour.w + a.x;
        a.y = colour.y * colour.w + a.y;
        a.z = colour.z * colour.w + a.z;
        a.w = colour.w + a.w;

        /* Step along the ray */
        p += step;
    }
    prd.out.pageTableAccesses += ptaccesses;

    accumulation = a;
}


/*! optix program for entering a volume region */
RT_PROGRAM void closest_hit()
{
    if(!dontSample)
    {
        accumulate(
            ray,
            prd.hit.entryDistance,
            prd.hit.exitDistance,
            prd.out.accumulation
        );
    }
}