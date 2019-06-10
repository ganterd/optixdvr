#include <iostream>

#include "../../optixdvr/optixdvr.hpp"
#include "../../optixdvr/optixdvr_instance.hpp"
#include "../../optixdvr/utils/argparse.hpp"

void SetUpAndParseArgs(int argc, char *argv[])
{
	/* Set up available program arguments */
	Arguments::AddFlagArgument("Lighting", "-l", "--enable_lighting");
	Arguments::AddFloatArgument("LightPosX", "-lx", "--lightx", 5);
	Arguments::AddFloatArgument("LightPosY", "-ly", "--lighty", 0);
	Arguments::AddFloatArgument("LightPosZ", "-lz", "--lightz", 0);

	// Transfer Function
	Arguments::AddStringArgument("TransferFunction", "-tf", "--transferFunction", "");
	Arguments::SetArgumentRequired("TransferFunction", true);
	Arguments::SetArgumentInfo("TransferFunction", "Set the path to .itf file. Usage: [-tf | --transferFunction] <path_to_itf>");

	// Volume info
	Arguments::AddStringArgument("VolumePath", "-v", "--volume", "");
	Arguments::SetArgumentRequired("VolumePath", true);
	Arguments::SetArgumentInfo("VolumePath", "Path to volume MHD.");
	Arguments::AddIntegerArgument("BrickSizeX", "-bsx", "--brick-size-x", 16);
	Arguments::AddIntegerArgument("BrickSizeY", "-bsy", "--brick-size-y", 16);
	Arguments::AddIntegerArgument("BrickSizeZ", "-bsz", "--brick-size-z", 16);
    Arguments::AddFlagArgument("Cluster", "-cluster", "--cluster");

	// Render Info
	Arguments::AddIntegerArgument("RenderSizeX", "-rx", "", 1024);
	Arguments::AddIntegerArgument("RenderSizeY", "-ry", "", 1024);
    Arguments::AddFlagArgument("HighlightERT", "-ert", "--highlight-ert");
    Arguments::AddFlagArgument("ShowDepthComplexity", "-dc", "--depth-complexity");
    Arguments::AddFlagArgument("StubSampling", "-stub", "--stub-sampling");
	Arguments::AddStringArgument("OutFile", "-o", "--output", "out.ppm");

	// Camera Info
	Arguments::AddFloatArgument("CameraX", "-cx", "", 0);
	Arguments::AddFloatArgument("CameraY", "-cy", "", 0);
	Arguments::AddFloatArgument("CameraZ", "-cz", "", 3);
	Arguments::AddFloatArgument("LookX", "-lkx", "", 0);
	Arguments::AddFloatArgument("LookY", "-lky", "", 0);
	Arguments::AddFloatArgument("LookZ", "-lkz", "", 0);
    Arguments::AddFloatArgument("CameraAperture", "-ap", "", 0);
    Arguments::AddFloatArgument("CameraFovY", "-fov", "", 30);
    Arguments::AddIntegerArgument("Samples", "-s", "", 1);

    // Experimental Arguments
    Arguments::AddIntegerArgument("Runs", "-runs", "", 50);
    Arguments::AddIntegerArgument("Views", "-views", "", 180);

	/* Parse and validate the arguments */
	Arguments::Parse(argc, argv);
	if (!Arguments::Validate())
		throw std::runtime_error("Arguments didn't pass validation!");
}

