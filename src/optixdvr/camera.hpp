#pragma once

#include "programs/vec.h"
#include <iostream>

struct Camera
{
	bool mUseLookAt = false;
	vec3f mLookat;
    vec3f mLookDirection;
    vec3f mUp;
	vec3f mOrigin;
    float mAperture;
    float mFOVY;
    float mAspect;
    float mFocusDistance;

	Camera()
	{
		mOrigin = vec3f(0, 0, 2);
		mLookDirection = vec3f(0, 0, -1);
        mUp = vec3f(0, 1, 0);
        mFOVY = 30.0f;
        mAspect = 1.0f;
        mFocusDistance = 1.0f;
        mAperture = 0.0f;
	}

	void lookat(const vec3f& p)
	{
		mUseLookAt = true;
		mLookat = p;
        mLookDirection = unit_vector(mLookat - mOrigin);
	}

	void lookdir(const vec3f& d)
	{
		mLookDirection = normalize(d);
		mUseLookAt = false;
	}

    vec3f lookdir()
    {
        return mLookDirection;
    }

	void origin(const vec3f& o)
	{
        mOrigin = o;
        if(mUseLookAt)
        {
            mLookDirection = unit_vector(mLookat - mOrigin);
        }
	}

	void set(optix::Context& context)
	{
        float lens_radius = mAperture / 2.0f;
		float theta = mFOVY * ((float)M_PI) / 180.0f;
		float half_height = tan(theta / 2.0f);
		float half_width = mAspect * half_height;
        vec3f w = -mLookDirection;
		vec3f u = unit_vector(cross(mUp, w));
		vec3f v = cross(w, u);
		vec3f lower_left_corner = mOrigin - half_width * mFocusDistance * u - half_height * mFocusDistance * v - mFocusDistance * w;
		vec3f horizontal = 2.0f * half_width * mFocusDistance * u;
		vec3f vertical = 2.0f * half_height * mFocusDistance * v;

		context["camera_lower_left_corner"]->set3fv(&lower_left_corner.x);
		context["camera_horizontal"]->set3fv(&horizontal.x);
		context["camera_vertical"]->set3fv(&vertical.x);
		context["camera_origin"]->set3fv(&mOrigin.x);
		context["camera_u"]->set3fv(&u.x); 
		context["camera_v"]->set3fv(&v.x);
		context["camera_w"]->set3fv(&w.x);
		context["camera_lens_radius"]->setFloat(lens_radius);
	}
};