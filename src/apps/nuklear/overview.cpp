#include "overview.hpp"
#include "../../optixdvr/optixdvr_instance.hpp"
#include "tinyfiledialogs.h"
#include "controls/camera.hpp"
#include "controls/transferfunction.hpp"
#include "controls/brickpool.hpp"
#include "controls/subdivision.hpp"

int Overview::gui(struct nk_context *ctx, int windowWidth, int windowHeight)
{
    /* window flags */
    static int show_menu = nk_true;
    static nk_flags window_flags = 0;
    static int minimizable = nk_true;

    /* window flags */
    ctx->style.window.header.align = NK_HEADER_RIGHT;

    if (nk_begin(ctx, "Settings", nk_rect(0, 0, 400, windowHeight), window_flags))
    {
        OptixDVR* renderer = OptixInstance::get();
        bool updateRenderer = false;
        if (show_menu)
        {
            /* menubar */
            enum menu_states {MENU_DEFAULT, MENU_WINDOWS};
            static nk_size mprog = 60;
            static int mslider = 10;
            static int mcheck = nk_true;
            nk_menubar_begin(ctx);

            /* menu #1 */
            nk_layout_row_begin(ctx, NK_STATIC, 25, 5);
            nk_layout_row_push(ctx, 45);
            if (nk_menu_begin_label(ctx, "MENU", NK_TEXT_LEFT, nk_vec2(120, 200)))
            {
                static size_t prog = 40;
                static int slider = 10;
                static int check = nk_true;
                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_menu_item_label(ctx, "Hide", NK_TEXT_LEFT))
                    show_menu = nk_false;
                nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
                nk_slider_int(ctx, 0, &slider, 16, 1);
                nk_checkbox_label(ctx, "check", &check);
                nk_menu_end(ctx);
            }
            /* menu #2 */
            nk_layout_row_push(ctx, 60);
            if (nk_menu_begin_label(ctx, "ADVANCED", NK_TEXT_LEFT, nk_vec2(200, 600)))
            {
                enum menu_state {MENU_NONE,MENU_FILE, MENU_EDIT,MENU_VIEW,MENU_CHART};
                static enum menu_state menu_state = MENU_NONE;
                enum nk_collapse_states state;

                state = (menu_state == MENU_FILE) ? NK_MAXIMIZED: NK_MINIMIZED;
                if (nk_tree_state_push(ctx, NK_TREE_TAB, "FILE", &state)) {
                    menu_state = MENU_FILE;
                    nk_menu_item_label(ctx, "New", NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, "Close", NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT);
                    nk_tree_pop(ctx);
                } else menu_state = (menu_state == MENU_FILE) ? MENU_NONE: menu_state;
                nk_menu_end(ctx);
            }
            nk_menubar_end(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "Volume", NK_MAXIMIZED))
        {

            nk_layout_row_dynamic(ctx, 20, 1);
            if (nk_button_label(ctx, "Open MHD..."))
            {
	            char const * filters[1] = { "*.mhd"  };
                char const * mhdfilepath = tinyfd_openFileDialog(
                    "Open MHD File...",
                    "",
                    1,
                    filters,
                    NULL,
                    0
                );

                if(mhdfilepath)
                {
                    renderer->loadvolume(mhdfilepath);
                    updateRenderer = true;
                }
            }

            if(renderer->m_volume)
            {
                size_t rowheight = 20;
                size_t namewidth = 100;
                size_t datawidth = 70;
                nk_layout_row_begin(ctx, NK_STATIC, rowheight, 4);
                nk_layout_row_push(ctx, namewidth);
                nk_label(ctx, "Dimensions: ", NK_TEXT_LEFT);
                nk_layout_row_push(ctx, datawidth);
                nk_label(ctx, std::to_string((int)renderer->m_volume->dataDimensions.x).c_str(), NK_TEXT_LEFT);
                nk_layout_row_push(ctx, datawidth);
                nk_label(ctx, std::to_string((int)renderer->m_volume->dataDimensions.y).c_str(), NK_TEXT_LEFT);
                nk_layout_row_push(ctx, datawidth);
                nk_label(ctx, std::to_string((int)renderer->m_volume->dataDimensions.z).c_str(), NK_TEXT_LEFT);
            }
            else
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "No volume loaded", NK_TEXT_CENTERED);
            }

            nk_tree_pop(ctx);
        }

        SubdivisionGui::gui(ctx, renderer, updateRenderer);
        BrickPoolGui::gui(ctx, renderer, updateRenderer);

        if (nk_tree_push(ctx, NK_TREE_TAB, "Render Options", NK_MINIMIZED))
        {
            nk_layout_row_static(ctx, 30, 80, 4);
            enum options {Render, ERT, DC, PT};
            static int option;
            int previousOption = option;
            option = nk_option_label(ctx, "Render", option == Render) ? Render : option;
            option = nk_option_label(ctx, "ERT", option == ERT) ? ERT : option;
            option = nk_option_label(ctx, "DC", option == DC) ? DC : option;
            option = nk_option_label(ctx, "PT", option == PT) ? PT : option;
            if(option != previousOption)
            {
                updateRenderer = true;
                renderer->m_highlightert = (option == ERT);
                renderer->m_showdepthcomplexity = (option == DC);
                renderer->m_showPageTableAccesses = (option == PT);
            }


            nk_layout_row_static(ctx, 30, 200, 1);
            int dontSample = renderer->m_dontsample;
            nk_checkbox_label(ctx, "Stub Sampling", &dontSample);
            if((dontSample == 1) != renderer->m_dontsample)
            {
                renderer->m_dontsample = (dontSample == 1);
                updateRenderer = true;
            }

            nk_tree_pop(ctx);
        }

        TransferFunctionGui::gui(ctx, renderer, updateRenderer);
        CameraGui::gui(ctx, renderer);

        if (nk_tree_push(ctx, NK_TREE_TAB, "Stats", NK_MAXIMIZED))
        {
            if(renderer->m_volume)
            {
                size_t rowheight = 20;
                size_t namewidth = 180;
                size_t datawidth = 70;
                {
                    nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
                    nk_layout_row_push(ctx, namewidth);
                    nk_label(ctx, "Last Render Time: ", NK_TEXT_LEFT);
                    nk_layout_row_push(ctx, datawidth);
                    nk_label(ctx, std::to_string(renderer->m_lastrenderduration).c_str(), NK_TEXT_LEFT);
                }

                {
                    nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
                    nk_layout_row_push(ctx, namewidth);
                    nk_label(ctx, "Last TF Test Time: ", NK_TEXT_LEFT);
                    nk_layout_row_push(ctx, datawidth);
                    nk_label(ctx, std::to_string(renderer->m_subdivision->mStats.get("tftesttime")).c_str(), NK_TEXT_LEFT);
                }

                if(renderer->m_subdivision->mCluster)
                {
                    nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
                    nk_layout_row_push(ctx, namewidth);
                    nk_label(ctx, "Last Cluster Time: ", NK_TEXT_LEFT);
                    nk_layout_row_push(ctx, datawidth);
                    nk_label(ctx, std::to_string(renderer->m_subdivision->mStats.get("clusteringtime")).c_str(), NK_TEXT_LEFT);
                }

                {
                    nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
                    nk_layout_row_push(ctx, namewidth);
                    nk_label(ctx, "Last Upload Time: ", NK_TEXT_LEFT);
                    nk_layout_row_push(ctx, datawidth);
                    nk_label(ctx, std::to_string(renderer->m_subdivision->mStats.get("lastuploadtime")).c_str(), NK_TEXT_LEFT);
                }

                {
                    nk_layout_row_begin(ctx, NK_STATIC, rowheight, 2);
                    nk_layout_row_push(ctx, namewidth);
                    nk_label(ctx, "Last BVH Build Time: ", NK_TEXT_LEFT);
                    nk_layout_row_push(ctx, datawidth);
                    nk_label(ctx, std::to_string(renderer->m_lastbvhbuildtime).c_str(), NK_TEXT_LEFT);
                }

            }

            nk_tree_pop(ctx);
        }
        if(updateRenderer)
        {
            renderer->render();
        }
    }
    nk_end(ctx);


    return !nk_window_is_closed(ctx, "Overview");
}