int main(int argc, char *argv[])
{
	SetUpAndParseArgs(argc, argv);

    OptixDVR* optixdvr = OptixInstance::get();

    // Camera Parameters
    optixdvr->m_camera.origin(vec3f(
        Arguments::GetAsFloat("CameraX"),
        Arguments::GetAsFloat("CameraY"),
        Arguments::GetAsFloat("CameraZ")
    ));
    optixdvr->m_camera.lookat(vec3f(
        Arguments::GetAsFloat("LookX"),
        Arguments::GetAsFloat("LookY"),
        Arguments::GetAsFloat("LookZ")
    ));
    optixdvr->m_camera.mAperture = Arguments::GetAsFloat("CameraAperture");
    optixdvr->m_camera.mFOVY = Arguments::GetAsFloat("CameraFovY");

    // Volume Parameters
    optixdvr->m_volumefilepath = Arguments::GetAsString("VolumePath");
    optixdvr->m_transferfuncpath = Arguments::GetAsString("TransferFunction");
    // optixdvr->m_subdivisions.x = Arguments::GetAsInt("SubDivisionsX");
    // optixdvr->m_subdivisions.y = Arguments::GetAsInt("SubDivisionsY");
    // optixdvr->m_subdivisions.z = Arguments::GetAsInt("SubDivisionsZ");
    optixdvr->m_useshading = Arguments::IsSet("Lighting");
    optixdvr->m_highlightert = Arguments::IsSet("HighlightERT");
    optixdvr->m_showdepthcomplexity = Arguments::IsSet("ShowDepthComplexity");
    optixdvr->m_dontsample = Arguments::IsSet("StubSampling");

    // Output Parameters
    vec3f bricksize;
    bricksize.x = Arguments::GetAsInt("BrickSizeX");
    bricksize.y = Arguments::GetAsInt("BrickSizeY");
    bricksize.z = Arguments::GetAsInt("BrickSizeZ");
    //optixdvr->m_subdivision->set_brick_size(bricksize);

    optixdvr->m_outppmpath = Arguments::GetAsString("OutFile");
    optixdvr->m_renderwidth = Arguments::GetAsInt("RenderSizeX");
    optixdvr->m_renderheight = Arguments::GetAsInt("RenderSizeY");
    optixdvr->m_samples = Arguments::GetAsInt("Samples");

    optixdvr->m_subdivision->mCluster = Arguments::IsSet("Cluster");

    optixdvr->resizeFrameBuffer(optixdvr->m_renderwidth, optixdvr->m_renderheight);
    optixdvr->m_subdivision->set_brick_size(bricksize);
    optixdvr->loadvolume(Arguments::GetAsString("VolumePath").c_str());
    optixdvr->loadtransferfunction(Arguments::GetAsString("TransferFunction").c_str());
    optixdvr->updateScene();

    int views = Arguments::GetAsInt("Views");
    float dt = 2.0f * M_PI / (float)views;
    float distance = 2.0f;
    vec3f cameraLookAt(0.0f);
    // Warming loop
    std::cout << "Warming frames..." << std::endl;
    for(int i = 0; i < views; i++)
    {
        float x = sinf(dt * (float)i);
        float y = cosf(dt * (float)i);
        vec3f p = vec3f(x * distance, 0.01f, y * distance);
        vec3f d = normalize(vec3f(-x, -0.0001f, -y));

        optixdvr->m_camera.origin(p);
        optixdvr->m_camera.lookdir(d);
        optixdvr->render();
    }

    std::vector<double> totalviewtimes(views, 0.0);
    std::vector<std::vector<float>> times;
    int runs = Arguments::GetAsInt("Runs");
    times.resize(runs);
    for(int r = 0; r < runs; ++r)
    {
        times[r].resize(views);
        for(int i = 0; i < views; i++)
        {
            std::cout  << "Run [" << i + views * r << "/" << views * runs << "]        \r";
            float x = sinf(dt * (float)i);
            float y = cosf(dt * (float)i);
            vec3f p = vec3f(x * distance, 0.01f, y * distance);
            vec3f d = normalize(vec3f(-x, -0.0001f, -y));

            optixdvr->m_camera.origin(p);
            optixdvr->m_camera.lookdir(d);
            optixdvr->render();
            times[r][i] = optixdvr->m_lastrenderduration;
            totalviewtimes[i] += (double)optixdvr->m_lastrenderduration;
        }
    }

    // Capture loop
    for(int i = 0; i < views; i++)
    {
        float x = sinf(dt * (float)i);
        float y = cosf(dt * (float)i);
        vec3f p = vec3f(x * distance, 0.01f, y * distance);
        vec3f d = normalize(vec3f(-x, -0.0001f, -y));

        optixdvr->m_camera.origin(p);
        optixdvr->m_camera.lookdir(d);
        optixdvr->render();

        std::stringstream ss;
        ss << "output/" << i << ".png";
        std::cout << "\033[KSaving image to " << ss.str() << "              \r";
        optixdvr->saveToPNG(ss.str().c_str());
    }
    std::cout << "\033[KCaptured frames... " << std::endl;

    std::ofstream csvfile("times.csv");
    csvfile << "avg_frame_time";
    for(int i = 0; i < runs; ++i)
        csvfile << ",frame_" << i << "_time";

    for(int v = 0; v < views; ++v)
    {
        csvfile << "\n";
        csvfile << (totalviewtimes[v] / (double)runs);
        for(int r = 0; r < runs; ++r)
        {
            csvfile << "," << times[r][v];
        }
    }
}