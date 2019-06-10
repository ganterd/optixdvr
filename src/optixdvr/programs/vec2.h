#pragma once

#include <cuda.h>
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

struct vec2f
{
  float x, y;
  inline __device__ vec2f(){};
  inline __device__ vec2f(float f) : x(f), y(f) {};
  inline __device__ vec2f(const float x, const float y) : x(x), y(y) {};
  inline __device__ vec2f(const vec2f& v) : x(v.x), y(v.y) {};
}; 
