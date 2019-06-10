#pragma once

#include "../nuklear_glfw_gl4.h"
#include "../../../optixdvr/optixdvr_instance.hpp"

class SubdivisionGui
{
    public:
    static void gui(struct nk_context *ctx, OptixDVR* renderer, bool& updateRenderer)
    {
        if (nk_tree_push(ctx, NK_TREE_TAB, "BVH Leaves", NK_MAXIMIZED))
        {
            size_t rowheight = 20;
            size_t namewidth = 100;
            size_t datawidth = 70;

            vec3size_t brickSize = renderer->m_subdivision->mVoxelsPerBrick;
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 5);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Leaf Size: ", NK_TEXT_LEFT);

            static bool updating = false;
            static char text[3][5];
            static int text_len[3];
            static int newbrickdims[3];
            if(updating)
            {
                vec3size_t maxSize = renderer->m_volume->dataDimensions;
                nk_layout_row_push(ctx, datawidth);
                nk_property_int(ctx, "x", 4, &newbrickdims[0], maxSize.x, 8, 8);
                nk_layout_row_push(ctx, datawidth);
                nk_property_int(ctx, "y", 4, &newbrickdims[1], maxSize.y, 8, 8);
                nk_layout_row_push(ctx, datawidth);
                nk_property_int(ctx, "z", 4, &newbrickdims[2], maxSize.z, 8, 8);
                nk_layout_row_push(ctx, 60);
                if(nk_button_label(ctx, "Set"))
                {
                    vec3size_t s(newbrickdims[0], newbrickdims[1], newbrickdims[2]);
                    renderer->m_subdivision->set_brick_size(s);
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


            vec3size_t bricks = renderer->m_subdivision->mNumLeaves;
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Num Leaves: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(bricks.x).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(bricks.y).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(bricks.z).c_str(), NK_TEXT_LEFT);

            size_t total_bricks = renderer->m_subdivision->m_total_subdivisions;
            size_t active_bricks = renderer->m_subdivision->m_active_subdivisions;
            nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
            nk_layout_row_push(ctx, namewidth);
            nk_label(ctx, "Active Leaves: ", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(active_bricks).c_str(), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, "/", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, datawidth);
            nk_label(ctx, std::to_string(total_bricks).c_str(), NK_TEXT_LEFT);

            nk_layout_row_static(ctx, 30, 200, 1);
            int cluster = renderer->m_subdivision->mCluster;
            nk_checkbox_label(ctx, "Cluster", &cluster);
            if((cluster == 1) != renderer->m_subdivision->mCluster)
            {
                renderer->m_subdivision->mCluster = (cluster == 1);
                renderer->m_subdivision->set_brick_size(renderer->m_subdivision->mVoxelsPerBrick);
                renderer->updateScene();
                updateRenderer = true;
            }

            if(renderer->m_subdivision->mCluster)
            {
                size_t numClusters = renderer->m_subdivision->mStats.get("numclusters");
                nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
                nk_layout_row_push(ctx, namewidth);
                nk_label(ctx, "Clusters: ", NK_TEXT_LEFT);
                nk_layout_row_push(ctx, datawidth);
                nk_label(ctx, std::to_string(numClusters).c_str(), NK_TEXT_LEFT);
            }

            nk_tree_pop(ctx);
        }
    };
};