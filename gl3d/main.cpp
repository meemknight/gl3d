#include <Windows.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/gtx/transform.hpp>

#include "src/gl3d.h"
#include "profiler.h"

#include <ctime>
#include <functional>
#include <algorithm>

int w = 840;
int h = 640;

#define USE_GPU_ENGINE 1
#define DEBUG_OUTPUT 1

#pragma region gpu
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = USE_GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = USE_GPU_ENGINE;
}
#pragma endregion

int main()
{

#pragma region init

	if (!glfwInit())
	{
		std::cout << "err initializing glfw";
	}

	glfwWindowHint(GLFW_SAMPLES, 1);

#if DEBUG_OUTPUT
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow *wind = glfwCreateWindow(w, h, "geam", nullptr, nullptr);
	glfwMakeContextCurrent(wind);
	glfwSwapInterval(0);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "err initializing glew";
	}

#pragma region enable debug output
#if DEBUG_OUTPUT
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gl3d::glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
#pragma endregion


	ImGui::CreateContext();
	ImGui::StyleColorsDark(); 

	ImGuiIO &io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	ImGuiStyle &style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 0.f;
		style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
	}

	ImGui_ImplGlfw_InitForOpenGL(wind, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


#pragma endregion

	gl3d::Renderer3D renderer;
	renderer.init(w, h);


#pragma region shader
	gl3d::Shader shader;
	shader.loadShaderProgramFromFile("shaders/color.vert", "shaders/color.frag");
	shader.bind();
	GLint location = glGetUniformLocation(shader.id, "u_transform");

	if (location == -1)
	{
		std::cout << "uniform error u_transform\n";
	}


	gl3d::Shader showNormalsShader;
	showNormalsShader.loadShaderProgramFromFile("shaders/showNormals.vert",
		"shaders/showNormals.geom", "shaders/showNormals.frag");

#pragma endregion


#pragma region texture

	//gl3d::Texture crateTexture("resources/other/crate.png");
	//gl3d::Texture crateNormalTexture("resources/other/crateNormal.png");
	//gl3d::Texture rockTexture("resources/other/boulder.png");
	//gl3d::Texture rockNormalTexture("resources/other/boulderNormal.png");

	//gl3d::Texture levelTexture("resources/obj/level.png");

#pragma endregion

	
	const char *names[6] = 
	{	"resources/skyBoxes/ocean/right.jpg",
		"resources/skyBoxes/ocean/left.jpg",
		"resources/skyBoxes/ocean/top.jpg",
		"resources/skyBoxes/ocean/bottom.jpg",
		"resources/skyBoxes/ocean/front.jpg",
		"resources/skyBoxes/ocean/back.jpg" };

	//renderer.skyBox = renderer.loadSkyBox(names);

	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBoxes/WinterForest_Ref.hdr");
	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBoxes/Newport_Loft_Ref.hdr");
	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBoxes/bell_park_dawn_1k.hdr");
	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBoxes/Milkyway_small.hdr");
	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBox.hdr");
	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBoxes/canary_wharf_2k.hdr");
	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBoxes/chinese_garden_2k.hdr");
	
	renderer.skyBox.clearTextures();
	renderer.skyBox = renderer.atmosfericScattering(glm::normalize(glm::vec3{-1, 1, -1}),
		glm::vec3(141 / 255.f, 217 / 255.f, 224 / 255.f),
		glm::vec3(207 / 255.f, 196 / 255.f, 157 / 255.f),
		0.76);

	//skyBox.loadTexture("resources/skyBoxes/ocean_1.png");
	//renderer.skyBox = renderer.loadHDRSkyBox("resources/skyBoxes/chinese_garden_2k.hdr");
	//renderer.skyBox = renderer.loadSkyBox("resources/skyBoxes/sky.png", 0);
	//renderer.skyBox = renderer.loadSkyBox("resources/skyBoxes/forest.png", 0);
	
	
	//VertexArrayContext va;
	//va.create();


	float cubePositions[] = {
		-1.0f, +1.0f, +1.0f, // 0
		+1.0f, +0.0f, +0.0f, // Color
		+1.0f, +1.0f, +1.0f, // 1
		+1.0f, +0.0f, +0.0f, // Color
		+1.0f, +1.0f, -1.0f, // 2
		+1.0f, +0.0f, +0.0f, // Color
		-1.0f, +1.0f, -1.0f, // 3
		+1.0f, +0.0f, +0.0f, // Color
		-1.0f, +1.0f, -1.0f, // 4
		+0.0f, +1.0f, +0.0f, // Color
		+1.0f, +1.0f, -1.0f, // 5
		+0.0f, +1.0f, +0.0f, // Color
		+1.0f, -1.0f, -1.0f, // 6
		+0.0f, +1.0f, +0.0f, // Color
		-1.0f, -1.0f, -1.0f, // 7
		+0.0f, +1.0f, +0.0f, // Color
		+1.0f, +1.0f, -1.0f, // 8
		+0.0f, +0.0f, +1.0f, // Colors
		+1.0f, +1.0f, +1.0f, // 9
		+0.0f, +0.0f, +1.0f, // Color
		+1.0f, -1.0f, +1.0f, // 10
		+0.0f, +0.0f, +1.0f, // Color
		+1.0f, -1.0f, -1.0f, // 11
		+0.0f, +0.0f, +1.0f, // Color
		-1.0f, +1.0f, +1.0f, // 12
		+1.0f, +1.0f, +0.0f, // Color
		-1.0f, +1.0f, -1.0f, // 13
		+1.0f, +1.0f, +0.0f, // Color
		-1.0f, -1.0f, -1.0f, // 14
		+1.0f, +1.0f, +0.0f, // Color
		-1.0f, -1.0f, +1.0f, // 15
		+1.0f, +1.0f, +0.0f, // Color
		+1.0f, +1.0f, +1.0f, // 16
		+0.0f, +1.0f, +1.0f, // Color
		-1.0f, +1.0f, +1.0f, // 17
		+0.0f, +1.0f, +1.0f, // Color
		-1.0f, -1.0f, +1.0f, // 18
		+0.0f, +1.0f, +1.0f, // Color
		+1.0f, -1.0f, +1.0f, // 19
		+0.0f, +1.0f, +1.0f, // Color
		+1.0f, -1.0f, -1.0f, // 20
		+1.0f, +0.0f, +1.0f, // Color
		-1.0f, -1.0f, -1.0f, // 21
		+1.0f, +0.0f, +1.0f, // Color
		-1.0f, -1.0f, +1.0f, // 22
		+1.0f, +0.0f, +1.0f, // Color
		+1.0f, -1.0f, +1.0f, // 23
		+1.0f, +0.0f, +1.0f, // Color
	};

	float uv = 1;
	float cubePositionsNormals[] = {
		-1.0f, +1.0f, +1.0f, // 0
		+0.0f, +1.0f, +0.0f, // Normal
		0, 0,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		+1.0f, +1.0f, +1.0f, // 1
		+0.0f, +1.0f, +0.0f, // Normal
		1* uv, 0,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent
		
		+1.0f, +1.0f, -1.0f, // 2
		+0.0f, +1.0f, +0.0f, // Normal
		1* uv, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent
		
		-1.0f, +1.0f, -1.0f, // 3
		+0.0f, +1.0f, +0.0f, // Normal
		0, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent



		-1.0f, +1.0f, -1.0f, // 4
		 0.0f, +0.0f, -1.0f, // Normal
		 0, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		+1.0f, +1.0f, -1.0f, // 5
		 0.0f, +0.0f, -1.0f, // Normal
		 1* uv, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		 +1.0f, -1.0f, -1.0f, // 6
		 0.0f, +0.0f, -1.0f, // Normal
		 1* uv, 0,				 //uv
		 //0,0,0,				 //tangent
		 //0,0,0,				 //btangent

		-1.0f, -1.0f, -1.0f, // 7
		 0.0f, +0.0f, -1.0f, // Normal
		 0, 0,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		+1.0f, +1.0f, -1.0f, // 8
		+1.0f, +0.0f, +0.0f, // Normal
		1* uv, 0,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		+1.0f, +1.0f, +1.0f, // 9
		+1.0f, +0.0f, +0.0f, // Normal
		1* uv, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		+1.0f, -1.0f, +1.0f, // 10
		+1.0f, +0.0f, +0.0f, // Normal
		0, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		+1.0f, -1.0f, -1.0f, // 11
		+1.0f, +0.0f, +0.0f, // Normal
		0, 0,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		-1.0f, +1.0f, +1.0f, // 12
		-1.0f, +0.0f, +0.0f, // Normal
		1* uv, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		-1.0f, +1.0f, -1.0f, // 13
		-1.0f, +0.0f, +0.0f, // Normal
		1* uv, 0,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		-1.0f, -1.0f, -1.0f, // 14
		-1.0f, +0.0f, +0.0f, // Normal
		0, 0,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		-1.0f, -1.0f, +1.0f, // 15
		-1.0f, +0.0f, +0.0f, // Normal
		0, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent


		+1.0f, +1.0f, +1.0f, // 16
		+0.0f, +0.0f, +1.0f, // Normal
		1* uv, 1* uv,				 //uv
		//0,0,0,				 //tangent
		//0,0,0,				 //btangent

		-1.0f, +1.0f, +1.0f, // 17
		+0.0f, +0.0f, +1.0f, // Normal
		0, 1* uv,				 //uv
		//0, 0, 0,				 //tangent
		//0, 0, 0,				 //btangent

		-1.0f, -1.0f, +1.0f, // 18
		+0.0f, +0.0f, +1.0f, // Normal
		0, 0,				 //uv
		//0, 0, 0,				 //tangent
		//0, 0, 0,				 //btangent

		+1.0f, -1.0f, +1.0f, // 19
		+0.0f, +0.0f, +1.0f, // Normal
		1* uv, 0,				 //uv
		//0, 0, 0,				 //tangent
		//0, 0, 0,				 //btangent


		+1.0f, -1.0f, -1.0f, // 20
		+0.0f, -1.0f, +0.0f, // Normal
		1* uv, 0,				 //uv
		//0, 0, 0,				 //tangent
		//0, 0, 0,				 //btangent

		-1.0f, -1.0f, -1.0f, // 21
		+0.0f, -1.0f, +0.0f, // Normal
		0, 0,				 //uv
		//0, 0, 0,				 //tangent
		//0, 0, 0,				 //btangent

		-1.0f, -1.0f, +1.0f, // 22
		+0.0f, -1.0f, +0.0f, // Normal
		0, 1* uv,				 //uv
		//0, 0, 0,				 //tangent
		//0, 0, 0,				 //btangent

		+1.0f, -1.0f, +1.0f, // 23
		+0.0f, -1.0f, +0.0f, // Normal
		1* uv, 1* uv,				 //uv
		//0, 0, 0,				 //tangent
		//0, 0, 0,				 //btangent

	};
	
	unsigned int cubeIndices[] = {
	0,   1,  2,  0,  2,  3, // Top
	4,   5,  6,  4,  6,  7, // Back
	8,   9, 10,  8, 10, 11, // Right
	12, 13, 14, 12, 14, 15, // Left
	16, 17, 18, 16, 18, 19, // Front
	20, 22, 21, 20, 23, 22, // Bottom
	};

	//renderer.pointLights.push_back(gl3d::internal::GpuPointLight());
	//renderer.pointLights[0].position = glm::vec4(0, 0.42, 2.44, 0);

	//renderer.directionalLights.push_back(gl3d::internal::GpuDirectionalLight());
	//renderer.directionalLights[0].direction = glm::vec4(glm::normalize(glm::vec3(0, -1, 0.2)),0);

	gl3d::DebugGraphicModel lightCubeModel;
	lightCubeModel.loadFromComputedData(sizeof(cubePositions),
	cubePositions,
		sizeof(cubeIndices), cubeIndices, true);
	lightCubeModel.scale = glm::vec3(0.05);


	PL::AverageProfiler loadProfiler;
	loadProfiler.start();

	//auto materials = renderer.loadMaterial("resources/materials/adventurer/adventurer.mtl");
	//auto materials = renderer.loadMaterial("resources/materials/ironMan/ironMan.mtl");


	//auto levelModel = renderer.loadModel("resources/donut/Donut.glb", 1.f);
	//auto levelModel = renderer.loadModel("resources/mutant/Biomech_Mutant_Skin_1.gltf", 1.f);
	//auto barelModel = renderer.loadModel("resources/wine/wine_barrel_01_2k.gltf");
	//auto rockModel = renderer.loadModel("resources/mutant/2/Biomech_Mutant_Skin_2.gltf", 1.f);
	//auto rockModel = renderer.loadModel("resources/amogus.glb", 1.f);
	//auto rockModel = renderer.loadModel("resources/animatedModels/arrow.glb", 1.f);
	auto rockModel = renderer.loadModel("resources/knight/uploads_files_1950170_Solus_the_knight.gltf", 1.f);
	//auto sphereModel = renderer.loadModel("resources/obj/sphere.obj");
	//auto levelModel = renderer.loadModel("resources/gltf/steve.glb");
	//auto levelModel = renderer.loadModel("resources/gltf/boomBox/BoomBox.gltf");
	//auto sphereModel = renderer.loadModel("resources/sponza2/sponza.obj", 0.008);
	//auto sphereModel = renderer.loadModel("resources/katana/antique_katana_01_1k.gltf");
	//auto rockModel = renderer.loadModel("resources/mutant/Biomech_Mutant_Skin_1.glb", 1.f);

	//auto barelModel = renderer.loadModel("resources/barrel/Barrel_01.obj");
	//auto rockModel = renderer.loadModel("resources/helmet/helmet.obj");
	//auto rockModel = renderer.loadObject("resources/other/boulder.obj", 0.1);
	//auto levelModel = renderer.loadModel("resources/city/city.obj", 0.01);
	//auto levelModel = renderer.loadModel("resources/sponza/sponza.obj");
	//auto rockModel = renderer.loadModel("resources/other/crate.obj", 0.01);
	//auto rockModel = renderer.loadModel("resources/obj/sphere3.obj");
	//auto levelModel = renderer.loadModel("resources/planeta.glb");
	//auto sphereModel = renderer.loadModel("resources/obj/sphere2.obj");
	auto sphereModel = renderer.loadModel("resources/birb.glb");
	auto levelModel = renderer.loadModel("resources/obj/sphere.obj");
	//auto rockModel = renderer.loadModel("resources/obj/sphere.obj");
	auto barelModel = renderer.loadModel("resources/obj/sphere.obj");
	
	auto rez = loadProfiler.end();



	std::cout << "\n\nLoad profiler: time(s):" << rez.timeSeconds << "    cpu clocks:" << rez.cpuClocks << "\n";

	
	//cube.loadFromModelMeshIndex(barelModel, 0);
	//cube.scale = glm::vec3(0.1);

	//auto objectTest = renderer.loadObject("resources/other/crate.obj", 0.01);
	//auto objectTest = renderer.loadObject("resources/sponza2/sponza.obj", 0.008);


	//auto objectTest = renderer.loadObject("resources/barrel/Barrel_01.obj");
	//auto objectTest2 = renderer.loadObject("resources/other/crate.obj", 0.01);

	std::vector< gl3d::Entity > models;
	static std::vector < const char* > items = {}; //just models names

	std::vector< gl3d::SpotLight > spotLights;
	std::vector< gl3d::PointLight > pointLights;
	std::vector< gl3d::DirectionalLight > directionalLights;


	renderer.camera.aspectRatio = (float)w / h;
	renderer.camera.fovRadians = glm::radians(100.f);
	renderer.camera.position = { 0.f,0.f,2.f };

	static int itemCurrent = 0;
	static int subItemCurent = 0;
	static bool borderItem = 0;
	static bool showNormals = 0;
	static bool normalMap = 1;

	static int currnetFps = 0;
	static float currnetMill = 0;
	const int FPS_RECORD_ARR_SIZE = 20;
	int recordPos = 0;
	static float fpsArr[FPS_RECORD_ARR_SIZE] = { };
	static float millArr[FPS_RECORD_ARR_SIZE] = { };

	const int DELTA_TIME_ARR_SIZE = 60;
	int recordPosDeltaTime = 0;
	static float deltaTimeArr[DELTA_TIME_ARR_SIZE] = { };

	int timeBeg = clock();

	//static PL::AverageProfiler renderProfiler;
	//static PL::ProfileRezults lastProfilerRezult = {};

	//const int Profiler_ARR_SIZE = 60;
	//int profilePos = 0;
	//static float profileeArr[Profiler_ARR_SIZE] = { };

	//const int ProfilerAverage_ARR_SIZE = 20;
	//int profileAveragePos = 0;
	//static float profileAverageArr[ProfilerAverage_ARR_SIZE] = { };

	PL::ImguiProfiler renderDurationProfiler("Render duration (ms) (avg over one sec)", 0, 30);
	PL::ImguiProfiler renderDurationProfilerFine("Render duration (ms) ", 0, 30);
	PL::ImguiProfiler imguiRenderDuration("Imgui render duration (ms) ", 0, 30);
	PL::ImguiProfiler swapBuffersDuration("Swap buffers duration (ms) ", 0, 30);

	while (!glfwWindowShouldClose(wind))
	{
		glfwGetWindowSize(wind, &w, &h);
		w = std::max(w, 1);
		h = std::max(h, 1);
		
		int timeEnd = clock();
		float deltaTime = (timeEnd - timeBeg) / 1000.f;
		timeBeg = clock();



		

	#pragma region profiler ui code
		{
			static float fpsCounterFloat;
			static float currentFpsCounter;

			fpsCounterFloat += deltaTime;
			currentFpsCounter += 1;
			if (fpsCounterFloat >= 1)
			{
				fpsCounterFloat -= 1;
				currnetFps = currentFpsCounter;
				currnetMill = (1.f / currnetFps) * 1000.f;
				currentFpsCounter = 0;

				if (recordPos < FPS_RECORD_ARR_SIZE)
				{
					fpsArr[recordPos] = currnetFps;
					millArr[recordPos] = currnetMill;
					recordPos++;
				}
				else
				{
					for (int i = 0; i < FPS_RECORD_ARR_SIZE - 1; i++)
					{
						fpsArr[i] = fpsArr[i + 1];
						millArr[i] = millArr[i + 1];

					}
					fpsArr[FPS_RECORD_ARR_SIZE - 1] = currnetFps;
					millArr[FPS_RECORD_ARR_SIZE - 1] = currnetMill;
				}

				//if (profileAveragePos < ProfilerAverage_ARR_SIZE)
				//{
				//
				//	profileAverageArr[profileAveragePos] = renderProfiler.getAverageAndResetData().timeSeconds * 1000;
				//	profileAveragePos++;
				//}
				//else
				//{
				//	for (int i = 0; i < ProfilerAverage_ARR_SIZE - 1; i++)
				//	{
				//		profileAverageArr[i] = profileAverageArr[i + 1];
				//	}
				//	profileAverageArr[ProfilerAverage_ARR_SIZE - 1] = renderProfiler.getAverageAndResetData().timeSeconds * 1000;
				//}

				renderDurationProfiler.updateValue(1000);
				imguiRenderDuration.updateValue(1000);
				swapBuffersDuration.updateValue(1000);
			}

			if (recordPosDeltaTime < DELTA_TIME_ARR_SIZE)
			{
				deltaTimeArr[recordPosDeltaTime] = deltaTime * 1000;
				recordPosDeltaTime++;
			}
			else
			{
				for (int i = 0; i < DELTA_TIME_ARR_SIZE - 1; i++)
				{
					deltaTimeArr[i] = deltaTimeArr[i + 1];
				}
				deltaTimeArr[DELTA_TIME_ARR_SIZE - 1] = deltaTime*1000;
			}

			//if (profilePos < Profiler_ARR_SIZE)
			//{
			//	profileeArr[profilePos] = lastProfilerRezult.timeSeconds;
			//	profilePos++;
			//}
			//else
			//{
			//	for (int i = 0; i < Profiler_ARR_SIZE - 1; i++)
			//	{
			//		profileeArr[i] = profileeArr[i + 1];
			//	}
			//	profileeArr[Profiler_ARR_SIZE - 1] = lastProfilerRezult.timeSeconds;
			//}

			renderDurationProfilerFine.updateValue(1000);

		}
	#pragma endregion

	#pragma region imgui

		static float gamaCorection = 1;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		//window_flags |= ImGuiWindowFlags_NoBackground;
		//static bool open = true;
		//ImGui::Begin("DockSpace Demo", &open, window_flags);
		//static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		//dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		//ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		//ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		//
		//ImGui::End();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		static bool lightEditor = 1;
		static bool cubeEditor = 1;
		static bool showStats = 0;
		

		auto drawImageNoQuality = [io](const char* c, GLuint id, int w, int h, int newW, int newH, int imguiId)
		{
			ImGui::PushID(imguiId);
			ImGui::Text(c, id);
			ImGui::SameLine();

			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImVec2 uv_min = ImVec2(1.0f, 0.0f);                 // Top-left
			ImVec2 uv_max = ImVec2(0.0f, 1.0f);                 // Lower-right
			ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
			ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
			ImGui::Image((void*)(id),
				ImVec2(w, h), uv_min, uv_max, tint_col, border_col);

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Image((void*)(id)
					, ImVec2(newW, newH), uv_min, uv_max, tint_col, border_col);
				ImGui::EndTooltip();
			}
			ImGui::PopID();
		};
		
		{
			ImGui::Begin("skyBox");

			static glm::vec3 color1 = glm::vec3(141 / 255.f, 217 / 255.f, 224 / 255.f);
			static glm::vec3 color2 = glm::vec3(207 / 255.f, 196 / 255.f, 157 / 255.f);
			static float g1 = 0.76;
			static glm::vec3 direction = glm::normalize(glm::vec3{-1, 1, -1});

			ImGui::DragFloat("g", &g1, 0.01, 0, 100);

			ImGui::ColorEdit3("Color1 ##c1", &color1[0]);
			ImGui::ColorEdit3("Color2 ##c2", &color2[0]);

			ImGui::DragFloat3("Pos sun", &direction[0], 0.01, -1.f, 1.f);

			if (glm::length(direction) == 0)
			{
				direction = glm::vec3{0, -1, 0};
			}
			else
			{
				direction = glm::normalize(direction);
			}

			if (ImGui::Button("generate"))
			{
			
				renderer.skyBox.clearTextures();
				renderer.skyBox = renderer.atmosfericScattering(direction,
					color1,
					color2,
					g1);
			}

			ImGui::End();
		}



		//imgui main menu
		{
			ImGui::Begin("Menu");
			ImGui::SetWindowFontScale(1.2f);

			ImGui::Checkbox("Light Editor##check", &lightEditor);
			ImGui::Checkbox("Object Editor##check", &cubeEditor);
			ImGui::Checkbox("Stats##check", &showStats);
			ImGui::NewLine();
			ImGui::Text("Settings");
			ImGui::SliderFloat("Gama Corections", &gamaCorection, 1, 3);
			ImGui::SliderFloat("Exposure", &renderer.internal.lightShader.lightPassUniformBlockCpuData.exposure , 0.1, 10);


			static bool antiAliasing = 0;
			ImGui::Checkbox("Anti aliasing", &antiAliasing);
			if (antiAliasing)
			{
				glEnable(GL_MULTISAMPLE);
			}
			else
			{
				glDisable(GL_MULTISAMPLE);
			}

			static bool sampleShading = 0;
			ImGui::Checkbox("Sample Shading", &sampleShading);

			if (sampleShading)
			{
				glEnable(GL_SAMPLE_SHADING);
			}
			else
			{
				glDisable(GL_SAMPLE_SHADING);
			}

			static bool cullFace = 1;
			ImGui::Checkbox("CullFace", &cullFace);
			
			if (cullFace)
			{
				glEnable(GL_CULL_FACE);
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}

			ImGui::Checkbox("Normal map", &normalMap);
			renderer.enableNormalMapping(normalMap);
			
			static bool lightSubScater = 1;
			ImGui::Checkbox("Light sub scater", &lightSubScater);
			renderer.enableLightSubScattering(lightSubScater);

			ImGui::Checkbox("FXAA", &renderer.antiAlias.usingFXAA);
			ImGui::Checkbox("Adaptive resolution", &renderer.adaptiveResolution.useAdaptiveResolution);
			ImGui::Text("Adaptive rez ratio: %.1f", renderer.adaptiveResolution.rezRatio);
			ImGui::Checkbox("Z pre pass", &renderer.zPrePass);
			ImGui::Checkbox("Frustum culling", &renderer.frustumCulling);


			if (ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_Framed || ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::PushID(__COUNTER__);

				ImGui::Checkbox("SSAO", &renderer.internal.lightShader.useSSAO);
				ImGui::SliderFloat("SSAO bias", &renderer.ssao.ssaoShaderUniformBlockData.bias, 0, 0.5);
				ImGui::SliderFloat("SSAO radius", &renderer.ssao.ssaoShaderUniformBlockData.radius, 0, 2);
				ImGui::SliderInt("SSAO sample count", &renderer.ssao.ssaoShaderUniformBlockData.samplesTestSize, 0, 64);
				ImGui::SliderFloat("SSAO exponent", &renderer.ssao.ssao_finalColor_exponent, 0, 16);

				ImGui::PopID();
			}
			ImGui::NewLine();

	
			if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_Framed || ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::PushID(__COUNTER__);

				ImGui::Checkbox("Bloom", &renderer.internal.lightShader.bloom);
				ImGui::DragFloat("Bloom tresshold", &renderer.internal.lightShader.lightPassUniformBlockCpuData.bloomTresshold,
					0.01, 0, 1);
				ImGui::DragFloat("Bloom intensity", &renderer.postProcess.bloomIntensty, 0.01, 0, 10);
				ImGui::Checkbox("High quality down sample", &renderer.postProcess.highQualityDownSample);
				ImGui::Checkbox("High quality up sample", &renderer.postProcess.highQualityUpSample);

				ImGui::PopID();
			}
			ImGui::NewLine();


			ImGui::Checkbox("Display item border", &borderItem);
			ImGui::Checkbox("Display normals", &showNormals);

			ImGui::PushID(234);

			//drawImageNoQuality("cascade 0: %d", renderer.directionalShadows.depthMapTexture[0],
			//	40, 40, 400, 400, __COUNTER__);
			//drawImageNoQuality("cascade 1: %d", renderer.directionalShadows.depthMapTexture[1],
			//	40, 40, 400, 400, __COUNTER__);
			//drawImageNoQuality("cascade 1: %d", renderer.directionalShadows.depthMapTexture[2],
			//	40, 40, 400, 400, __COUNTER__);
			//drawImageNoQuality("cascade 1: %d", renderer.directionalShadows.cascadesTexture,
			//	40, 120, 160, 480, __COUNTER__);
			
			//drawImageNoQuality("shadow map texture: %d", renderer.directionalShadows.varianceShadowTexture,
			//	40, 40, 400, 400, __COUNTER__);

			//drawImageNoQuality("fxaa texture: %d", renderer.fxaa.texture,
			//		40, 40, 400, 400, __COUNTER__);


			ImGui::PopID();


			ImGui::End();
		}

		ImGuiWindowFlags flags = {};
			
		//imgui light editor
		if(lightEditor)
		{
			ImGui::PushID(6996);
			ImGui::Begin("Light Editor", &lightEditor, flags);
			ImGui::SetWindowFontScale(1.2f);
		
			static int pointLightSelector = -1;
			ImGui::Text("Point lightd Count %d", pointLights.size());
			ImGui::InputInt("Current Point light:", &pointLightSelector);
			int n = ImGui::Button("New Light"); ImGui::SameLine();
			int remove = ImGui::Button("Remove Light");

			int lightSize = renderer.getPointLightShadowSize();
			ImGui::DragInt("Point light shadow texture size", &lightSize);
			renderer.setPointLightShadowSize(lightSize);

			if(pointLightSelector < -1)
			{
				pointLightSelector = -1;
			}

			if(n || (pointLightSelector >= (int)pointLights.size()) )
			{
				pointLights.push_back(renderer.createPointLight({ 0,0,0 }));
			}

			pointLightSelector = std::min(pointLightSelector, (int)pointLights.size() - 1);

			if(remove)
			{
				if(pointLightSelector>=0)
				{
					renderer.detletePointLight(pointLights[pointLightSelector]);
					pointLights.erase(pointLights.begin() + pointLightSelector);
					pointLightSelector = std::min(pointLightSelector, (int)pointLights.size() - 1);
				}
			
			}

			ImGui::NewLine();

			if(pointLightSelector >= 0)
			{
				ImGui::PushID(12);
				
				glm::vec3 color = renderer.getPointLightColor(pointLights[pointLightSelector]);
				ImGui::ColorEdit3("Color", &color[0]);
				renderer.setPointLightColor(pointLights[pointLightSelector], color);

				glm::vec3 position = renderer.getPointLightPosition(pointLights[pointLightSelector]);
				ImGui::DragFloat3("Position", &position[0], 0.1);
				renderer.setPointLightPosition(pointLights[pointLightSelector], position);

				float distance = renderer.getPointLightDistance(pointLights[pointLightSelector]);
				ImGui::DragFloat("Distance##point", &distance, 0.05, 0);
				renderer.setPointLightDistance(pointLights[pointLightSelector], distance);

				float attenuation = renderer.getPointLightAttenuation(pointLights[pointLightSelector]);
				ImGui::DragFloat("Attenuation##point", &attenuation, 0.05, 0);
				renderer.setPointLightAttenuation(pointLights[pointLightSelector], attenuation);
				
				float hardness = renderer.getPointLightHardness(pointLights[pointLightSelector]);
				ImGui::DragFloat("Hardness##point", &hardness, 0.05, 0.001);
				renderer.setPointLightHardness(pointLights[pointLightSelector], hardness);

				bool shadows = renderer.getPointLightShadows(pointLights[pointLightSelector]);
				ImGui::Checkbox("Cast shadows##point", &shadows);
				renderer.setPointLightShadows(pointLights[pointLightSelector], shadows);

				ImGui::PopID();
			}

			{
				ImGui::NewLine();

				static int directionalLightSelector = -1;
				ImGui::Text("Directional lightd Count %d", directionalLights.size());
				ImGui::InputInt("Current directional light:", &directionalLightSelector);
				int n = ImGui::Button("New Directional Light"); ImGui::SameLine();
				int remove = ImGui::Button("Remove Directional Light");

				int lightSize = renderer.getDirectionalLightShadowSize();
				ImGui::DragInt("Directional light shadow texture size", &lightSize);
				renderer.setDirectionalLightShadowSize(lightSize);

				if (directionalLightSelector < -1)
				{
					directionalLightSelector = -1;
				}

				if (n || directionalLightSelector >= (int)directionalLights.size())
				{
					directionalLights.push_back(renderer.createDirectionalLight(glm::vec3(0.f)));
				}

				directionalLightSelector 
					= std::min(directionalLightSelector, (int)directionalLights.size() - 1);

				if (remove)
				{
					if (directionalLightSelector >= 0)
					{
						renderer.deleteDirectionalLight(directionalLights[directionalLightSelector]);
						directionalLights.erase(directionalLights.begin() + directionalLightSelector);
						directionalLightSelector = std::min(directionalLightSelector, 
							(int)directionalLights.size() - 1);
					}

				}

				ImGui::NewLine();

				if (directionalLightSelector >= 0)
				{
					ImGui::PushID(13);

					glm::vec3 color = renderer.getDirectionalLightColor(directionalLights[directionalLightSelector]);
					ImGui::ColorEdit3("Color##dir", &color[0]);
					renderer.setDirectionalLightColor(directionalLights[directionalLightSelector], color);

					glm::vec3 direction = renderer.getDirectionalLightDirection(directionalLights[directionalLightSelector]);
					ImGui::DragFloat3("Direction##dir", &direction[0], 0.01);
					renderer.setDirectionalLightDirection(directionalLights[directionalLightSelector], direction);

					float hardness = renderer.getDirectionalLightHardness(directionalLights[directionalLightSelector]);
					ImGui::SliderFloat("Hardness##dir", &hardness, 0.1, 10);
					renderer.setDirectionalLightHardness(directionalLights[directionalLightSelector], hardness);

					bool castShadows = renderer.getDirectionalLightShadows(directionalLights[directionalLightSelector]);
					ImGui::Checkbox("Cast shadows##dir", &castShadows);
					renderer.setDirectionalLightShadows(directionalLights[directionalLightSelector], castShadows);


					//ImGui::SliderFloat3("frustumSplit",
					//	&renderer.directionalShadows.frustumSplits[0], 0, 1);

					ImGui::PopID();
				}
			}

			{
				
				ImGui::NewLine();

				static int spotLightSelector = -1;
				ImGui::Text("Spot lightd Count %d", spotLights.size());
				ImGui::InputInt("Current spot light:", &spotLightSelector);
				int n = ImGui::Button("New Spot Light"); ImGui::SameLine();
				int remove = ImGui::Button("Remove Spot Light");

				int lightSize = renderer.getSpotLightShadowSize();
				ImGui::DragInt("Spot light shadow texture size", &lightSize);
				renderer.setSpotLightShadowSize(lightSize);

				if (spotLightSelector < -1)
				{
					spotLightSelector = -1;
				}

				if (n || spotLightSelector >= (int)spotLights.size())
				{

					spotLights.push_back(renderer.createSpotLight({ 0,0,0 }, glm::radians(90.f), 
						{ 0,-1,0 }));
				}

				spotLightSelector
					= std::min(spotLightSelector, (int)spotLights.size() - 1);

				if (remove)
				{
					if (spotLightSelector >= 0)
					{
						renderer.deleteSpotLight(spotLights[spotLightSelector]);

						spotLights.erase(spotLights.begin() + spotLightSelector);
						
						spotLightSelector = std::min(spotLightSelector,
							(int)renderer.internal.spotLights.size() - 1);
					}

				}

				ImGui::NewLine();

				if (spotLightSelector >= 0)
				{
					ImGui::PushID(14);

					glm::vec3 color = renderer.getSpotLightColor(spotLights[spotLightSelector]);
					ImGui::ColorEdit3("Color##spot", &color[0]);
					renderer.setSpotLightColor(spotLights[spotLightSelector], color);

					glm::vec3 position = renderer.getSpotLightPosition(spotLights[spotLightSelector]);
					ImGui::DragFloat3("Position##spot", &position[0], 0.1);
					renderer.setSpotLightPosition(spotLights[spotLightSelector], position);

					glm::vec3 direction = renderer.getSpotLightDirection(spotLights[spotLightSelector]);
					ImGui::DragFloat3("Direction##spot", &direction[0], 0.05);
					renderer.setSpotLightDirection(spotLights[spotLightSelector], direction);

					float distance = renderer.getSpotLightDistance(spotLights[spotLightSelector]);
					ImGui::DragFloat("Distance##spot", &distance, 0.05, 0);
					renderer.setSpotLightDistance(spotLights[spotLightSelector], distance);

					float attenuation = renderer.getSpotLightAttenuation(spotLights[spotLightSelector]);
					ImGui::DragFloat("Attenuation##spot", &attenuation, 0.05, 0);
					renderer.setSpotLightAttenuation(spotLights[spotLightSelector], attenuation);

					float hardness = renderer.getSpotLightHardness(spotLights[spotLightSelector]);
					ImGui::DragFloat("Hardness##spot", &hardness, 0.05, 0, 20);
					renderer.setSpotLightHardness(spotLights[spotLightSelector], hardness);


					float angle = renderer.getSpotLightFov(spotLights[spotLightSelector]);
					ImGui::SliderAngle("fov", &angle, 0, 180);
					renderer.setSpotLightFov(spotLights[spotLightSelector], angle);

					bool castShadows = renderer.getSpotLightShadows(spotLights[spotLightSelector]);
					ImGui::Checkbox("Cast shadows##spot", &castShadows);
					renderer.setSpotLightShadows(spotLights[spotLightSelector], castShadows);


					ImGui::PopID();
				}
			}

			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::ColorEdit3("Global Ambient color", &renderer.skyBox.color[0]);
		
			ImGui::End();
			ImGui::PopID();

		}

		//imgui objectEditor
		if (cubeEditor)
		{
		
			ImGui::Begin("Object Editor", &cubeEditor, flags);
			ImGui::SetWindowFontScale(1.2f);

			ImGui::ListBox("World objects", &itemCurrent, items.data(), models.size(), 4);

			if (ImGui::Button("Add barrel"))
			{
				items.push_back("Barrel");
				auto e = renderer.createEntity(barelModel);
				models.push_back(e);

			}
			ImGui::SameLine();
			if (ImGui::Button("Add rock"))
			{
				items.push_back("Rock");
				auto e = renderer.createEntity(rockModel);
				models.push_back(e);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add crate"))
			{
				items.push_back("Crate");
				auto e = renderer.createEntity(levelModel);

				//renderer.setEntityMeshMaterial(e, 0, materials[0]);
				//auto textures = renderer.getEntityMeshMaterialTextures(e, 0);
				//gl3d::GpuTexture{ renderer.getTextureOpenglId(textures.albedoTexture) }.setTextureQuality(gl3d::TextureLoadQuality::leastPossible);
				//gl3d::GpuTexture{ renderer.getTextureOpenglId(textures.emissiveTexture) }.setTextureQuality(gl3d::TextureLoadQuality::leastPossible);
				//gl3d::GpuTexture{ renderer.getTextureOpenglId(textures.pbrTexture.texture) }.setTextureQuality(gl3d::TextureLoadQuality::leastPossible);

				models.push_back(e);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add sphere"))
			{
				items.push_back("Sphere");
				auto e = renderer.createEntity(sphereModel);
				models.push_back(e);
			}
			if (ImGui::Button("Remove object") && itemCurrent < items.size() )
			{
				renderer.deleteEntity(models[itemCurrent]);

				items.erase(items.begin() + itemCurrent);
				models.erase(models.begin() + itemCurrent);
				if(itemCurrent) itemCurrent--;
			}
			
			if (itemCurrent < models.size())
			{
				auto entityNames = renderer.getEntityMeshesNames(models[itemCurrent]);
				if (entityNames)
				{
					int subitemsCount = renderer.getEntityMeshesCount(models[itemCurrent]);

					ImGui::ListBox("Object components", &subItemCurent,
						entityNames->data(), subitemsCount);
				}
			

			}
			
			ImGui::NewLine();
			ImGui::Text("Total materials: %d", 
				renderer.internal.materialIndexes.size());

			ImGui::NewLine();

			if(!models.empty() && itemCurrent < items.size())
			{
				auto transform = renderer.getEntityTransform(models[itemCurrent]);

				ImGui::NewLine();

				ImGui::Text("Object transform");
				ImGui::DragFloat3("position", &transform.position[0], 0.1);
				ImGui::DragFloat3("rotation", &transform.rotation[0], 0.1);
				ImGui::DragFloat3("scale", &transform.scale[0], 0.1);
				float s = 0;
				ImGui::InputFloat("sameScale", &s);
				ImGui::NewLine();

				int animation = renderer.getEntityAnimationIndex(models[itemCurrent]);
				float animationSpeed = renderer.getEntityAnimationSpeed(models[itemCurrent]);
				bool animate = renderer.getEntityAnimate(models[itemCurrent]);
				ImGui::Checkbox("animate: ", &animate); ImGui::SameLine();
				ImGui::InputInt("animation", &animation);
				ImGui::DragFloat("animation speed", &animationSpeed, 0.001, 0);
				renderer.setEntityAnimationIndex(models[itemCurrent], animation);
				renderer.setEntityAnimationSpeed(models[itemCurrent], animationSpeed);
				renderer.setEntityAnimate(models[itemCurrent], animate);
				ImGui::NewLine();


				ImGui::Text("Object flags");
				bool staticEntity = renderer.isEntityStatic(models[itemCurrent]);
				ImGui::Checkbox("static geometry", &staticEntity);
				renderer.setEntityStatic(models[itemCurrent], staticEntity);
				
				bool visible = renderer.isEntityVisible(models[itemCurrent]);
				ImGui::Checkbox("visible", &visible);
				renderer.setEntityVisible(models[itemCurrent], visible);

				bool castShadows = renderer.getEntityCastShadows(models[itemCurrent]);
				ImGui::Checkbox("cast shadows", &castShadows);
				renderer.setEntityCastShadows(models[itemCurrent], castShadows);

				ImGui::NewLine();

				if (s > 0)
				{
					transform.scale = glm::vec3(s);
				}

				renderer.setEntityTransform(models[itemCurrent], transform);

				if (subItemCurent < renderer.getEntityMeshesCount(models[itemCurrent]))
				{
					
					std::string name = renderer.getEntityMeshMaterialName(
						models[itemCurrent], subItemCurent);

					auto materialData = renderer.getEntityMeshMaterialValues(
						models[itemCurrent], subItemCurent);

					auto entityIndex = renderer.internal.getEntityIndex(models[itemCurrent]);
					auto entity = renderer.internal.cpuEntities[entityIndex];

					auto modelData = entity.models[subItemCurent];

					name = "Material name: " + name;

					//ImGui::Text("Min boundary %f, %f, %f", modelData.minBoundary.x, modelData.minBoundary.y, modelData.minBoundary.z);
					//ImGui::Text("Max boundary %f, %f, %f", modelData.maxBoundary.x, modelData.maxBoundary.y, modelData.maxBoundary.z);
					ImGui::Text("Object material");
					ImGui::Text(name.c_str());
					ImGui::ColorEdit3("difuse", &materialData.kd[0]);
					ImGui::SliderFloat("emmisive", &materialData.emmisive, 0, 1);
					ImGui::SliderFloat("roughness", &materialData.roughness, 0, 1);
					ImGui::SliderFloat("metallic", &materialData.metallic, 0, 1);
					ImGui::SliderFloat("ambient oclusion", &materialData.ao, 0, 1);

					renderer.setEntityMeshMaterialValues(
						models[itemCurrent], subItemCurent, materialData);

					auto drawImage = [io](const char* c, GLuint id, int w, int h, int imguiId)
						{
							ImGui::PushID(imguiId);
							ImGui::Text(c, id);
							ImGui::SameLine();

							int my_tex_w = 20;
							int my_tex_h = 20;
							ImVec2 pos = ImGui::GetCursorScreenPos();
							ImVec2 uv_min = ImVec2(1.0f, 0.0f);                 // Top-left
							ImVec2 uv_max = ImVec2(0.0f, 1.0f);                 // Lower-right
							ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
							ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
							ImGui::Image((void*)(id),
								ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);

							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								float region_sz = 64.0f * 4;
								ImGui::Image((void*)(id)
									, ImVec2(region_sz, region_sz));
								ImGui::EndTooltip();
							}

							gl3d::GpuTexture t;
							t.id = id;

							int quality = t.getTextureQuality();
							//ImGui::RadioButton("leastPossible", &quality, 0);
							//ImGui::RadioButton("nearestMipmap", &quality, 1);
							//ImGui::RadioButton("linearMipmap", &quality, 2);
							//ImGui::RadioButton("maxQuality", &quality, 3);

							//if (ImGui::RadioButton("leastPossible", quality == gl3d::leastPossible)) { quality = gl3d::leastPossible; }
							//if (ImGui::RadioButton("nearestMipmap", quality == gl3d::nearestMipmap)) { quality = gl3d::nearestMipmap; }
							//if (ImGui::RadioButton("linearMipmap", quality == gl3d::linearMipmap)) { quality = gl3d::linearMipmap; }
							//if (ImGui::RadioButton("maxQuality", quality == gl3d::maxQuality)) { quality = gl3d::maxQuality; }

							static const char const* items[] =
							{
							"leastPossible",
							"nearestMipmap",
							"linearMipmap",
							"maxQuality",
							};

							ImGui::Combo("Texture Quality", &quality, items, 4);

							t.setTextureQuality(quality);
							ImGui::PopID();
						};

					
					auto materialTextureData = renderer.getEntityMeshMaterialTextures(
					models[itemCurrent], subItemCurent);

					{
						drawImage("Object albedo, id: %d", renderer.getTextureOpenglId(materialTextureData.albedoTexture), 20, 20, __COUNTER__);
						drawImage("Object normal map, id: %d", renderer.getTextureOpenglId(materialTextureData.normalMapTexture), 20, 20, __COUNTER__);
						drawImage("Object RMA map, id: %d", renderer.getTextureOpenglId(materialTextureData.pbrTexture.texture), 20, 20, __COUNTER__);

					}

				}


			}


			ImGui::End();
		}

		if(showStats)
		{
			ImGui::Begin("Stats", &showStats, flags);
			ImGui::SetWindowFontScale(2.0f);
		
			ImGui::PlotHistogram("Fps graph", fpsArr, FPS_RECORD_ARR_SIZE, 0, 0,
			0, 60);
			ImGui::Text("Fps %d", currnetFps);

			ImGui::PlotHistogram("Mill second avg", millArr, FPS_RECORD_ARR_SIZE, 0, 0,
				0, 30);
			ImGui::Text("Milliseconds: %f", currnetMill);

			ImGui::PlotHistogram("Milli seconds graph", deltaTimeArr, DELTA_TIME_ARR_SIZE, 0, 0,
				0, 30);

			//ImGui::PlotHistogram("Frame duration graph", deltaTimeArr, DELTA_TIME_ARR_SIZE, 0, 0,
			//0, 0.32);
			//ImGui::Text("Frame duration (ms) %f", deltaTime * 1000);


			//ImGui::PlotHistogram("Render duration graph", profileeArr, Profiler_ARR_SIZE, 0, 0,
			//0, 0.32);
			//ImGui::Text("Render duration (ms) %f", lastProfilerRezult.timeSeconds * 1000);
			//renderDurationProfilerFine.imguiPlotValues();

			//renderDurationProfiler.imguiPlotValues();

			//imguiRenderDuration.imguiPlotValues();

			//swapBuffersDuration.imguiPlotValues();

			ImGui::End();
		}

		//ImGui::ShowDemoWindow(0);

	#pragma endregion


	#pragma region camera

		float speed = 4;
		glm::vec3 dir = {};
		if(GetAsyncKeyState('W'))
		{
			dir.z -= speed * deltaTime;
		}
		if (GetAsyncKeyState('S'))
		{
			dir.z += speed * deltaTime;
		}

		if (GetAsyncKeyState('A'))
		{
			dir.x -= speed * deltaTime;
		}
		if (GetAsyncKeyState('D'))
		{
			dir.x += speed * deltaTime;
		}

		if (GetAsyncKeyState('Q'))
		{
			dir.y -= speed * deltaTime;
		}
		if (GetAsyncKeyState('E'))
		{
			dir.y += speed * deltaTime;
		}

		renderer.camera.moveFPS(dir);

		{
			static glm::dvec2 lastMousePos = {};
			if (glfwGetMouseButton(wind, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				glm::dvec2 currentMousePos = {};
				glfwGetCursorPos(wind, &currentMousePos.x, &currentMousePos.y);

				float speed = 0.8f;

				glm::vec2 delta = lastMousePos - currentMousePos;
				delta *= speed * deltaTime;

				renderer.camera.rotateCamera(delta);

				lastMousePos = currentMousePos;
			}
			else
			{
				glfwGetCursorPos(wind, &lastMousePos.x, &lastMousePos.y);

			}
		}

		renderer.camera.aspectRatio = (float)w / h;

	#pragma endregion

		//cube.rotation.y += (glm::radians(360.f)/ 5 )* deltaTime;

		renderDurationProfiler.start();
		renderDurationProfilerFine.start();

		//for (int i = 0; i < models.size(); i++)
		//{
		//	//models[i].models[0].position = models[i].position;
		//	//models[i].models[0].scale = models[i].scale;
		//	//models[i].models[0].rotation = models[i].rotation;
		//
		//	renderer.renderModel(models[i].obj, models[i].position, models[i].rotation, models[i].scale);
		//	
		//}
		//
		
		//if(showNormals)
		//{
		//	renderer.renderSubModelNormals(models[itemCurrent].obj, subItemCurent, 
		//		models[itemCurrent].position, models[itemCurrent].rotation,
		//		models[itemCurrent].scale, 0.2);
		//
		//}

		renderer.render(deltaTime);

		//renderer.renderObject(objectTest, { 0,0,0 });
		//renderer.renderObject(objectTest2, { 3,0,0 });


		//lastProfilerRezult = renderDurationProfiler.end();
		renderDurationProfiler.end();
		renderDurationProfilerFine.end();

	#pragma region light cube
		glDisable(GL_DEPTH_TEST);
		for (auto &i : renderer.internal.spotLights)
		{
			lightCubeModel.position = i.position;
		
			auto projMat = renderer.camera.getProjectionMatrix();
			auto viewMat = renderer.camera.getWorldToViewMatrix();
			auto transformMat = lightCubeModel.getTransformMatrix();
		
			auto viewProjMat = projMat * viewMat * transformMat;
			shader.bind();
			glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);
			lightCubeModel.draw();
		}
		glEnable(GL_DEPTH_TEST);
	#pragma endregion


	#pragma region render and events

		//glBindTexture(GL_TEXTURE_2D_ARRAY, renderer.directionalShadows.cascadesTexture);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		imguiRenderDuration.start();

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(wind, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow *backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		imguiRenderDuration.end();

		//glBindTexture(GL_TEXTURE_2D_ARRAY, renderer.directionalShadows.cascadesTexture);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

		swapBuffersDuration.start();
		
		glfwSwapBuffers(wind);
		glfwPollEvents();
		swapBuffersDuration.end();

		glViewport(0, 0, w, h);
		renderer.updateWindowMetrics(w, h);
	#pragma endregion

	}
	

	
	return 0;
}
