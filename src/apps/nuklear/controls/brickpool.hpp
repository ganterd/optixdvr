#pragma once

#include "../nuklear_glfw_gl4.h"
#include "../../../optixdvr/optixdvr_instance.hpp"

class BrickPoolGui
{
    public:
    static void gui(struct nk_context *ctx, OptixDVR* renderer, bool& updateRenderer)
    {
        if (nk_tree_push(ctx, NK_TREE_TAB, "Volume Pool", NK_MAXIMIZED))
        {
            size_t rowheight = 20;
            size_t namewidth = 100;
            size_t datawidth = 70;

            vec3size_t poolDim = renderer->mPool->mDataDimensions;
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Pool Size: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(poolDim.x).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(poolDim.y).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(poolDim.z).c_str(), NK_TEXT_LEFT);

            vec3size_t brickSize = renderer->mPool->mBrickSize;
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 5);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Brick Size: ", NK_TEXT_LEFT);

            static bool updating = false;
            static char text[3][5];
            static int text_len[3];
            static int newbrickdims[3];
            if(updating)
            {

                nk_layout_row_push(ctx, datawidth);
                nk_property_int(ctx, "x", 8, &newbrickdims[0], 512, 8, 8);
                nk_layout_row_push(ctx, datawidth);
                nk_property_int(ctx, "y", 8, &newbrickdims[1], 512, 8, 8);
                nk_layout_row_push(ctx, datawidth);
                nk_property_int(ctx, "z", 8, &newbrickdims[2], 512, 8, 8);
                nk_layout_row_push(ctx, 60);
                if(nk_button_label(ctx, "Set"))
                {
                    vec3size_t s(newbrickdims[0], newbrickdims[1], newbrickdims[2]);
                    renderer->mPool->set_brick_size(s);
                    renderer->updateScene();
                    updating = false;
                };
            }
            else
            {
                nk_layout_row_push(ctx, datawidth);
                nk_label(ctx, std::to_string(brickSize.x).c_str(), NK_TEXT_LEFT);
                nk_layout_row_push(ctx, datawidth);
                nk_label(ctx, std::to_string(brickSize.y).c_str(), NK_TEXT_LEFT);
                nk_layout_row_push(ctx, datawidth);
                nk_label(ctx, std::to_string(brickSize.z).c_str(), NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 60);
                if(nk_button_label(ctx, "Change"))
                {
                    newbrickdims[0] = brickSize.x;
                    newbrickdims[1] = brickSize.y;
                    newbrickdims[2] = brickSize.z;
                    updating = true;
                };
            }


            vec3size_t bricks = renderer->mPool->mNumBricks;
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Num Bricks: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(bricks.x).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(bricks.y).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(bricks.z).c_str(), NK_TEXT_LEFT);

            size_t total_bricks = renderer->mPool->mStats.get("numbricks");
            size_t active_bricks = renderer->mPool->mStats.get("numactivebricks");
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Active Bricks: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(active_bricks).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, "/", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(total_bricks).c_str(), NK_TEXT_LEFT);

            size_t poolSlotsUsed = renderer->mPool->mNextUploadSlot;
            size_t poolSlotsTotal = renderer->mPool->mTotalPoolBrickSlots;
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Pool Slots: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(poolSlotsUsed).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, "/", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(poolSlotsTotal).c_str(), NK_TEXT_LEFT);

            nk_tree_pop(ctx);
        }
    };
};