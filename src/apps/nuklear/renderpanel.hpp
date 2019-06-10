#include "nuklear_glfw_gl4.h"
#include "../../optixdvr/optixdvr.hpp"

#include <GLFW/glfw3.h>
#include <GL/glu.h>

class RenderPanel
{
protected:
    int mPreviousWidth;
    int mPreviousHeight;

    int mImageHeight;
    int mImageWidth;
    int m_textureid;
    struct nk_image img;

    int mBackgroundTextureID;
    struct nk_image mBackgroundImage;

    void render();
    void updateTexture();

    float mCameraRotationY;
    float mCameraRotationX;
    float mCameraRadius = 2.0f;
    vec3f mCameraPosition;
    vec3f mCameraLookAt;

public:
    RenderPanel();
    void setImage(char* data, int w, int h);
    int gui(struct nk_context *ctx, int windowWidth, int windowHeight);
};