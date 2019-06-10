#pragma once

#include <ctime>
#include <chrono>
#include <vector>

#include <optix.h>
#include <optixu/optixpp.h>

#include "camera.hpp"
#include "programs/vec.h"
#include "volume/brickedvolume.hpp"
#include "volume/optixtransferfunction.hpp"
#include "volume/optixbrickpool.hpp"

#include "utils/stats.hpp"

class OptixDVR
{
public:
    Stats mStats;
    optix::Context m_context;
    optix::Program m_volumedvrprogram;
    optix::Program m_volumeshadingdvrprogram;
    optix::Program m_program_aabb_hit;
    optix::Program m_program_aabb_bounds;
    optix::Geometry m_geometry_aabb;
    optix::Material m_volumedvrmaterial;
    optix::Material m_volumeshadingdvrmaterial;
    optix::GeometryGroup m_world;
    optix::Acceleration m_bvh;
    //std::vector<optix::GeometryInstance> m_optixbricks;
    std::vector<vec4f> mAABBMinData;
    std::vector<vec4f> mAABBMaxData;
    optix::Buffer m_aabbMinBuffer;
    optix::Buffer m_aabbMaxBuffer;
    optix::GeometryInstance m_volumegeometryinstance;
    optix::Geometry m_volumegeometry;

    std::string m_volumefilepath;
    std::string m_transferfuncpath;
    Volume *m_volume = nullptr;
    BrickedVolume *m_subdivision = nullptr;
    OptixVolumeBrickPool *mPool = nullptr;
    vec3size_t mPreviousBrickSize;
    OptixTransferFunction *m_transferfunction = nullptr;

    Camera m_camera;
    float m_lastrenderduration;
    float m_lastbvhbuildtime;
    int m_samples = 1;
    bool m_highlightert = false;
    bool m_showdepthcomplexity = false;
    bool m_showPageTableAccesses = false;

    bool m_dontsample = false;
    bool m_useshading = false;
    bool m_needsrecreate = false;
    vec3f m_lightposition = vec3f(5, 0, 0);

    int m_renderwidth = 1024;
    int m_renderheight = 1024;
    std::string m_outppmpath;

    optix::Buffer m_framebuffer;
    unsigned char* m_renderdata;
    std::chrono::time_point<std::chrono::system_clock> m_previousframetimepoint;

    bool m_ready = false;
    bool m_frameavailable = false;

    OptixDVR();

    void loadvolume(const char* volumepath);
    void loadtransferfunction(const char* tfpath);
    optix::GeometryInstance createAABB(const vec3f &boxCenter, const vec3f &boxRadius);
    void createScene();
    void renderFrame(int Nx, int Ny);
    optix::Buffer createFrameBuffer(int Nx, int Ny);
    void setRayGenProgram();
    void setMissProgram();
    int setup();
    void updateScene();
    int render();
    void saveToPNG(const char* path);
    void resizeFrameBuffer(int w, int h);

    void volumeMaterial(
        optix::GeometryInstance &gi,
        const vec3f &boxMin,
        const vec3f &boxMax
    );

    void shadedVolumeMaterial(
        optix::GeometryInstance &gi,
        const vec3f &boxMin,
        const vec3f &boxMax,
        const vec3f &lpos
    );
};