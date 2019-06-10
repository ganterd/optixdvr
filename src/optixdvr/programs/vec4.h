#pragma once

#include <cuda.h>
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

struct vec4f
{
    float x, y, z, w;
    inline __device__ vec4f() {}
    inline __device__ vec4f(float f)
    : x(f), y(f), z(f), w(f)
    {}
    inline __device__ vec4f(const float x, const float y, const float z, const float w)
    : x(x), y(y), z(z), w(w)
    {}
    inline __device__ vec4f(const vec4f &v)
    : x(v.x), y(v.y), z(v.z), w(v.w)
    {}
    inline __device__ vec4f(const vec3f &v, const float w)
    : x(v.x), y(v.y), z(v.z), w(w)
    {}
    #ifdef __CUDACC__
    inline __device__ vec4f(const float4 v)
    : x(v.x), y(v.y), z(v.z), w(v.w)
    {}
    inline __device__ float4 as_float4() const { return make_float4(x, y, z, w); }
    inline __device__ float3 as_float3() const { return make_float3(x, y, z); }
    #endif
};

struct mat4x4f
{
    float data[16];

    mat4x4f()
    {
        data[ 0]= 1.0f;   data[ 4]=0.0f;   data[ 8]=0.0f;   data[12]=0.0f;
        data[ 1]= 0.0f;   data[ 5]=1.0f;   data[ 9]=0.0f;   data[13]=0.0f;
        data[ 2]= 0.0f;   data[ 6]=0.0f;   data[10]=1.0f;   data[14]=0.0f;
        data[ 3]= 0.0f;   data[ 7]=0.0f;   data[11]=0.0f;   data[15]=1.0f;
    }

    void lookat(const vec3f& from, const vec3f& to, const vec3f& up)
    {
        vec3f f = normalize(to - from);
        vec3f u = normalize(up);
        vec3f s = normalize(cross(f, u));
        u = normalize(cross(s, f));

        data[ 0]= s.x;  data[ 4]= s.y;  data[ 8]= s.z;  data[12]=-dot(s, from);
        data[ 1]= u.x;  data[ 5]= u.y;  data[ 9]= u.z;  data[13]=-dot(u, from);
        data[ 2]=-f.x;  data[ 6]=-f.y;  data[10]=-f.z;  data[14]= dot(f, from);
        data[ 3]= 0.0f; data[ 7]=0.0f;  data[11]=0.0f;  data[15]= 1.0f;
    }

    void perspective(float fovy, float aspect, float n, float f)
    {
        fovy *= (M_PI / 180.0f);
        float cotan = 1.0f / tanf(fovy * 0.5f);
        data[ 0]= cotan/aspect;  data[ 4]=0.0f;     data[ 8]=0.0f;             data[12]=0.0f;
        data[ 1]= 0.0f;          data[ 5]=cotan;    data[ 9]=0.0f;             data[13]=0.0f;
        data[ 2]= 0.0f;          data[ 6]=0.0f;     data[10]=-(f+n)/(f-n);     data[14]=-2.0f*(f*n)/(f-n);
        data[ 3]= 0.0f;          data[ 7]=0.0f;     data[11]=-1.0f;            data[15]=0.0f;
    }

    mat4x4f operator * ( const mat4x4f& other ) const {
        mat4x4f result;
        for (int x = 0;x<16;x+=4)
        for (int y = 0;y<4;y++)
            result.data[x+y] = data[0 + x] * other.data[0 + y]+
                        data[1 + x] * other.data[4 + y]+
                        data[2 + x] * other.data[8 + y]+
                        data[3 + x] * other.data[12 + y];

        return result;
    }
};
