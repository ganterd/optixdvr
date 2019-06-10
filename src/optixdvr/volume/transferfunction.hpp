#pragma once

#include "../programs/vec.h"
#include "../utils/utils.h"
#include "../utils/tinyxml2.h"

#include <iomanip>
#include <vector>
#include <fstream>

class TransferFunctionPoint
{
public:
    float mIntensity;
    vec4f mColour;
};

class TransferFunction
{
public:
    std::vector<TransferFunctionPoint> mPoints;
    float mRangeMin = 0.0f;
    float mRangeMax = 1.0f;
    vec4f *mLUT = nullptr;

    const int mSize = 256;

    TransferFunction(){};
    ~TransferFunction(){};

    void fromFile(const std::string& filePath);

    void addControlPoint(float value, float r, float g, float b, float a);
    std::vector<TransferFunctionPoint>& points(){ return mPoints; };
    void updateLUT();
    virtual void updateTexture(){};

    bool rangeActive(const float from, const float to) const;

    friend std::ostream& operator<< (std::ostream& out, const TransferFunction& tf);
};

class TFITransferFunctionLoader
{
    public:
    static bool load(const std::string& filePath, TransferFunction* tf)
    {
        const char* text = utils::io::readFile(filePath.c_str());
        if(text == NULL)
        {
            std::cerr << "Couldn't read TF file: " << filePath;
            return NULL;
        }

        tinyxml2::XMLDocument doc;
        doc.Parse(text);
        tinyxml2::XMLElement* key = doc.FirstChildElement("InviwoWorkspace")->FirstChildElement("Points")->FirstChildElement("Point");

        while (key)
        {
            float intensity = (float)atof(key->FirstChildElement("pos")->Attribute("content"));

            float r = atof(key->FirstChildElement("rgba")->Attribute("x"));
            float g = atof(key->FirstChildElement("rgba")->Attribute("y"));
            float b = atof(key->FirstChildElement("rgba")->Attribute("z"));
            float a = atof(key->FirstChildElement("rgba")->Attribute("w"));

            tf->addControlPoint(intensity, r, g, b, a);

            key = key->NextSiblingElement();
        }
    }
};

class I3DTransferFunction
{
    public:
    static void save(const TransferFunction* tf, const std::string& filepath, const int size = 256)
    {
        std::ofstream file;
        file.open(filepath);
        file << size << std::endl;

        if(tf->mPoints.size() == 0)
        {
            std::cerr << "TF has no control points" << std::endl;
        }
        else
        {
            float range = tf->mRangeMax - tf->mRangeMin;

            unsigned int leftIndex = 0;
            unsigned int rightIndex = 0;

            TransferFunctionPoint left;
            TransferFunctionPoint right;
            for(unsigned int i = 0; i < size; ++i)
            {
                float intensity = (float)i / (float)size;
                intensity -= tf->mRangeMin;
                intensity *= range;

                for(; leftIndex < tf->mPoints.size(); ++leftIndex)
                {
                    if(tf->mPoints[leftIndex].mIntensity > intensity)
                    {
                        break;
                    }
                }

                leftIndex = std::max(0, ((int)leftIndex - 1));

                for(rightIndex = leftIndex; rightIndex < tf->mPoints.size(); ++rightIndex)
                {
                    if(tf->mPoints[rightIndex].mIntensity >= intensity || rightIndex == tf->mPoints.size() - 1)
                    {
                        break;
                    }
                }

                left = tf->mPoints[leftIndex];
                right = tf->mPoints[rightIndex];

                float diff = (right.mIntensity - left.mIntensity);
                float t = 0.0f;
                if(diff > 0.0f)
                {
                    t = (intensity - left.mIntensity) / diff;
                }


                vec4f c = left.mColour * (1.0f - t) + right.mColour * t;
                file << c.x << " " << c.y << " " << c.z << " " << c.w << std::endl;
            }
        }
        file.close();
    }
};