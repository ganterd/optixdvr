/* nuklear - 1.32.0 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "../../optixdvr/utils/argparse.hpp"

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include "nuklear_glfw_gl4.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#include "overview.hpp"
#include "renderpanel.hpp"
#include "../../optixdvr/optixdvr_instance.hpp"

void SetUpAndParseArgs(int argc, char *argv[])
{
	/* Set up available program arguments */
	Arguments::AddFlagArgument("Lighting", "-l", "--enable_lighting");
	Arguments::AddFloatArgument("LightPosX", "-lx", "--lightx", 5);
	Arguments::AddFloatArgument("LightPosY", "-ly", "--lighty", 0);
	Arguments::AddFloatArgument("LightPosZ", "-lz", "--lightz", 0);

	// Transfer Function
	Arguments::AddStringArgument("TransferFunction", "-tf", "--transferFunction", "");
	Arguments::SetArgumentInfo("TransferFunction", "Set the path to .itf file. Usage: [-tf | --transferFunction] <path_to_itf>");

	// Volume info
	Arguments::AddStringArgument("VolumePath", "-v", "--volume", "");
	Arguments::SetArgumentInfo("VolumePath", "Path to volume MHD.");
	Arguments::AddIntegerArgument("SubDivisionsX", "-sdx", "--subDivisionsX", 8);
	Arguments::AddIntegerArgument("SubDivisionsY", "-sdy", "--subDivisionsY", 8);
	Arguments::AddIntegerArgument("SubDivisionsZ", "-sdz", "--subDivisionsZ", 8);

	// Render Info
	Arguments::AddIntegerArgument("RenderSizeX", "-rx", "", 1024);
	Arguments::AddIntegerArgument("RenderSizeY", "-ry", "", 1024);
    Arguments::AddFlagArgument("HighlightERT", "-ert", "--highlight-ert");
    Arguments::AddFlagArgument("ShowDepthComplexity", "-dc", "--depth-complexity");
	Arguments::AddStringArgument("OutFile", "-o", "--output", "out.ppm");

	/* Parse and validate the arguments */
	Arguments::Parse(argc, argv);
	if (!Arguments::Validate())
		throw std::runtime_error("Arguments didn't pass validation!");
}

int main(int argc, char **argv)
{
	SetUpAndParseArgs(argc, argv);
    /* Platform */
    static GLFWwindow *win;
    int width = 0, height = 0;
    struct nk_context *ctx;
    struct nk_colorf bg;

    /* GLFW */
    //glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Demo", NULL, NULL);
    glfwMakeContextCurrent(win);
    glfwGetWindowSize(win, &width, &height);

    /* OpenGL */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }

    ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&atlas);
        //struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", 14, 0);
        nk_glfw3_font_stash_end();
    }

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

    OptixDVR *rendererInstance = OptixInstance::get();

    // Volume Parameters
    rendererInstance->m_volumefilepath = Arguments::GetAsString("VolumePath");
    rendererInstance->m_transferfuncpath = Arguments::GetAsString("TransferFunction");
    rendererInstance->m_useshading = Arguments::IsSet("Lighting");
    rendererInstance->m_highlightert = Arguments::IsSet("HighlightERT");
    rendererInstance->m_showdepthcomplexity = Arguments::IsSet("ShowDepthComplexity");

    if(Arguments::IsSet("VolumePath"))
        rendererInstance->loadvolume(Arguments::GetAsString("VolumePath").c_str());
    
    if(Arguments::IsSet("TransferFunction"))
        rendererInstance->loadtransferfunction(Arguments::GetAsString("TransferFunction").c_str());

    RenderPanel renderPanel;
    while (!glfwWindowShouldClose(win))
    {
        /* Input */
        glfwPollEvents();
        nk_glfw3_new_frame();
        glfwGetWindowSize(win, &width, &height);

        /* Bindless Texture */
        renderPanel.gui(ctx, width, height);
        Overview::gui(ctx, width, height);

        /* Draw */
        glfwGetWindowSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(win);
    }
    nk_glfw3_shutdown();
    glfwTerminate();
    return 0;
}
