#pragma once

#include <cuda.h>
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

struct vec3f 
{
    float x, y, z;
    
    inline __device__ vec3f() {}
    inline __device__ vec3f(float f) 
    : x(f), y(f), z(f) 
    {}
    inline __device__ vec3f(const float x, const float y, const float z)
    : x(x), y(y), z(z)
    {}
    inline __device__ vec3f(const vec3f &v)
    : x(v.x), y(v.y), z(v.z)
    {}
    #ifdef __CUDACC__
    inline __device__ vec3f(const float3 v)
    : x(v.x), y(v.y), z(v.z)
    {}
    inline __device__ float3 as_float3() const { return make_float3(x, y, z); }
    #endif
    inline __device__ float squared_length() const { return x * x + y * y + z * z; }
    inline __device__ float length() const { return sqrtf(x * x + y * y + z * z); }
    inline __device__ vec2f xy() const { return vec2f(x, y); };
    inline __device__ vec2f xz() const { return vec2f(x, z); };
    inline __device__ vec2f yz() const { return vec2f(y, z); };

};

struct mat3x3f
{
    float m00, m10, m20;
    float m01, m11, m21;
    float m02, m12, m22;

    inline __device__ mat3x3f() {
        m00 = 1.0f; m10 = 0.0f; m20 = 0.0f;
        m01 = 0.0f; m11 = 1.0f; m21 = 0.0f;
        m02 = 0.0f; m12 = 0.0f; m22 = 1.0f;
    }
};

struct vec3size_t
{
    size_t x, y, z;

    inline __device__ vec3size_t(){};
    inline __device__ vec3size_t(size_t s)
        : x(s), y(s), z(s) {};
    inline __device__ vec3size_t(const size_t x, const size_t y, const size_t z)
        : x(x), y(y), z(z) {};
    inline __device__ vec3size_t(const vec3f &v)
        : x((size_t)v.x), y((size_t)v.y), z((size_t)v.z) {};
};