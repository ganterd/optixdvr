#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#include "../programs/vec.h"
#include "volume.hpp"

static void splitFilename(const std::string& str, std::string& dir, std::string& file)
{
	size_t found;
	found = str.find_last_of("/\\");
	dir = str.substr(0, found);
	file = str.substr(found + 1);
}

#define READ_BUFFER_SIZE 256
#if defined(_WIN32) || defined(_WIN64)
#define STRING_COMPARE(a, b) _stricmp (a, b)
#else
#include <string.h>
#define STRING_COMPARE(a, b) strcasecmp(a, b)
#endif

#define PARSE_ARGUMENTS(SOURCE_STRING, FAIL_STRING, RETURN_CODE, DATA_DESCRIPTOR, ...) \
		if(!sscanf(SOURCE_STRING, DATA_DESCRIPTOR, __VA_ARGS__)) \
		{ \
			printf(FAIL_STRING); \
			return RETURN_CODE;  \
		}

class MHDHeaderReader
{
public:
    static VolumeFile Load(const char* filename)
    {
        // Open the file
        char line_buffer [READ_BUFFER_SIZE];
        char token_buffer[READ_BUFFER_SIZE];
        char test_string [READ_BUFFER_SIZE];
        VolumeFile temp_model_info;
        FILE* f;
        int num_lines, num_valid_lines, num_chars_read;
        int test, data_valid;

        // Report messages
        char missing [] = "Missing data in file, exiting...\n";
        char invalid[] = "Data is invalid, exiting...\n";

        num_lines = num_valid_lines = 0;

        f = fopen(filename, "r");

        if(f == NULL)
        {
            std::cerr << "Couldn't open MHD file " << filename << std::endl;
            return temp_model_info;
        }

        std::string fname;
        std::string dir;
        splitFilename(filename, dir, fname);
        temp_model_info.directory = dir;

        while(fgets(line_buffer, READ_BUFFER_SIZE, f)!= NULL)
        {
            if(sscanf(line_buffer, "%s", token_buffer))
            {
                num_lines++;

                if(!STRING_COMPARE(token_buffer, "DimSize"))
                {
                    if(!handlerDimension(line_buffer, temp_model_info))
                        return temp_model_info;
                }
                else if(!STRING_COMPARE(token_buffer, "ElementSpacing"))
                {
                    if(!handlerElementSpacing(line_buffer, temp_model_info))
                        return temp_model_info;
                }
                else if(!STRING_COMPARE(token_buffer, "ElementDataFile"))
                {
                    if(!mhd_ElementDataFile_handler(&line_buffer[strlen(token_buffer)], temp_model_info))
                        return temp_model_info;
                }
                else if(!STRING_COMPARE(token_buffer, "ElementType"))
                {
                    if(!mhd_ElementType_handler(&line_buffer[strlen(token_buffer)], temp_model_info))
                        return temp_model_info;
                }
                else
                {
                    //std::cerr << "Unrecognized token '" << token_buffer << "', skipping..." << std::endl;
                    continue;
                }

                num_valid_lines++;
            }
            else
            {
                std::cerr << "Invalid or empty input line, skipping..." << std::endl;
            }
        }

        return temp_model_info;
    }

private:
    static bool handlerDimension(char* line_buffer, VolumeFile& model_info)
    {
        if(
            !sscanf(line_buffer, "%*[^0-9]%f%*[^0-9]%f%*[^0-9]%f",
                &model_info.dataDimensions.x,
                &model_info.dataDimensions.y,
                &model_info.dataDimensions.z
            )
        )
        {
            std::cerr << "Data missing for volume dimensions" << std::endl;
            return false;
        }

        if(!(model_info.dataDimensions.x >= 0) || !(model_info.dataDimensions.y >= 0) || !(model_info.dataDimensions.z >= 0))
        {
            std::cerr << "Invalid data dimensions given for volume" << std::endl;
            return false;
        }

        return true;
    }

    static bool handlerElementSpacing(char* line_buffer, VolumeFile& model_info)
    {
        if(
            !sscanf(line_buffer, "%*[^0-9]%f%*[^0-9]%f%*[^0-9]%f",
                &model_info.elementSpacing.x,
                &model_info.elementSpacing.y,
                &model_info.elementSpacing.z
            )
        )
        {
            std::cerr << "Data missing for element spacing" << std::endl;
            return false;
        }

        if(!(model_info.elementSpacing.x >= 0) || !(model_info.elementSpacing.y >= 0) || !(model_info.elementSpacing.z >= 0))
        {
            std::cerr << "Invalid element spacing given for volume" << std::endl;
            return false;
        }

        return true;
    }

    static bool mhd_ElementDataFile_handler(char* line_buffer, VolumeFile& model_info)
    {
        char buffer[READ_BUFFER_SIZE];
        if(!sscanf(line_buffer, "%*[^A-Za-z.0-9_]%s", buffer))
        {
            std::cerr << "No data given for data file" << std::endl;
            return false;
        }
        std::stringstream ss;
        ss << model_info.directory << "/" << buffer;
        model_info.volumes.push_back(ss.str());;
        return true;
    }

    static bool mhd_ElementType_handler(char* line_buffer, VolumeFile& model_info)
    {
        char buffer[READ_BUFFER_SIZE];
        if(!sscanf(line_buffer, "%*[^A-Za-z_]%s", buffer))
        {
            std::cerr << "No data given for element type" << std::endl;
            return false;
        }

        if(!STRING_COMPARE(buffer, "MET_CHAR"))
        {
            model_info.type = Volume::CHAR;
            model_info.bytesPerElement = 1;
        }
        else if(!STRING_COMPARE(buffer, "MET_UCHAR"))
        {
            model_info.type = Volume::UCHAR;
            model_info.bytesPerElement = 1;
        }
        else if(!STRING_COMPARE(buffer, "MET_SHORT"))
        {
            model_info.type = Volume::SHORT;
            model_info.bytesPerElement = 2;
        }
        else if(!STRING_COMPARE(buffer, "MET_USHORT"))
        {
            model_info.type = Volume::USHORT;
            model_info.bytesPerElement = 2;
        }
        else if(!STRING_COMPARE(buffer, "MET_INT"))
        {
            model_info.type = Volume::INT;
            model_info.bytesPerElement = 4;
        }
        else if(!STRING_COMPARE(buffer, "MET_UINT"))
        {
            model_info.type = Volume::UINT;
            model_info.bytesPerElement = 4;
        }
        else if(!STRING_COMPARE(buffer, "MET_LONG"))
        {
            model_info.type = Volume::LONG;
            model_info.bytesPerElement = 8;
        }
        else if(!STRING_COMPARE(buffer, "MET_ULONG"))
        {
            model_info.type = Volume::ULONG;
            model_info.bytesPerElement = sizeof(unsigned long);
        }
        else if (!STRING_COMPARE(buffer, "MET_FLOAT"))
        {
            model_info.type = Volume::FLOAT;
            model_info.bytesPerElement = sizeof(float);
        }
        else
        {
            std::cerr << "Unknown data type '" << buffer << "'" << std::endl;
            return false;
        }

        return true;
    }

};