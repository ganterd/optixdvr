#include "vec3.h"


#include <cuda.h>
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

inline __device__ vec3f operator*(const vec3f &a, const vec3f &b)
{
  return vec3f(a.x*b.x, a.y*b.y, a.z*b.z);
}

inline __device__ vec3f operator/(const vec3f &a, const vec3f &b)
{
  return vec3f(a.x/b.x, a.y/b.y, a.z/b.z);
}

inline __device__ vec3f operator-(const vec3f &a, const vec3f &b)
{
  return vec3f(a.x-b.x, a.y-b.y, a.z-b.z);
}

inline __device__ vec3f operator-(const vec3f &a)
{
  return vec3f(-a.x, -a.y, -a.z);
}

inline __device__ vec3f operator+(const vec3f &a, const vec3f &b)
{
  return vec3f(a.x+b.x, a.y+b.y, a.z+b.z);
}

inline __device__ vec3f &operator+=(vec3f &a, const vec3f &b)
{
  a = a + b;
  return a;
}

inline __device__ vec3f &operator*=(vec3f &a, const vec3f &b)
{
  a = a * b;
  return a;
}

inline __device__ vec3f operator*(const mat3x3f &m, const vec3f &b)
{
  vec3f r(
    b.x * m.m00 + b.y * m.m10 + b.z * m.m20,
    b.x * m.m01 + b.y * m.m11 + b.z * m.m21,
    b.x * m.m02 + b.y * m.m12 + b.z * m.m22
  );
  return r;
}

inline __device__ bool operator!=(const vec3f &a, const vec3f &b)
{
  return (a.x != b.x) || (a.y != b.y) || (a.z != b.z);
}

inline __device__ mat3x3f rotationX(float radians)
{
  mat3x3f m;
  m.m00 = 1.0f; m.m10 = 0.0f; m.m20 = 0.0f;
  m.m01 = 0.0f; m.m11 = cosf(radians); m.m21 = -sinf(radians);
  m.m02 = 0.0f; m.m12 = sinf(radians); m.m22 = cosf(radians);
  return m;
}

inline __device__ mat3x3f rotationY(float radians)
{
  mat3x3f m;
  m.m00 = cosf(radians); m.m10 = 0.0f; m.m20 = sinf(radians);
  m.m01 = 0.0f; m.m11 = 1.0f; m.m21 = 0.0f;
  m.m02 = -sinf(radians); m.m12 = 0.0f; m.m22 = cosf(radians);
  return m;
}

inline __device__ float dot(const vec3f &a, const vec3f &b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline __device__ vec3f cross(const vec3f &a, const vec3f &b)
{
  return vec3f(
               a.y*b.z - a.z*b.y,
               a.z*b.x - a.x*b.z,
               a.x*b.y - a.y*b.x);
}

inline __device__ vec3f normalize(const vec3f &v)
{
  return v * (1.f / sqrtf(dot(v, v)));
}


inline __device__ vec3f unit_vector(const vec3f &v)
{
  return normalize(v);
}

/*! return absolute value of each component */
inline __device__ vec3f abs(const vec3f &v)
{
  return vec3f(fabsf(v.x),fabsf(v.y),fabsf(v.z));
}

inline __device__ vec3f mod(const vec3f &a,const vec3f &b)
{
  return vec3f(fmodf(a.x,b.x),
               fmodf(a.y,b.y),
               fmodf(a.z,b.z));
}

inline __device__ vec3f min(const vec3f& a, const vec3f& b)
{
  return vec3f(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}

/*! return max of the three components */
inline __device__ float max(const vec3f& v)
{
  return fmaxf(fmaxf(v.x, v.y), v.z);
}

/*! return a vector with the component-wise maxes */
inline __device__ vec3f max(const vec3f& a, const vec3f& b)
{
  return vec3f(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}

#ifdef __CUDACC__
inline __device__ float3 min(const float3& a, const float3& b)
{
  return make_float3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z));
}

inline __device__ float3 max(const float3& a, const float3& b)
{
  return make_float3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z));
}
#endif



//////////////////////// vec3size_t ////////////////////////
inline __device__ vec3size_t operator+(const vec3size_t &a, const vec3size_t &b)
{
  return vec3size_t(a.x+b.x, a.y+b.y, a.z+b.z);
}

inline __device__ bool operator>(const vec3size_t &a, const vec3size_t &b)
{
  return (a.x > b.x) || (a.y > b.y) || (a.z > b.z);
}

inline __device__ bool operator!=(const vec3size_t &a, const vec3size_t &b)
{
  return (a.x != b.x) || (a.y != b.y) || (a.z != b.z);
}