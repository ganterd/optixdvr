#pragma once

#include "../nuklear_glfw_gl4.h"
#include "../../../optixdvr/optixdvr_instance.hpp"

class CameraGui
{
    public:
    static void gui(struct nk_context *ctx, OptixDVR* renderer)
    {
        if (nk_tree_push(ctx, NK_TREE_TAB, "Camera", NK_MINIMIZED))
        {
            size_t rowheight = 20;
            size_t namewidth = 100;
            size_t datawidth = 70;

            // Camera Origin
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Origin: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mOrigin.x).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mOrigin.y).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mOrigin.z).c_str(), NK_TEXT_LEFT);

            // Camera Direction
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Direction: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mLookDirection.x).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mLookDirection.y).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mLookDirection.z).c_str(), NK_TEXT_LEFT);

            // Field-of-view
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Fov: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mFOVY).c_str(), NK_TEXT_LEFT);

            // Aspect Ratio
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Aspect: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mAspect).c_str(), NK_TEXT_LEFT);

            // Aperture
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Aperture: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(renderer->m_camera.mAperture).c_str(), NK_TEXT_LEFT);

            nk_tree_pop(ctx);
        }
    };
};