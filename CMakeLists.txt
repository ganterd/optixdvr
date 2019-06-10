project(OptixDVR)

cmake_minimum_required(VERSION 2.8)

include(cmake/configure_optix.cmake)

mark_as_advanced(CUDA_SDK_ROOT_DIR)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory(ext)
add_subdirectory(src)
