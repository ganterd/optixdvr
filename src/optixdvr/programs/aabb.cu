#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>
#include "prd.h"

rtBuffer<float4> aabbMinBuffer;
rtBuffer<float4> aabbMaxBuffer;
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(PerRayData, prd, rtPayload, );

RT_PROGRAM void get_aabb_bounds(int pid, float result[6])
{
    result[0] = aabbMinBuffer[pid].x;
    result[1] = aabbMinBuffer[pid].y;
    result[2] = aabbMinBuffer[pid].z;
    result[3] = aabbMaxBuffer[pid].x;
    result[4] = aabbMaxBuffer[pid].y;
    result[5] = aabbMaxBuffer[pid].z;
}

RT_PROGRAM void hit_aabb(int pid)
{
    float3 aabbMin = make_float3(aabbMinBuffer[pid].x, aabbMinBuffer[pid].y, aabbMinBuffer[pid].z);
    float3 aabbMax = make_float3(aabbMaxBuffer[pid].x, aabbMaxBuffer[pid].y, aabbMaxBuffer[pid].z);
    float3 vminv = (aabbMin - ray.origin) * prd.in.rayDirectionInverse;
    float3 vmaxv = (aabbMax - ray.origin) * prd.in.rayDirectionInverse;
    float3 tnear = fminf(vminv, vmaxv);
    float3 tfar = fmaxf(vminv, vmaxv);
    float tmin = fmaxf(tnear);
    float tmax = fminf(tfar);

    tmin = fmaxf(ray.tmin, tmin);
    if(tmax > tmin)
    {
        if(rtPotentialIntersection(tmin))
        {
            prd.hit.entryDistance = tmin;
            prd.hit.exitDistance = tmax;
            rtReportIntersection(0);
        }
    }
}