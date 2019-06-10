#pragma once

#include "vec2.h"

inline __device__ vec2f operator+(const vec2f& a, const vec2f& b)
{
  return vec2f(a.x + b.x, a.y + b.y);
}

inline __device__ vec2f operator-(const vec2f& a, const vec2f& b)
{
  return vec2f(a.x - b.x, a.y - b.y);
}

inline __device__ vec2f operator*(const vec2f& a, float b)
{
  return vec2f(a.x * b, a.y * b);
}

inline __device__ vec2f operator*(const vec2f& a, const vec2f &b)
{
  return vec2f(a.x * b.x, a.y * b.y);
}

inline __device__ vec2f operator/(const vec2f& a, const vec2f &b)
{
  return vec2f(a.x / b.x, a.y / b.y);
}

inline __device__ vec2f abs(const vec2f& a)
{
  return vec2f(abs(a.x), abs(a.y));
}

inline __device__ bool allLessThan(const vec2f& a, const vec2f& b)
{
  return (a.x < b.x) && (a.y < b.y);
}
