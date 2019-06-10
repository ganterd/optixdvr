#pragma once

#include "transferfunction.hpp"


#include <optix.h>
#include <optixu/optixpp.h>

class OptixTransferFunction : public TransferFunction
{
public:
    bool mBufferCreated = false;
    optix::Buffer mBuffer;
    optix::TextureSampler mTextureSampler;

    optix::TextureSampler toTextureSampler(optix::Context& context)
    {
        if(!mBufferCreated)
        {
            mBuffer = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, mSize);

            mTextureSampler = context->createTextureSampler();
            mTextureSampler->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
            mTextureSampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_LINEAR);
            mTextureSampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
            mTextureSampler->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);
            mTextureSampler->setBuffer(0, 0, mBuffer);

            mBufferCreated = true;
        }

        updateLUT();



        return mTextureSampler;

    }

    void updateTexture()
    {
        if(mBufferCreated)
        {
            char* mappedbufferdata = (char*)mBuffer->map();
            memcpy(mappedbufferdata, mLUT, sizeof(vec4f) * mSize);
            mBuffer->unmap();
        }
    }
};