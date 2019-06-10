#include "transferfunction.hpp"

void TransferFunction::updateLUT()
{
    if (mLUT)
        delete[] mLUT;

    mLUT = new vec4f[mSize];
    memset(mLUT, 0, sizeof(vec4f) * mSize);

    if(mPoints.size() == 0)
    {
        std::cerr << "TF has no control points" << std::endl;
    }
    else
    {
        float range = mRangeMax - mRangeMin;

        unsigned int leftIndex = 0;
        unsigned int rightIndex = 0;

        TransferFunctionPoint left;
        TransferFunctionPoint right;
        for(unsigned int i = 0; i < mSize; ++i)
        {
            float intensity = (float)i / (float)mSize;
            intensity -= mRangeMin;
            intensity *= range;

            for(; leftIndex < mPoints.size(); ++leftIndex)
            {
                if(mPoints[leftIndex].mIntensity > intensity)
                {
                    break;
                }
            }

            leftIndex = std::max(0, ((int)leftIndex - 1));

            for(rightIndex = leftIndex; rightIndex < mPoints.size(); ++rightIndex)
            {
                if(mPoints[rightIndex].mIntensity >= intensity || rightIndex == mPoints.size() - 1)
                {
                    break;
                }
            }

            left = mPoints[leftIndex];
            right = mPoints[rightIndex];

            float diff = (right.mIntensity - left.mIntensity);
            float t = 0.0f;
            if(diff > 0.0f)
            {
                t = (intensity - left.mIntensity) / diff;
            }


            vec4f c = left.mColour * (1.0f - t) + right.mColour * t;
            mLUT[i] = c;
        }
    }

    updateTexture();
}

bool TransferFunction::rangeActive(const float from, const float to) const
{
    // Temporary super basic check
    bool rightactive =
        from > mPoints[mPoints.size() - 1].mIntensity
        && mPoints[mPoints.size() - 1].mColour.w == 0.0f;
    bool leftactive =
        to < mPoints[0].mIntensity
        && mPoints[0].mColour.w == 0.0f;
    return !(rightactive || leftactive);
}

void TransferFunction::addControlPoint(float v, float r, float g, float b, float a)
{
    std::vector<TransferFunctionPoint>::iterator it;
    it = mPoints.begin();
    for(; it < mPoints.end(); ++it)
    {
        if((*it).mIntensity > v)
            break;
    }

    TransferFunctionPoint p;
    p.mIntensity = v;
    p.mColour = vec4f(r, g, b, a);
    mPoints.insert(it, p);
}

std::ostream& operator<< (std::ostream& out, const TransferFunction& tf)
{
    out << "TF: " << tf.mPoints.size() << " points" << std::endl;
    for(unsigned int i = 0; i < tf.mPoints.size(); ++i)
    {
        TransferFunctionPoint p = tf.mPoints[i];
        out << "[" << std::setw(3) << i << "]";
        out << " v:" << std::fixed << std::setw(5) << p.mIntensity;
        out << " r:" << std::fixed << std::setw(5) << p.mColour.x;
        out << " g:" << std::fixed << std::setw(5) << p.mColour.y;
        out << " b:" << std::fixed << std::setw(5) << p.mColour.z;
        out << " a:" << std::fixed << std::setw(5) << p.mColour.w;
        out << std::endl;
    }

    return out;
}