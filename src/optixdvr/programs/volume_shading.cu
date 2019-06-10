#include <optix_world.h>
#include "prd.h"

/*! the parameters that describe each individual sphere geometry */
// rtDeclareVariable(float3, boxMin, , );
// rtDeclareVariable(float3, boxMax, , );
rtDeclareVariable(float3, poolRegionOffset, , );
rtDeclareVariable(float3, poolRegionSize, , );

/*! the implicit state's ray we will intersect against */
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

/*! the attributes we use to communicate between intersection programs and hit program */
rtDeclareVariable(float3, hit_rec_normal, attribute hit_rec_normal, );
rtDeclareVariable(float3, hit_rec_p, attribute hit_rec_p, );

/*! the per ray data we operate on */
rtDeclareVariable(PerRayData, prd, rtPayload, );

rtDeclareVariable(float, ertThreshold, , );
rtTextureSampler<float, 3> volumeTexture;
rtTextureSampler<float4, 1> transferFunction;
rtDeclareVariable(float3, volumeDimensions, ,  );
rtDeclareVariable(float3, lightPositionVolumeSpace, , );
rtDeclareVariable(float3, gradientStep, , );


inline __device__ void accumulate(
    const optix::Ray &ray_in,
    float entryDistance,
    float exitDistance,
    vec4f &accumulation)
{
    // const vec3f rayDirection = vec3f(ray_in.direction);
    // const vec3f rayOrigin = vec3f(ray_in.origin);
    // entryDistance = fmaxf(prd.in.worldSpaceStepSize, entryDistance);
    // exitDistance = fmaxf(prd.in.worldSpaceStepSize, exitDistance);
    // const float depth = exitDistance - entryDistance;

    // vec3f worldSpaceEntry;
    // worldSpaceEntry.x = rayDirection.x * entryDistance + rayOrigin.x;
    // worldSpaceEntry.y = rayDirection.y * entryDistance + rayOrigin.y;
    // worldSpaceEntry.z = rayDirection.z * entryDistance + rayOrigin.z;

    // const int steps = depth / prd.in.worldSpaceStepSize;
    // const vec3f brickEntryPoint = (worldSpaceEntry - prd.hit.boxMin) / (prd.hit.boxMax - prd.hit.boxMin);
    // const vec3f step = prd.in.poolSpaceStep;

    // /* Ray-stepping loop */
    // vec3f p;
    // p.x = brickEntryPoint.x * poolRegionSize.x + poolRegionOffset.x;
    // p.y = brickEntryPoint.y * poolRegionSize.y + poolRegionOffset.y;
    // p.z = brickEntryPoint.z * poolRegionSize.z + poolRegionOffset.z;

    // vec4f a = accumulation;
    // const float opacityCorrection = prd.in.opacityCorrection;
    // for(int i = 0; i < steps && a.w < ertThreshold; ++i)
    // {
    //     float value = tex3D(volumeTexture, p.x, p.y, p.z);
    //     vec4f colour = tex1D(transferFunction, value);

    //     if(colour.w > 0.0f)
    //     {
    //         vec3f normal;
    //         normal.x = tex3D(volumeTexture, p.x + gradientStep.x, p.y, p.z)
    //                 - tex3D(volumeTexture, p.x - gradientStep.x, p.y, p.z);
    //         normal.y = tex3D(volumeTexture, p.x, p.y + gradientStep.y, p.z)
    //                 - tex3D(volumeTexture, p.x, p.y - gradientStep.y, p.z);
    //         normal.z = tex3D(volumeTexture, p.x, p.y, p.z + gradientStep.z)
    //                 - tex3D(volumeTexture, p.x, p.y, p.z - gradientStep.z);
    //         normal = normalize(normal);

    //         vec3f lightPosition(2, 0, 0);
    //         vec3f toLight = normalize(lightPosition - p);
    //         float diff = fmaxf(fminf(dot(toLight, normal), 1.0f), 0.0f);

    //         colour.x *= diff;
    //         colour.y *= diff;
    //         colour.z *= diff;
    //         float opacity = 1.0f - a.w;
    //         a += colour * opacity;
    //     }


    //     p += step;
    // }

    // accumulation = a;
}


/*! optix program for entering a volume region */
RT_PROGRAM void closest_hit()
{
    accumulate(
        ray,
        prd.hit.entryDistance,
        prd.hit.exitDistance,
        prd.out.accumulation
    );
}