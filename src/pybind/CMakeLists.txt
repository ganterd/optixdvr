
pybind11_add_module(pyoptixdvr
  # C++ host code
  #optixprograms/vec.h
  #optixprograms/material.h
  optixdvrbindings.cpp

  ../optixdvr/utils/tinyxml2.cpp
  ../optixdvr/utils/argparse.cpp
  ../optixdvr/volume/brickedvolume.cpp
  ../optixdvr/volume/brickpool.cpp
  ../optixdvr/volume/optixbrickpool.cpp
  ../optixdvr/volume/transferfunction.cpp
  ../optixdvr/optixdvr.cpp
  ../optixdvr/optixdvr_instance.cpp

  # embedded cuda kernels:
  ${embedded_raygen_program}
  ${embedded_aabb_program}
  ${embedded_volume_program}
  ${embedded_volume_shading_program}
  ${embedded_miss_program}
)

target_link_libraries(pyoptixdvr PRIVATE
  ${optix_LIBRARY}
  ${CUDA_LIBRARIES}
  ${PNG_LIBRARIES}
)

add_executable(optixdvr_py optixdvr_py.cpp)
target_link_libraries(optixdvr_py PRIVATE pybind11::embed)