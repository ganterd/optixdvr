// ======================================================================== //
// Copyright 2018 Ingo Wald                                                 //
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

#include "material.h"

/*! the implicit state's ray we will intersect against */
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
/*! the per ray data we operate on */
rtDeclareVariable(PerRayData, prd, rtPayload, );

RT_PROGRAM void miss_program()
{
  vec4f backgroundColour(vec3f(0.0f), 0.0f);
  backgroundColour.x *= (1.0f - prd.out.accumulation.w);
  backgroundColour.y *= (1.0f - prd.out.accumulation.w);
  backgroundColour.z *= (1.0f - prd.out.accumulation.w);
  prd.out.accumulation += backgroundColour;
  prd.rayTerminated = true;
  //prd.out.accumulation.w = 1.0f;
}
