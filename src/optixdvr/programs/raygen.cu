// ======================================================================== //
// Copyright 2018 Ingo Wald                                                 //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

// optix code:
#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "prd.h"
#include "sampling.h"

/*! the 'builtin' launch index we need to render a frame */
rtDeclareVariable(uint2, pixelID,   rtLaunchIndex, );
rtDeclareVariable(uint2, launchDim, rtLaunchDim,   );

/*! the ray related state */
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(PerRayData, prd, rtPayload, );

/*! the 2D, float3-type color frame buffer we'll write into */
rtBuffer<uchar4, 2> fb;

rtDeclareVariable(int, numSamples, , );
rtDeclareVariable(int, maxBounces, , );
rtDeclareVariable(float, ertThreshold, , );
rtDeclareVariable(int, highlightERT, , );
rtDeclareVariable(int, showDepthComplexity, , );
rtDeclareVariable(int, showPageTableAccesses, , );

rtDeclareVariable(rtObject, world, , );

rtDeclareVariable(float3, camera_lower_left_corner, , );
rtDeclareVariable(float3, camera_horizontal, , );
rtDeclareVariable(float3, camera_vertical, , );
rtDeclareVariable(float3, camera_origin, , );
rtDeclareVariable(float3, camera_u, , );
rtDeclareVariable(float3, camera_v, , );
rtDeclareVariable(float, camera_lens_radius, , );

rtDeclareVariable(float3, volumeDimensions, , );
rtDeclareVariable(float3, volumeMin, , );
rtDeclareVariable(float3, volumeSize, , );
rtDeclareVariable(float3, poolDimensions, , );

struct Camera {
  static __device__ optix::Ray generateRay(float s, float t, DRand48 &rnd)
  {
    const vec3f rd = camera_lens_radius * random_in_unit_disk(rnd);
    const vec3f lens_offset = camera_u * rd.x + camera_v * rd.y;
    const vec3f origin = camera_origin + lens_offset;
    const vec3f direction
      = camera_lower_left_corner
      + s * camera_horizontal
      + t * camera_vertical
      - origin;
    return optix::make_Ray(origin.as_float3(),
                          normalize(direction).as_float3(), 0, 1e-6f, RT_DEFAULT_MAX);
  }
};

inline __device__ vec4f missColor(const optix::Ray &ray)
{
  //const vec3f unit_direction = normalize(ray.direction);
  //const float t = 0.5f*(unit_direction.y + 1.0f);
  //const vec3f c = (1.0f - t)*vec3f(1.0f, 1.0f, 1.0f) + t * vec3f(0.5f, 0.7f, 1.0f);
  return vec4f(0.0f, 0.0f, 0.0f, 0.0f);
}

bool __device__ entryExitPoints(
  optix::Ray &ray,
  const vec3f& boxmin,
  const vec3f& boxmax,
  float &entryDistance,
  float &exitDistance
){
    const vec3f rayDirectionInverse = 1.0f / ray.direction;
    vec3f vminv = (boxmin - ray.origin) * rayDirectionInverse;
    vec3f vmaxv = (boxmax - ray.origin) * rayDirectionInverse;

    vec3f vmin = min(vminv, vmaxv);
    vec3f vmax = max(vminv, vmaxv);

    float tmin = fmaxf(fmaxf(vmin.x, vmin.y), vmin.z);
    float tmax = fminf(fminf(vmax.x, vmax.y), vmax.z);

    tmin = fmaxf(ray.tmin + 1e-9f, tmin);

    entryDistance = tmin;
    exitDistance = tmax;
    return tmax > tmin;
}

