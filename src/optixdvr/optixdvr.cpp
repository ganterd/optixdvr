// ======================================================================== //
// Copyright 2019 David Ganter                                              //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "optixdvr.hpp"
#include "optixdvr_instance.hpp"

#include "utils/savePPM.h"

#include "volume/mhdreader.hpp"
#define _USE_MATH_DEFINES 1
#include <math.h>
#include <cmath>
#include <random>
#include <iostream>
#include <iomanip>
#include <chrono>

//
extern "C" const char embedded_aabb_program[];
extern "C" const char embedded_volume_program[];
extern "C" const char embedded_volume_shading_program[];
extern "C" const char embedded_raygen_program[];
extern "C" const char embedded_miss_program[];

OptixDVR::OptixDVR()
{
	OptixInstance::set(this);

	m_volumefilepath = "";
	m_transferfuncpath = "";
	m_outppmpath = "out.ppm";
	m_renderdata = nullptr;
	m_framebuffer = nullptr;
	mPreviousBrickSize = vec3size_t(0);

	// Create OptiX context with RTX on or off
	int rtxEnabled = 1;
  	rtGlobalSetAttribute(RT_GLOBAL_ATTRIBUTE_ENABLE_RTX, sizeof(int), &rtxEnabled);
	m_context = optix::Context::create();
	m_context->setRayTypeCount(1);
	m_context->setStackSize(16000);

	setRayGenProgram();
	setMissProgram();

	// Create the un-shaded DVR program and material
	m_volumedvrprogram = m_context->createProgramFromPTXString(
		embedded_volume_program, "closest_hit"
	);
	m_volumedvrmaterial = m_context->createMaterial();
	m_volumedvrmaterial->setClosestHitProgram(0, m_volumedvrprogram);

	// Create the shaded DVR program and material
	m_volumeshadingdvrprogram = m_context->createProgramFromPTXString(
		embedded_volume_shading_program, "closest_hit"
	);
	m_volumeshadingdvrmaterial = m_context->createMaterial();
	m_volumeshadingdvrmaterial->setClosestHitProgram(0, m_volumeshadingdvrprogram);

	// Create the AABB bounds and intersection programs
	m_program_aabb_bounds = m_context->createProgramFromPTXString(
		embedded_aabb_program, "get_aabb_bounds"
	);
	m_program_aabb_hit = m_context->createProgramFromPTXString(
		embedded_aabb_program, "hit_aabb"
	);

	// Create a single geometry type for AABBs
	m_geometry_aabb = m_context->createGeometry();
	m_geometry_aabb->setPrimitiveCount(1);
	m_geometry_aabb->setBoundingBoxProgram(m_program_aabb_bounds);
	m_geometry_aabb->setIntersectionProgram(m_program_aabb_hit);

	// Create the acceleration ds
	m_bvh = m_context->createAcceleration("Trbvh");
	m_bvh->setProperty("refit", "1");

	// Instantiate the world geometry group
	m_world = m_context->createGeometryGroup();
	m_world->setAcceleration(m_bvh);
	m_context["world"]->set(m_world);

	m_volumegeometry = m_context->createGeometry();
	m_volumegeometry->setBoundingBoxProgram(m_program_aabb_bounds);
	m_volumegeometry->setIntersectionProgram(m_program_aabb_hit);

	m_volumegeometryinstance = m_context->createGeometryInstance();
	m_volumegeometryinstance->setGeometry(m_volumegeometry);
	m_volumegeometryinstance->setMaterialCount(1);
	m_volumegeometryinstance->setMaterial(0, m_volumedvrmaterial);
	m_world->addChild(m_volumegeometryinstance);

	m_aabbMinBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, 1);
	m_aabbMaxBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, 1);

	m_subdivision = new BrickedVolume();
	mPool = new OptixVolumeBrickPool();



	resizeFrameBuffer(512, 512);
};


void OptixDVR::volumeMaterial(
	optix::GeometryInstance &gi,
	const vec3f &boxMin,
	const vec3f &boxMax
){
	gi->setMaterial(0, m_volumedvrmaterial);
}


