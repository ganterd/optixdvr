set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH on)

# Build embeddable CUDA programs
cuda_compile_and_embed(embedded_raygen_program optixdvr/programs/raygen.cu)
cuda_compile_and_embed(embedded_aabb_program optixdvr/programs/aabb.cu)
cuda_compile_and_embed(embedded_volume_program optixdvr/programs/volume.cu)
cuda_compile_and_embed(embedded_volume_shading_program optixdvr/programs/volume_shading.cu)
cuda_compile_and_embed(embedded_miss_program optixdvr/programs/miss.cu)

option(USE_OMP "Use OpenMP where possible and specified" ON)
if(USE_OMP)
	find_package(OpenMP)
	if(OPENMP_FOUND)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
	endif()
endif()

find_package(PNG)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(optixdvr_cli
  # C++ host code
  optixdvr/utils/tinyxml2.cpp
  optixdvr/utils/argparse.cpp
  optixdvr/volume/brickedvolume.cpp
  optixdvr/volume/brickpool.cpp
  optixdvr/volume/optixbrickpool.cpp
  optixdvr/volume/transferfunction.cpp
  optixdvr/optixdvr.cpp
  optixdvr/optixdvr_instance.cpp

  # CLI App
  apps/cli/main.cpp

  # Embedded kernels
  ${embedded_raygen_program}
  ${embedded_aabb_program}
  ${embedded_volume_program}
  ${embedded_volume_shading_program}
  ${embedded_miss_program}
)

target_link_libraries(optixdvr_cli
  ${optix_LIBRARY}
  ${CUDA_LIBRARIES}
  ${PNG_LIBRARIES}
)

target_include_directories(optixdvr_cli PUBLIC
  ./
)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(optixdvr_nuklear
  # C++ host code
  optixdvr/utils/tinyxml2.cpp
  optixdvr/utils/argparse.cpp
  optixdvr/optixdvr.cpp
  optixdvr/optixdvr_instance.cpp
  optixdvr/volume/brickedvolume.cpp
  optixdvr/volume/brickpool.cpp
  optixdvr/volume/optixbrickpool.cpp
  optixdvr/volume/transferfunction.cpp

  # GUI Elements
  apps/nuklear/tinyfiledialogs.c

  apps/nuklear/overview.cpp
  apps/nuklear/renderpanel.cpp

  apps/nuklear/optixdvr_nuklear_app.cpp

  # embedded cuda kernels
  ${embedded_raygen_program}
  ${embedded_aabb_program}
  ${embedded_volume_program}
  ${embedded_volume_shading_program}
  ${embedded_miss_program}
)
target_include_directories(optixdvr_nuklear PUBLIC
  "apps/nuklear/controls/"
)



add_library(optixlib SHARED IMPORTED)
set_target_properties(optixlib PROPERTIES
	IMPORTED_LOCATION  ${optix_LIBRARY}
)

target_link_libraries(optixdvr_nuklear
	PUBLIC
	${OPENGL_LIBRARIES}
	GLEW
	glfw
	${optix_LIBRARY}
	${CUDA_LIBRARIES}
	${PNG_LIBRARIES}
)

if(UNIX)
  install(TARGETS optixdvr_cli
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
  )

  file(GLOB optix_lib_files ${optix_LIBRARY}*)
  install(
    FILES ${optix_lib_files}
    DESTINATION lib
    PERMISSIONS
      OWNER_READ
      GROUP_READ
      WORLD_READ
      OWNER_WRITE
      GROUP_WRITE
      WORLD_WRITE
      OWNER_EXECUTE
      GROUP_EXECUTE
      WORLD_EXECUTE
  )

  install(
    FILES "../scripts/experiment.sh"
    DESTINATION scripts
    PERMISSIONS
      OWNER_READ
      GROUP_READ
      WORLD_READ
      OWNER_WRITE
      GROUP_WRITE
      WORLD_WRITE
      OWNER_EXECUTE
      GROUP_EXECUTE
      WORLD_EXECUTE
  )
endif()

add_subdirectory(pybind)