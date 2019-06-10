# OptiX BVH Direct Volume Rendering

Experimental implementation of our High Performance Graphics 2019 paper "An Analysis of Region Clustered BVH Volume Rendering on GPU".
This code implements direct volume rendering using OptiX BVH traversal as the means of empty-space-skipping (ESS).
In order to accelerate the rendering, we reduce BVH leaf complexity by clustering neighbouring bricks.
See paper for more information.

## Requirements
- CUDA 10.1+ (https://developer.nvidia.com/cuda-downloads)
- OptiX SDK 6.0+ (https://developer.nvidia.com/optix)
- Nvidia RTX compatible driver
- Python3 (for pybind bindings)

## Notes
- Not the most stable implementation, will probably crash if you try to open a second volume in the same session.
- Can currently only open MHD volume files, but can open more types with a little extra implementation.
- Opens Inviwo's (https://inviwo.org/) .itf transfer functions.
- The starting place for this code was Ingo Wald's OptiX version of Ray-Tracing in One Weekend (https://github.com/ingowald/RTOW-OptiX), hence his name being in a few places.