void OptixDVR::shadedVolumeMaterial(
	optix::GeometryInstance &gi,
	const vec3f &boxMin,
	const vec3f &boxMax,
	const vec3f &lpos
){
	gi->setMaterial(0, m_volumeshadingdvrmaterial);
	gi["lightPosition"]->setFloat(lpos.x, lpos.y, lpos.z);

	const vec3f gradientStep = 1.0f / m_volume->dataDimensions;
	gi["gradientStep"]->setFloat(gradientStep.x, gradientStep.y, gradientStep.z);
}

optix::GeometryInstance OptixDVR::createAABB(
	const vec3f &boxCenter,
	const vec3f &boxRadius
){
	optix::GeometryInstance gi = m_context->createGeometryInstance();
	gi->setGeometry(m_geometry_aabb);
	gi->setMaterialCount(1);

	vec3f boxMin = boxCenter - boxRadius;
	vec3f boxMax = boxCenter + boxRadius;
	gi["aabbMin"]->setFloat(boxMin.x, boxMin.y, boxMin.z);
	gi["aabbMax"]->setFloat(boxMax.x, boxMax.y, boxMax.z);

	if(m_useshading)
	{
		shadedVolumeMaterial(gi, boxMin, boxMax, m_lightposition);
	}
	else
	{
		volumeMaterial(gi, boxMin, boxMax);
	}

	return gi;
}

void OptixDVR::loadvolume(const char* volumepath)
{
	m_volumefilepath = std::string(volumepath);
	VolumeFile volumefile = MHDHeaderReader::Load(m_volumefilepath.c_str());
	utils::Timer timer;
	timer.start();
	m_volume = volumefile.loadFrame(0);
	timer.stop();
	mStats.set("volumeloadtime", timer.getTime());


	// Do an initial subdivision on the volume
	timer.start();
	m_subdivision->set_volume(m_volume);
	timer.stop();
	m_subdivision->mStats.set("subdivisiontime", timer.getTime());

	mPool->volume(m_volume);

	setup();
}

void OptixDVR::loadtransferfunction(const char* tfpath)
{
	m_transferfuncpath = std::string(tfpath);
	m_transferfunction = new OptixTransferFunction();
	TFITransferFunctionLoader::load(m_transferfuncpath, m_transferfunction);
	m_transferfunction->toTextureSampler(m_context);
	setup();
}

void OptixDVR::createScene()
{
	// Load the transfer function
	m_context["transferFunction"]->set(
		m_transferfunction->toTextureSampler(m_context)
	);

	// Test the subdivision with the TF and upload bricks
	updateScene();

	return;
}