inline __device__ vec4f color(optix::Ray &ray, DRand48 &rnd)
{
  PerRayData prd;

  vec4f accumulatedColour(vec3f(0.0f), 0.0f);
  prd.out.accumulation = accumulatedColour;

  float volumeEntryDistance, volumeExitDistance;
  vec3f boxsize = volumeSize;
  vec3f boxmin = volumeMin;
  vec3f boxmax = volumeMin + volumeSize;
  bool intersect = entryExitPoints(ray, boxmin, boxmax, volumeEntryDistance, volumeExitDistance);
  if(!intersect)
   return accumulatedColour;

  float worldSpaceDepth = volumeExitDistance - volumeEntryDistance;

  vec3f volumeEntryPoint = ray.origin + ray.direction * volumeEntryDistance;
  vec3f volumeExitPoint = ray.origin + ray.direction * volumeExitDistance;
  volumeEntryPoint = (volumeEntryPoint + boxmax) / boxsize;
  volumeExitPoint = (volumeExitPoint + boxmax) / boxsize;

  vec3f volumeDirection = volumeExitPoint - volumeEntryPoint;
  float volumeSpaceDepth = volumeDirection.length();

  vec3f dataSpaceVector = volumeDirection * volumeDimensions;
  vec3f poolSpaceStep = normalize(dataSpaceVector) / (2.0f * vec3f(poolDimensions));

  float steps = (2.0f * dataSpaceVector.length());
  vec3f volumeSpaceStep = normalize(volumeDirection) / (2.0f * volumeDimensions);
  float volumeSpaceStepSize = volumeSpaceStep.length();
  float worldSpaceStepSize = worldSpaceDepth / steps;//volumeSpaceStepSize * (worldSpaceDepth /volumeSpaceDepth);

  prd.in.rayDirectionInverse = make_float3(1.0f, 1.0f, 1.0f) / ray.direction;
  prd.in.worldSpaceStepSize = worldSpaceStepSize;
  prd.in.worldSpaceStepSizeInv = 1.0f / worldSpaceStepSize;
  prd.in.poolSpaceStep = poolSpaceStep;
  prd.in.opacityCorrection = volumeSpaceStepSize * 150.0f;
  prd.in.volumeSpaceStep = volumeSpaceStep;
  prd.out.pageTableAccesses = 0;

  //prd.out.accumulation = vec4f(volumeEntryPoint, 1.0f);

  prd.rayTerminated = false;

  int depth = 0;
  for (; depth < maxBounces; ++depth) {
    rtTrace(world, ray, prd);

    if(prd.out.accumulation.w >= ertThreshold || prd.rayTerminated)
      break;

    ray.tmin = prd.hit.exitDistance;
  }

  if(showDepthComplexity)
  {
    /* Convert scalar to RGB heat (blue minimum, red maximum) */
    float minimum = 0.0f;
    float maximum = (float)maxBounces;
    float ratio = 2 * ((float)depth - minimum) / (maximum - minimum);
    float b = fmaxf(0.0f, (1.0f - ratio));
    float r = fmaxf(0.0f, (ratio - 1.0f));
    float g = 1.0f - b - r;
    prd.out.accumulation = vec4f(r,g,b, 1.0f);
  }
  else if(showPageTableAccesses)
  {
    float minimum = 0.0f;
    float maximum = (float)maxBounces;
    float ratio = 2 * ((float)prd.out.pageTableAccesses - minimum) / (maximum - minimum);
    float b = fmaxf(0.0f, (1.0f - ratio));
    float r = fmaxf(0.0f, (ratio - 1.0f));
    float g = 1.0f - b - r;
    prd.out.accumulation = vec4f(r,g,b, 1.0f);
  }
  else
  {
    /* Highlight the ERT-terminated pixels */
    if(highlightERT)
    {
      if(prd.out.accumulation.w >= ertThreshold)
      {
        /* Highlight the ERT with red */
        prd.out.accumulation = vec4f(1, 0, 0, 1);
      }
      else
      {
        /* Convert anything that isn't an ERT to grayscale */
        vec4f c = prd.out.accumulation;
        float luminance = c.x * 0.2126f + c.y * 0.7152f + c.z * 0.0722f;
        prd.out.accumulation = vec4f(vec3f(luminance), 1.0f);
      }
    }
  }

  return prd.out.accumulation;
}

/*! the actual ray generation program - note this has no formal
  function parameters, but gets its paramters throught the 'pixelID'
  and 'pixelBuffer' variables/buffers declared above */
RT_PROGRAM void renderPixel()
{
  int pixel_index = pixelID.y * launchDim.x + pixelID.x;
  vec4f col(0.0f);
  DRand48 rnd;
  rnd.init(pixel_index);
  for (int s = 0; s < numSamples; s++) {
    float u = float(pixelID.x) / float(launchDim.x);
    float v = float(pixelID.y) / float(launchDim.y);
    optix::Ray ray = Camera::generateRay(u, v, rnd);
    col += color(ray, rnd);
  }
  col = col / float(numSamples);

  col = saturate(col);
  uchar4 c;
  c.x = col.x * 255.0f;
  c.y = col.y * 255.0f;
  c.z = col.z * 255.0f;
  //c.w = col.w * 255.0f;
  c.w = 255.0f;
  fb[pixelID] = c;
}
