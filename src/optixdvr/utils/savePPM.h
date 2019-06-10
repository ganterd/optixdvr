// ======================================================================== //
// Copyright 2018 Ingo Wald                                                 //
// Modified 2019 David Ganter                                               //
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

#pragma once

// ooawe
#include "../programs/vec.h"
// std
#include <fstream>
#include <vector>
#include <assert.h>
#include <png.h>

/*! saving to a 'P3' type PPM file (255 values per channel).  There
  are many other, more C++-like, ways of writing this; this version is
  intentionally written as close to the RTOW version as possible */
inline void savePPM(const std::string &fileName,
                    const size_t Nx, const size_t Ny, const unsigned char *pixels, const float range=255.0f)
{
  std::ofstream file(fileName);
  assert(file.good());

  file << "P6\n" << Nx << " " << Ny << "\n" << 255 << "\n";
  for (int iy=(int)Ny-1; iy>=0; --iy)
    for (int ix=0; ix<(int)Nx; ix++) {
      char ir = pixels[(ix + Nx * iy) * 4 + 0];
      char ig = pixels[(ix + Nx * iy) * 4 + 1];
      char ib = pixels[(ix + Nx * iy) * 4 + 2];
      file << (char)ir << (char)ig << (char)ib;
    }
  assert(file.good());
}

inline void savePNG(
  const std::string &fileName,
  const size_t Nx, 
  const size_t Ny, 
  const unsigned char *pixels
){
  /* create file */
  FILE *fp = fopen(fileName.c_str(), "wb");
  if (!fp)
  {
    printf("[write_png_file] File %s could not be opened for writing\n", fileName.c_str());
    return;
  }


  /* initialize stuff */
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
  {
    printf("[write_png_file] png_create_write_struct failed\n");
    return;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    printf("[write_png_file] png_create_info_struct failed\n");
    return;
  }

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    printf("[write_png_file] Error during init_io\n");
    return;
  }

  png_init_io(png_ptr, fp);


  /* write header */
  if (setjmp(png_jmpbuf(png_ptr)))
  {
    printf("[write_png_file] Error during writing header\n");
    return;
  }

  png_set_IHDR(
    png_ptr, info_ptr, 
    Nx, Ny,
    8, PNG_COLOR_TYPE_RGB_ALPHA, 
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE, 
    PNG_FILTER_TYPE_BASE
  );

  png_write_info(png_ptr, info_ptr);


  /* write bytes */
  if (setjmp(png_jmpbuf(png_ptr)))
  {
    printf("[write_png_file] Error during writing bytes\n");
    return;
  }

  for(int y = Ny - 1; y >= 0; --y)
  {
    png_write_row(png_ptr, &pixels[(Nx * y) * 4]);
  }
  //png_write_image(png_ptr, (png_bytep*)pixels);
  png_write_end(png_ptr, NULL);
  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

  fclose(fp);
}