void OptixDVR::updateScene()
{
	utils::Timer timer;

	if(!m_volume)
	{
		return;
	}

	if(!m_transferfunction)
	{
		return;
	}

	/* Test the subdivision bricks against the transfer funcion */
	timer.start();
	size_t changes = m_subdivision->testbricks(*m_transferfunction);
	timer.stop();

	timer.start();
	size_t poolChanges = mPool->testBricks(*m_transferfunction);
	timer.stop();

	/* Upload bricks if any unpaged bricks need to be uploaded */
	{
		timer.start();
		if(mPool->upload() > 0)
		{
			/* Dummy code to flush the uploads */
			m_volumegeometryinstance["aabbMinBuffer"]->set(m_aabbMinBuffer);
			m_volumegeometryinstance["aabbMaxBuffer"]->set(m_aabbMaxBuffer);
			m_volumegeometry->setPrimitiveCount(0);
			renderFrame(0,0);
			timer.stop();
			m_subdivision->mStats.set("lastuploadtime", timer.getTime());
		}
		else
		{
			timer.stop();
		}
	}

	mAABBMinData.clear();
	mAABBMaxData.clear();

	/* Copy ESS bounds data to OptiX AABBs. */
	vec3f center = vec3f(0.0f);
	vec3f volumeRadius = m_volume->volumeSize * 0.5f;
	vec3f subdivisions = m_subdivision->mNumLeaves;
	vec3f subdivisionsize = m_volume->volumeSize / subdivisions;
	vec3f voxelsize = vec3f(1.0f) / m_volume->dataDimensions;
	if(m_subdivision->mCluster)
	{
		size_t totalclusters = m_subdivision->mClusters.size();
		for (int i = 0; i < totalclusters; ++i)
		{
			struct BrickedVolume::Cluster& cluster = m_subdivision->mClusters[i];

			vec3f boxMin = cluster.mStart * subdivisionsize + (center - volumeRadius);
			vec3f boxMax = (cluster.mStart + cluster.mSize) * subdivisionsize + (center - volumeRadius);
			vec3f dummyMin(0.0f);
			vec3f dummyMax(0.00000001f);

			mAABBMinData.push_back(vec4f(boxMin.x, boxMin.y, boxMin.z, 0.0f));
			mAABBMaxData.push_back(vec4f(boxMax.x, boxMax.y, boxMax.z, 0.0f));
		}
	}
	else
	{
		size_t totalsubdivisions = subdivisions.x * subdivisions.y * subdivisions.z;

		vec3f subdivisionradius = volumeRadius / subdivisions;
		vec3f volumeMin = center - volumeRadius;

		for (int z = 0; z < (int)subdivisions.z; ++z)
		{
			for (int y = 0; y < (int)subdivisions.y; ++y)
			{
				for (int x = 0; x < (int)subdivisions.x; ++x)
				{
					vec3f index(x, y, z);
					if(m_subdivision->leafActive(x, y, z))
					{
						vec3f boxMin = volumeMin + subdivisionsize * index;
						vec3f boxMax = boxMin + subdivisionsize + voxelsize;
						mAABBMinData.push_back(vec4f(boxMin.x, boxMin.y, boxMin.z, 0.0f));
						mAABBMaxData.push_back(vec4f(boxMax.x, boxMax.y, boxMax.z, 0.0f));
					}
				}
			}
		}
	}

	m_aabbMinBuffer->destroy();
	m_aabbMaxBuffer->destroy();

	m_aabbMinBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, mAABBMinData.size());
	m_aabbMaxBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, mAABBMaxData.size());

	optix::float4 *minMap = (optix::float4*)m_aabbMinBuffer->map();
	optix::float4 *maxMap = (optix::float4*)m_aabbMaxBuffer->map();

	memcpy(minMap, &mAABBMinData[0], mAABBMinData.size() * sizeof(optix::float4));
	memcpy(maxMap, &mAABBMaxData[0], mAABBMaxData.size() * sizeof(optix::float4));

	m_aabbMinBuffer->unmap();
	m_aabbMaxBuffer->unmap();

	m_volumegeometryinstance["aabbMinBuffer"]->set(m_aabbMinBuffer);
	m_volumegeometryinstance["aabbMaxBuffer"]->set(m_aabbMaxBuffer);
	m_volumegeometry->setPrimitiveCount(mAABBMinData.size());
	mStats.set("optixprimitivescount", mAABBMinData.size());

	m_context["volumeDimensions"]->setFloat(
		m_volume->dataDimensions.x,
		m_volume->dataDimensions.y,
		m_volume->dataDimensions.z
	);

	VolumeBrick& exampleBrick = mPool->brick(0, 0, 0);
	m_context["brickDimensions"]->setFloat(
		exampleBrick.mDataDimensions.x,
		exampleBrick.mDataDimensions.y,
		exampleBrick.mDataDimensions.z
	);

	vec3f brickDimensions(
		exampleBrick.mDataDimensions.x,
		exampleBrick.mDataDimensions.y,
		exampleBrick.mDataDimensions.z
	);
	vec3f brickSizeVolumeSpace = brickDimensions / m_volume->dataDimensions;
	m_context["brickSizeVolumeSpace"]->setFloat(
		brickSizeVolumeSpace.x,
		brickSizeVolumeSpace.y,
		brickSizeVolumeSpace.z
	);

	m_context["poolDimensions"]->setFloat(
		mPool->mDataDimensions.x,
		mPool->mDataDimensions.y,
		mPool->mDataDimensions.z
	);

	m_context["volumeMin"]->setFloat(
		center.x - volumeRadius.x,
		center.y - volumeRadius.y,
		center.z - volumeRadius.z
	);

	m_context["volumeSize"]->setFloat(
		m_volume->volumeSize.x,
		m_volume->volumeSize.y,
		m_volume->volumeSize.z
	);

	m_context->validate();
	if(changes > 0)
	{
		m_bvh->markDirty();
		cudaDeviceSynchronize();
		timer.start();
		renderFrame(0, 0);
		cudaDeviceSynchronize();
		timer.stop();
		mStats.set("lastbvhbuildtime", timer.getTime());
		m_lastbvhbuildtime = timer.getTime();
	}
}

