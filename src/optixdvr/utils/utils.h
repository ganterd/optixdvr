#pragma once

#include <iostream>
#include <iomanip>
#include <chrono>

namespace utils
{
    class io
    {
    public:
        static const char* readFile(const char* filePath){
            char* text = nullptr;

            FILE *file = fopen(filePath, "r");

            if (file == NULL)
                return NULL;

            fseek(file, 0, SEEK_END);
            size_t count = ftell(file);
            rewind(file);


            if (count > 0) {
                text = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, file);
                text[count] = '\0';
            }
            fclose(file);

            return text;
        }
    };

    class Timer
    {
    private:
        std::chrono::time_point<std::chrono::system_clock> mStart;
        std::chrono::time_point<std::chrono::system_clock> mEnd;
        float mElapsed;

    public:

        void start()
        {
            mStart = std::chrono::system_clock::now();
        }

        float stop()
        {
            mEnd = std::chrono::system_clock::now();
            
            mElapsed = std::chrono::duration<float>(mEnd - mStart).count() * 1000.0f;
            return mElapsed;
        }

        float getTime() const { return mElapsed; }

        std::string toString() const
        {
            return std::to_string(mElapsed);
        }

        void reset()
        {
            mElapsed = 0.0f;
        }
    };
}