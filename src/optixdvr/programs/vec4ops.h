#pragma once

#include "vec4.h"

inline __device__ vec4f operator*(const vec4f &a, const vec4f &b)
{
  return vec4f(a.x*b.x, a.y*b.y, a.z*b.z, a.w * b.w);
}

inline __device__ vec4f operator/(const vec4f &a, const vec4f &b)
{
  return vec4f(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

inline __device__ vec4f operator-(const vec4f &a, const vec4f &b)
{
  return vec4f(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

inline __device__ vec4f operator-(const vec4f &a)
{
  return vec4f(-a.x, -a.y, -a.z, -a.w);
}

inline __device__ vec4f operator+(const vec4f &a, const vec4f &b)
{
  return vec4f(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline __device__ vec4f &operator+=(vec4f &a, const vec4f &b)
{
  a = a + b;
  return a;
}

inline __device__ vec4f &operator*=(vec4f &a, const vec4f &b)
{
  a = a * b;
  return a;
}

inline __device__ vec4f abs(const vec4f &v)
{
  return vec4f(fabsf(v.x),fabsf(v.y),fabsf(v.z),fabsf(v.w));
}

inline __device__ vec4f saturate(const vec4f& v)
{
  return vec4f(
    fminf(1.0f, fmaxf(0.0f, v.x)),
    fminf(1.0f, fmaxf(0.0f, v.y)),
    fminf(1.0f, fmaxf(0.0f, v.z)),
    fminf(1.0f, fmaxf(0.0f, v.w))
  );
}