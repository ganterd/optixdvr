#include "renderpanel.hpp"
#include "../../optixdvr/programs/vec.h"
#include "../../optixdvr/optixdvr_instance.hpp"

RenderPanel::RenderPanel()
{
    mImageWidth = 512;
    mImageHeight = 512;
    mPreviousWidth = 512;
    mPreviousHeight = 512;
    m_textureid = -1;


    // Camera Parameters
    mCameraLookAt = vec3f(0, 0, 0);
    mCameraRotationX = 0.0f;
    mCameraRotationY = 0.0f;

    OptixDVR* renderer = OptixInstance::get();
    renderer->m_camera.origin(mCameraPosition);
    renderer->m_camera.lookat(mCameraLookAt);

    // Volume Parameters
    renderer->m_useshading = 0;
    renderer->m_highlightert = 0;
    renderer->m_showdepthcomplexity = 0;

    // Output Parameters
    renderer->m_renderwidth = mImageWidth;
    renderer->m_renderheight = mImageHeight;
    renderer->m_samples = 1;

    char tmpData[] = {(char)0, (char)0, (char)0, (char)255};
    mBackgroundTextureID = nk_glfw3_create_texture(tmpData, 1, 1);
    mBackgroundImage = nk_image_id(mBackgroundTextureID);
}

void RenderPanel::render()
{
    vec3f v(0, 0, 1);
    v = rotationX(mCameraRotationY * 0.0174533f) * v;
    v = rotationY(mCameraRotationX * 0.0174533f) * v;
    v = normalize(v) * mCameraRadius;
    mCameraPosition = v + mCameraLookAt;

    OptixDVR* renderer = OptixInstance::get();
    renderer->resizeFrameBuffer(mImageWidth, mImageHeight);
    renderer->m_camera.origin(mCameraPosition);
    renderer->m_camera.lookat(mCameraLookAt);
    renderer->render();
}

void RenderPanel::updateTexture()
{
    static std::chrono::time_point<std::chrono::system_clock> previoustime;
    OptixDVR* renderer = OptixInstance::get();
    if(renderer->m_frameavailable)
    {
        if(previoustime != renderer->m_previousframetimepoint)
        {
            if(m_textureid != -1)
                nk_glfw3_destroy_texture(m_textureid);
            m_textureid = nk_glfw3_create_texture(renderer->m_renderdata, mImageWidth, mImageHeight);
        }
    }
    else
    {
        if(m_textureid != -1)
                nk_glfw3_destroy_texture(m_textureid);
        char tmpData[] = {(char)20,(char)20,(char)20,(char)20};
        m_textureid = nk_glfw3_create_texture(tmpData, 1, 1);
    }
    previoustime = renderer->m_previousframetimepoint;


    img = nk_image_id(m_textureid);
}

int RenderPanel::gui(struct nk_context *ctx, int windowWidth, int windowHeight)
{
    /* window flags */
    if (nk_begin(ctx, "Render Window", nk_rect(400, 0, windowWidth - 400, windowHeight),
        NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR
    ))
    {
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
        struct nk_rect total_space = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, total_space.h - 10, 1);
        struct nk_rect widgetBounds = nk_widget_bounds(ctx);
        if(nk_widget_has_mouse_click_down(ctx, NK_BUTTON_LEFT, nk_true))
        {
            mCameraRotationX -= (float)ctx->input.mouse.delta.x;
            mCameraRotationY += (float)ctx->input.mouse.delta.y;
            render();
        }
        else if(nk_widget_has_mouse_click_down(ctx, NK_BUTTON_RIGHT, nk_true))
        {
            mCameraRadius += (float)ctx->input.mouse.delta.y * 0.1f;
            render();
        }
        else if(nk_widget_has_mouse_click_down(ctx, NK_BUTTON_MIDDLE, nk_true))
        {
            vec3f lookvector = normalize(mCameraLookAt - mCameraPosition);
            vec3f right = normalize(cross(lookvector, vec3f(0, 1, 0)));
            vec3f up = normalize(cross(right, lookvector));

            float upchange = -(float)ctx->input.mouse.delta.y * 0.001f;
            float rightchange = -(float)ctx->input.mouse.delta.x * 0.001f;

            mCameraLookAt = mCameraLookAt + right * rightchange + up * upchange;

            render();
        }
        else if(widgetBounds.w != mImageWidth || widgetBounds.h != mImageHeight)
        {
            mImageWidth = widgetBounds.w;
            mImageHeight = widgetBounds.h;
            render();
        }
        struct nk_color white = {255,255,255,255};
        updateTexture();
        nk_draw_image(canvas, widgetBounds, &mBackgroundImage, white);
        nk_draw_image(canvas, widgetBounds, &img, white);

    }
    nk_end(ctx);
    return !nk_window_is_closed(ctx, "Render Window");
}