void OptixDVR::renderFrame(int Nx, int Ny)
{
	m_context->launch(0, Nx, Ny);
}

optix::Buffer OptixDVR::createFrameBuffer(int Nx, int Ny)
{
	optix::Buffer pixelBuffer = m_context->createBuffer(RT_BUFFER_OUTPUT);
	pixelBuffer->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
	pixelBuffer->setSize(Nx, Ny);
	return pixelBuffer;
}

void OptixDVR::setRayGenProgram()
{
	optix::Program program = m_context->createProgramFromPTXString(
		embedded_raygen_program, "renderPixel"
	);
	m_context->setEntryPointCount(1);
	m_context->setRayGenerationProgram(0, program);
}

void OptixDVR::setMissProgram()
{
	optix::Program program = m_context->createProgramFromPTXString(
		embedded_miss_program, "miss_program"
	);
	m_context->setMissProgram(0, program);
}

int OptixDVR::setup()
{
	if(m_ready)
	{
		return 0;
	}
	if(!m_volume)
	{
		return -1;
	}

	if(!m_transferfunction)
	{
		return -1;
	}

	const size_t Nx = m_renderwidth, Ny = m_renderheight;

	m_camera.mAspect = float(Nx) / float(Ny);
	m_camera.set(m_context);

	createScene();

	m_ready = true;
}

int OptixDVR::render()
{
	if(!m_ready)
		return -1;

	m_camera.mAspect = float(m_renderwidth) / float(m_renderheight);
	m_camera.set(m_context);

	m_context["ertThreshold"]->setFloat(0.99f);
	m_context["highlightERT"]->setInt(m_highlightert ? 1 : 0);
	m_context["showDepthComplexity"]->setInt(m_showdepthcomplexity ? 1 : 0);
	m_context["showPageTableAccesses"]->setInt(m_showPageTableAccesses ? 1 : 0);
	m_context["dontSample"]->setInt(m_dontsample ? 1 : 0);

	const int numSamples = m_samples;
	m_context["numSamples"]->setInt(numSamples);

	vec3f subdivisions = m_subdivision->mNumLeaves;
	const int maxBounces = subdivisions.x + subdivisions.y + subdivisions.z;
	m_context["maxBounces"]->setInt(maxBounces);

	// Render Frame
	utils::Timer timer;
	timer.start();
	renderFrame(m_renderwidth, m_renderheight);
	timer.stop();
	m_lastrenderduration = timer.getTime();
	mStats.set("rendertime", m_lastrenderduration);

	m_previousframetimepoint = std::chrono::system_clock::now();

	// Copy render buffer to host
	if(m_renderdata)
		delete[] m_renderdata;
	const unsigned char *pixels = (const unsigned char *)m_framebuffer->map();
	m_renderdata = new unsigned char[m_renderwidth * m_renderheight * 4];
	memcpy(m_renderdata, pixels, m_renderwidth * m_renderheight * 4);
	m_framebuffer->unmap();
	m_frameavailable = true;

	return 0;
}

void OptixDVR::saveToPNG(const char* path)
{
	const unsigned char *pixels = (const unsigned char *)m_framebuffer->map();
	savePNG(path, m_renderwidth, m_renderheight, pixels);
	m_framebuffer->unmap();
}

void OptixDVR::resizeFrameBuffer(int w, int h)
{
	if(m_framebuffer)
		m_framebuffer->destroy();
	m_framebuffer = createFrameBuffer(w, h);
	m_context["fb"]->set(m_framebuffer);
	m_renderwidth = w;
	m_renderheight = h;

	m_camera.mAspect = float(m_renderwidth) / float(m_renderheight);
	m_camera.set(m_context);
}