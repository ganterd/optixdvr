#pragma once

#include <string.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define checkGLErrors() {\
    GLenum err = glGetError();\
    const GLubyte* errString;\
    while(err != GL_NO_ERROR)\
    {\
        errString = gluErrorString(err);\
        std::cerr << __FILENAME__ << ":" << __LINE__ << " - GL Error: " << errString << std::endl;\
        err = glGetError();\
    }\
}

#define GL(x) do{ x; checkGLErrors(); }while(0);