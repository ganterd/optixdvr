#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../optixdvr/optixdvr_instance.hpp"


namespace py = pybind11;
PYBIND11_MODULE(pyoptixdvr, m)
{
    /* Bindings for vector classes */
    py::class_<vec3f> pyVec3f(m, "vec3f");
    pyVec3f.def(py::init<float>());
    pyVec3f.def(py::init<float, float, float>());
    pyVec3f.def_readwrite("x", &vec3f::x);
    pyVec3f.def_readwrite("y", &vec3f::y);
    pyVec3f.def_readwrite("z", &vec3f::z);
    m.def("normalize", (vec3f (*)(const vec3f&)) &normalize);

    py::class_<vec3size_t> pyVec3i(m, "vec3i");
    pyVec3i.def(py::init<int>());
    pyVec3i.def(py::init<int, int, int>());
    pyVec3i.def_readwrite("x", &vec3size_t::x);
    pyVec3i.def_readwrite("y", &vec3size_t::y);
    pyVec3i.def_readwrite("z", &vec3size_t::z);

    /* Bindings for Stats class*/
    py::class_<Stats> pyStats(m, "Stats");
    pyStats.def("has", &Stats::has);
    pyStats.def("get", &Stats::get);
    pyStats.def("list", &Stats::list);

    /* Bindings for Brick Pool */
    py::class_<VolumeBrickPool> pyBrickPool(m, "VolumeBrickPool");
    pyBrickPool.def("setBrickSize", &VolumeBrickPool::set_brick_size, py::arg("brickSize")=vec3size_t(32), py::arg("padding")=vec3size_t(1));
    pyBrickPool.def_readwrite("stats", &VolumeBrickPool::mStats);

    /* Bindings for Sub Division */
    py::class_<BrickedVolume> pyBrickedVolume(m, "VolumeSubdivision");
    pyBrickedVolume.def("setLeafSize", &BrickedVolume::set_brick_size);
    pyBrickedVolume.def_readwrite("cluster", &BrickedVolume::mCluster);
    pyBrickedVolume.def_readwrite("stats", &BrickedVolume::mStats);

    /* Bindings for the Camear class */
    py::class_<Camera> pyCamera(m, "Camera");
    pyCamera.def(py::init());
    pyCamera.def("origin", &Camera::origin);
    pyCamera.def("lookdir", (void (Camera::*)(const vec3f&)) &Camera::lookdir);

    /* Bindings for the renderer itself */
    py::class_<OptixDVR> pyOptixDVR(m, "OptixDVR");
    pyOptixDVR.def("loadVolume", &OptixDVR::loadvolume);
    pyOptixDVR.def("loadTransferFunction", &OptixDVR::loadtransferfunction);
    pyOptixDVR.def("render", &OptixDVR::render);
    pyOptixDVR.def("updateScene", &OptixDVR::updateScene);
    pyOptixDVR.def("resizeFrameBuffer", &OptixDVR::resizeFrameBuffer);
    pyOptixDVR.def("updateScene", &OptixDVR::updateScene);
    pyOptixDVR.def("saveToPNG", &OptixDVR::saveToPNG);
    pyOptixDVR.def_readwrite("subdivision", &OptixDVR::m_subdivision);
    pyOptixDVR.def_readwrite("pool", &OptixDVR::mPool);
    pyOptixDVR.def_readwrite("stats", &OptixDVR::mStats);
    pyOptixDVR.def_readwrite("camera", &OptixDVR::m_camera);
    pyOptixDVR.def_readwrite("highlightERT", &OptixDVR::m_highlightert);
    pyOptixDVR.def_readwrite("showDepthComplexity", &OptixDVR::m_showdepthcomplexity);

    /* Bindings for DVR instance (should be used to get a renderer) */
    py::class_<OptixInstance> pyOptixInstance(m, "instance");
    pyOptixInstance.def_static("get", &OptixInstance::get);
}