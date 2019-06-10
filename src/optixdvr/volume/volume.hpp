#pragma once

#include "../programs/vec.h"
#include "../utils/savePPM.h"

#include <iostream>
#include <limits>
#include <string.h>
//#include <optix.h>
//#include <optixu/optixpp.h>

class Volume
{
public:
    enum DataType
    {
        CHAR,
        UCHAR,
        SHORT,
        USHORT,
        INT,
        UINT,
        LONG,
        ULONG,
        FLOAT,
        DOUBLE
    };

    vec3f dataDimensions;
    vec3f dataLimits;
    vec3f volumeSize = vec3f(1.0f);
    size_t voxelsTotal;
    size_t dataTotal;
    size_t bytesPerVoxel;
    DataType dataType;
    char* data;
    float dataScale;

    Volume() :
        dataDimensions(0),
        voxelsTotal(0),
        dataScale(1.0f),
        data(NULL)
    {
    }

    inline size_t XYZToIdx(const vec3f &p)
    {
        return
            (size_t)p.x
            + (size_t)p.y * (size_t)dataDimensions.x
            + (size_t)p.z * (size_t)dataDimensions.x * (size_t)dataDimensions.y;
    }

    virtual char* voxeladdress(const vec3f& p) = 0;
    virtual float GetNormalisedVoxel(const vec3f& p) = 0;
};

template <typename T> class VolumeRepresentation : public Volume
{
public:
    VolumeRepresentation() : Volume()
    {
        bytesPerVoxel = sizeof(T);
    };

    char* voxeladdress(const vec3f& p){ return (char*)&((T*)data)[XYZToIdx(p)]; };
    float GetNormalisedVoxel(const vec3f& p){	return TypeToNormalisedFloat(((T*)data)[XYZToIdx(p)]); }
    void SetNormalisedVoxel(const vec3f& p, float volume){ ((T*)data)[XYZToIdx(p)] = NormalisedFloatToType(volume);	}
    T NormalisedFloatToType(float volume){ return (T)(volume * (float)std::numeric_limits<T>::max()); }
    float TypeToNormalisedFloat(T volume){ return (float)volume / (float)std::numeric_limits<T>::max(); }
};

template<> class VolumeRepresentation<float> : public Volume
{
public:
    VolumeRepresentation() : Volume() { bytesPerVoxel = sizeof(float); };

    char* voxeladdress(const vec3f& p){ return (char*)&((float*)data)[XYZToIdx(p)]; };
    float GetNormalisedVoxel(const vec3f& p){ return ((float*)data)[XYZToIdx(p)]; }
    void SetNormalisedVoxel(const vec3f& p, float volume){ ((float*)data)[XYZToIdx(p)] = volume; }
};

template <> class VolumeRepresentation<double> : public Volume
{
public:
    VolumeRepresentation() : Volume() { bytesPerVoxel = sizeof(double); };

    char* voxeladdress(const vec3f& p){ return (char*)&((double*)data)[XYZToIdx(p)]; };
    float GetNormalisedVoxel(const vec3f& p){ return (float)((double*)data)[XYZToIdx(p)]; }
    void SetNormalisedVoxel(const vec3f& p, float volume){ ((double*)data)[XYZToIdx(p)] = (double)volume; }
};

class VolumeFile
{
public:

    vec3f dataDimensions;
    vec3f elementSpacing = vec3f(1.0f);
    Volume::DataType type;
    char bytesPerElement;
    std::string directory;
    std::vector<std::string> volumes;
    Volume* volume = nullptr;

    Volume* loadFrame(int f)
    {
        if(volume == nullptr)
        {
            switch(type)
            {
            case Volume::FLOAT:
                volume = new VolumeRepresentation<float>();
                break;
            case Volume::UCHAR:
                volume = new VolumeRepresentation<unsigned char>();
                break;
            case Volume::USHORT:
                volume = new VolumeRepresentation<unsigned short>();
                break;
            default:
                std::cerr << "Haven't handled this type yet..." << std::endl;
            }

            volume->dataType = type;
            volume->dataDimensions = dataDimensions;
            volume->volumeSize = dataDimensions * elementSpacing;
            volume->dataLimits = dataDimensions - vec3f(1.0f);
            volume->voxelsTotal = (size_t)dataDimensions.x
                * (size_t)dataDimensions.y
                * (size_t)dataDimensions.z;
            volume->dataTotal = volume->bytesPerVoxel * volume->voxelsTotal;
            volume->data = new char[volume->dataTotal];
        }

        const char* dataPath = volumes[f].c_str();
        FILE* fp = fopen(dataPath, "rb");
        if (fp == NULL)
        {
            std::cerr << "Error opening file: " << dataPath << std::endl;
            exit(0);
        }
        else
        {
            if(fread(volume->data, 1, volume->dataTotal, fp) < volume->dataTotal)
            {
                std::cerr << "Fewer bytes were found that expected in voxel data file" << std::endl;
            }
            else
            {
                std::cout << "Successfully read '" << dataPath << "' (" << volume->dataTotal << " bytes)" << std::endl;
            }
            fclose(fp);
        }

        return volume;
    }
};