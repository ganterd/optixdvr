#pragma once

#include "../nuklear_glfw_gl4.h"
#include "../../../optixdvr/optixdvr_instance.hpp"

class TransferFunctionGui
{
    public:
    static void gui(struct nk_context *ctx, OptixDVR* renderer, bool& updateRenderer)
    {
        if (nk_tree_push(ctx, NK_TREE_TAB, "Transfer Function", NK_MAXIMIZED))
        {
            nk_layout_row_dynamic(ctx, 20, 1);
            if (nk_button_label(ctx, "Open Transfer Function..."))
            {
	            char const * filters[1] = { "*.itf"  };
                char const * itffilepath = tinyfd_openFileDialog(
                    "Open ITF File...",
                    "",
                    1,
                    filters,
                    NULL,
                    0
                );

                if(itffilepath)
                {
                    renderer->loadtransferfunction(itffilepath);
                    updateRenderer = true;
                }
            }

            TransferFunction* tf = renderer->m_transferfunction;

            struct nk_window *win = ctx->current;
            struct nk_command_buffer *out = &win->buffer;
            nk_layout_row_dynamic(ctx, 200, 1);
            struct nk_rect bounds = nk_widget_bounds(ctx);
            nk_fill_rect(out, bounds, 0, nk_rgb(100,100,100));
            if(tf)
            {
                std::vector<TransferFunctionPoint> &points = tf->points();
                float id = 0;

                static int col_index = -1;
                static int line_index = -1;
                float step = 1.0f / points.size();

                int i;
                int index = -1;

                /* line chart */
                id = 0;
                index = -1;
                // nk_layout_row_dynamic(ctx, 100, 1);

                static float zoom_min = 0.0f;
                static float zoom_max = 1.0f;
                nk_layout_row_dynamic(ctx, 20, 2);
                nk_property_float(ctx, "Zoom Min:", 0.0f, &zoom_min, zoom_max, 0.001f, 0.001f);
                nk_property_float(ctx, "Zoom Max:", zoom_min, &zoom_max, 1.0f, 0.001f, 0.001f);



                /* mixed colored chart */


                vec2f pMin(bounds.x, bounds.y + bounds.h);
                vec2f pMax(bounds.x + bounds.w, bounds.y);
                vec2f pSize(bounds.w, -bounds.h);

                static int selectedNode = -1;
                struct nk_input *input = &ctx->input;

                vec2f point(0.0f, 0.0f);
                if(points.size())
                {
                    point = vec2f(points[0].mIntensity, points[0].mColour.w);
                }
                vec2f p = pMin + pSize * point;
                vec2f p_prev(pMin.x, p.y);

                /* Draw lines first so they're at the back */
                for(int i = 0; i < points.size(); ++i)
                {
                    point.x = (points[i].mIntensity - zoom_min) * (1.0f / (zoom_max - zoom_min));
                    point.y = points[i].mColour.w;
                    p = pMin + pSize * point;
                    nk_stroke_line(out, p_prev.x, p_prev.y, p.x, p.y, 2, nk_rgb(0, 0, 0));
                    p_prev = p;
                }
                nk_stroke_line(out, p.x, p.y, pMax.x, p.y, 2, nk_rgb(0, 0, 0));

                /* Draw boxes for all the points */
                for(int i = 0; i < points.size(); ++i)
                {
                    point.x = (points[i].mIntensity - zoom_min) * (1.0f / (zoom_max - zoom_min));
                    point.y = points[i].mColour.w;
                    p = pMin + pSize * point;

                    struct nk_rect rect;
                    float size = 10.0f;
                    rect.x = p.x - size * 0.5f;
                    rect.y = p.y - size * 0.5f;
                    rect.w = size;
                    rect.h = size;

                    struct nk_rect actualSelectableRegion = bounds;
                    actualSelectableRegion.x -= size * 0.5f;
                    actualSelectableRegion.y -= size * 0.5f;
                    actualSelectableRegion.w += size;
                    actualSelectableRegion.h += size;
                    if(nk_input_has_mouse_click_down_in_rect(input, NK_BUTTON_LEFT, actualSelectableRegion, nk_true))
                    {
                        if(selectedNode == -1)
                        {
                            if(
                                nk_input_is_mouse_hovering_rect(input, rect)
                                || nk_input_is_mouse_prev_hovering_rect(input, rect)
                            ){
                                selectedNode = i;
                            }
                            else
                            {
                                selectedNode = -1;
                            }
                        }
                    }
                    else
                    {
                        selectedNode = -1;
                    }

                    if(nk_input_is_mouse_hovering_rect(input, rect) || selectedNode == i)
                    {
                        nk_fill_rect(out, rect, size * 0.5f, nk_rgb(255, 255, 255));
                    }
                    else
                    {
                        nk_fill_rect(out, rect, size * 0.5f, nk_rgb(
                            points[i].mColour.x * 255.0f,
                            points[i].mColour.y * 255.0f,
                            points[i].mColour.z * 255.0f
                        ));
                    }
                    nk_stroke_rect(out, rect, 4.0f, 2, nk_rgb(0, 0, 0));
                }

                /* Node Editing: If dragging a node, then determine    */
                /* the TF region that the cursor is in and update that */
                /* node's details. Should be clamped by TF min/max and */
                /* also clamped by previous and next nodes in TF       */
                /*                                                     */
                /* TODO: Add nodes                                     */
                /* TODO: Remove nodes                                  */
                /* TODO: Save TF                                       */
                /* TODO: Create TF                                     */
                if(selectedNode != -1)
                {
                    vec2f mousep(input->mouse.pos.x, input->mouse.pos.y);
                    mousep = (mousep - pMin) / pSize;
                    mousep.x = (mousep.x) * (zoom_max - zoom_min) + zoom_min;

                    float leftbound = selectedNode > 0 ? points[selectedNode - 1].mIntensity : 0.0f;
                    float rightbound = selectedNode < points.size() - 1 ? points[selectedNode + 1].mIntensity : 1.0f;

                    mousep.x = fmin(fmax(mousep.x, leftbound), rightbound);
                    mousep.y = fmin(fmax(mousep.y, 0.0f), 1.0f);

                    /* Update node */
                    points[selectedNode].mIntensity = mousep.x;
                    points[selectedNode].mColour.w = mousep.y;

                    //tf->updateTexture();
                    tf->updateLUT();
                    renderer->updateScene();
                    updateRenderer = true;
                }
            }

            nk_tree_pop(ctx);
        }
    }
};