#pragma once

#include "optixdvr.hpp"

class OptixInstance
{
private:
    static OptixDVR* mInstance;

public:
    static OptixDVR* get()
    {
        if(mInstance == nullptr)
        {
            mInstance = new OptixDVR();
        }

        return mInstance;
    }

    static OptixDVR* set(OptixDVR* instance)
    {
        mInstance = instance;
    }
};