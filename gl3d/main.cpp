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

gl3d::GpuMaterial material = gl3d::GpuMaterial().setDefaultMaterial();

#define USE_GPU_ENGINE 0

#pragma region gpu
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = USE_GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = USE_GPU_ENGINE;
}
#pragma endregion

struct ObjectAndTransform
{
	gl3d::Object obj = {};
	glm::vec3 position = {};
	glm::vec3 rotation = {};
	glm::vec3 scale = {1,1,1};
};


int main()
{

#pragma region init

	if (!glfwInit())
	{
		std::cout << "err initializing glfw";
	}

	glfwWindowHint(GLFW_SAMPLES, 4);

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow *wind = glfwCreateWindow(w, h, "geam", nullptr, nullptr);
	glfwMakeContextCurrent(wind);
	glfwSwapInterval(0);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "err initializing glew";
	}

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

#pragma endregion

	gl3d::Renderer3D renderer;
	renderer.init();


#pragma region shader
	gl3d::Shader shader;
	shader.loadShaderProgramFromFile("shaders/color.vert", "shaders/color.frag");
	shader.bind();
	GLint location = glGetUniformLocation(shader.id, "u_transform");

	if (location == -1)
	{
		std::cout << "uniform error u_transform\n";
	}

	gl3d::LightShader lightShader;
	lightShader.create();


	gl3d::Shader showNormalsShader;
	showNormalsShader.loadShaderProgramFromFile("shaders/showNormals.vert",
		"shaders/showNormals.geom", "shaders/showNormals.frag");

	GLint normalsModelTransformLocation = glGetUniformLocation(showNormalsShader.id, "u_modelTransform");
	GLint normalsProjectionLocation = glGetUniformLocation(showNormalsShader.id, "u_projection");
	
#pragma endregion

#pragma region texture

	//gl3d::Texture crateTexture("resources/other/crate.png");
	//gl3d::Texture crateNormalTexture("resources/other/crateNormal.png");
	//gl3d::Texture rockTexture("resources/other/boulder.png");
	//gl3d::Texture rockNormalTexture("resources/other/boulderNormal.png");

	//gl3d::Texture levelTexture("resources/obj/level.png");

#pragma endregion

	{
		const char *names[6] = 
		{	"resources/skyBoxes/ocean/right.jpg",
			"resources/skyBoxes/ocean/left.jpg",
			"resources/skyBoxes/ocean/top.jpg",
			"resources/skyBoxes/ocean/bottom.jpg",
			"resources/skyBoxes/ocean/front.jpg",
			"resources/skyBoxes/ocean/back.jpg" };

		renderer.skyBox.loadTexture(names);
		//skyBox.loadTexture("resources/skyBoxes/ocean_1.png");
		//skyBox.loadTexture("resources/skyBoxes/uffizi_cross.png", 1);
	
	}

	
	//VertexArrayContext va;
	//va.create();


	float bufferData[] =
	{
		0.f, 1.5f, 0.f,
		-0.5f, 0.5f, 0.f,
		0.5f, 0.5f, 0.f,
	};

	float bufferData2[] =
	{
		0.f, 0.5f, -1.f,
		-0.5f, -0.5f, -1.f,
		0.5f, -0.5f, 0-1.f,
	};

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
		+0.0f, +0.0f, +1.0f, // Color
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

	renderer.pointLights.push_back(gl3d::internal::GpuPointLight());
	renderer.pointLights[0].position = glm::vec4(0, 0.42, 2.44, 0);

	gl3d::GraphicModel lightCubeModel;
	lightCubeModel.loadFromComputedData(sizeof(cubePositions),
	cubePositions,
		sizeof(cubeIndices), cubeIndices, true);
	lightCubeModel.scale = glm::vec3(0.05);

	//gl3d::GraphicModel cube;
	//cube.loadFromComputedData(sizeof(cubePositionsNormals), cubePositionsNormals,
	//	sizeof(cubeIndices), cubeIndices);
	//cube.loadFromFile("resources/obj/sphere.obj");
	//cube.loadFromFile("resources/other/barrel.obj");

	
	//gl3d::LoadedModelData barelModel("resources/other/barrel.obj", 0.1);
	auto barelModel = renderer.loadObject("resources/barrel/Barrel_01.obj", 1);
	auto rockModel = renderer.loadObject("resources/other/boulder.obj", 0.1);
	//auto levelModel = renderer.loadObject("resources/sponza/sponza.obj");
	//gl3d::LoadedModelData levelModel("resources/sponza2/sponza.obj", 0.008);
	auto levelModel = renderer.loadObject("resources/other/crate.obj", 0.01);
	auto sphereModel = renderer.loadObject("resources/obj/sphere.obj");
	//cube.loadFromModelMeshIndex(barelModel, 0);
	//cube.scale = glm::vec3(0.1);

	//auto objectTest = renderer.loadObject("resources/other/crate.obj", 0.01);
	//auto objectTest = renderer.loadObject("resources/sponza2/sponza.obj", 0.008);


	//auto objectTest = renderer.loadObject("resources/barrel/Barrel_01.obj");
	//auto objectTest2 = renderer.loadObject("resources/other/crate.obj", 0.01);

	std::vector< ObjectAndTransform > models;
	static std::vector < const char* > items = {}; //just models names

	renderer.camera.aspectRatio = (float)w / h;
	renderer.camera.fovRadians = glm::radians(100.f);
	renderer.camera.position = { 0.f,0.f,2.f };

	static int itemCurrent = 0;
	static int subItemCurent = 0;
	static bool borderItem = 0;
	static bool showNormals = 0;
	static bool normalMap = 1;

	static int currnetFps = 0;
	const int FPS_RECORD_ARR_SIZE = 20;
	int recordPos = 0;
	static float fpsArr[FPS_RECORD_ARR_SIZE] = { };

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
		glStencilMask(0xFF);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
				currentFpsCounter = 0;

				if (recordPos < FPS_RECORD_ARR_SIZE)
				{
					fpsArr[recordPos] = currnetFps;
					recordPos++;
				}
				else
				{
					for (int i = 0; i < FPS_RECORD_ARR_SIZE - 1; i++)
					{
						fpsArr[i] = fpsArr[i + 1];
					}
					fpsArr[FPS_RECORD_ARR_SIZE - 1] = currnetFps;
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
				deltaTimeArr[recordPosDeltaTime] = deltaTime;
				recordPosDeltaTime++;
			}
			else
			{
				for (int i = 0; i < DELTA_TIME_ARR_SIZE - 1; i++)
				{
					deltaTimeArr[i] = deltaTimeArr[i + 1];
				}
				deltaTimeArr[DELTA_TIME_ARR_SIZE - 1] = deltaTime;
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

			static bool cullFace = 0;
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
			
			lightShader.normalMap = normalMap;
			renderer.lightShader.normalMap = normalMap;

			ImGui::Checkbox("Display item border", &borderItem);
			ImGui::Checkbox("Display normals", &showNormals);


			ImGui::End();
		}

		ImGuiWindowFlags flags = {};
			
		//imgui light editor
		if(lightEditor)
		{
			ImGui::PushID(6996);
			ImGui::Begin("Light Editor", &lightEditor, flags);
			ImGui::SetWindowFontScale(1.2f);
		
			static int pointLightSelector = 0;
			ImGui::Text("Point lightd Count %d", renderer.pointLights.size());
			ImGui::InputInt("Current Point light:", &pointLightSelector);
			int n = ImGui::Button("New Light"); ImGui::SameLine();
			int remove = ImGui::Button("Remove Light");

			if(pointLightSelector < -1)
			{
				pointLightSelector = -1;
			}

			if(n || pointLightSelector >= (int)renderer.pointLights.size())
			{
				gl3d::internal::GpuPointLight l = {};
				l.color = { 1,1,1,0 };

				renderer.pointLights.push_back(l);
			}

			pointLightSelector = std::min(pointLightSelector, (int)renderer.pointLights.size() - 1);

			if(remove)
			{
				if(pointLightSelector>=0)
				{
					renderer.pointLights.erase(renderer.pointLights.begin() + pointLightSelector);
					pointLightSelector = std::min(pointLightSelector, (int)renderer.pointLights.size() - 1);
				}
			
			}

			ImGui::NewLine();

			if(pointLightSelector >= 0)
			{
				ImGui::PushID(12);

				ImGui::ColorEdit3("Color", &renderer.pointLights[pointLightSelector].color[0]);
				ImGui::DragFloat3("Position", &renderer.pointLights[pointLightSelector].position[0], 0.1);
				
				ImGui::PopID();
			}

			ImGui::NewLine();

		
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
				ObjectAndTransform model;
				model.obj = barelModel;
				models.push_back(model);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add rock"))
			{
				items.push_back("Rock");
				ObjectAndTransform model;
				model.obj = rockModel;
				models.push_back(model);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add crate"))
			{
				items.push_back("Crate");
				ObjectAndTransform model;
				model.obj = levelModel;
				models.push_back(model);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add sphere"))
			{
				items.push_back("Sphere");
				ObjectAndTransform model;
				model.obj = sphereModel;
				models.push_back(model);
			}
			if (ImGui::Button("Remove object") && itemCurrent < items.size() )
			{
				items.erase(items.begin() + itemCurrent);
				models.erase(models.begin() + itemCurrent);
				if(itemCurrent) itemCurrent--;
			}
			
			if (itemCurrent < models.size())
			{
				auto curentModel = renderer.getObjectData(models[itemCurrent].obj);
				int subitemsCount = curentModel->subModelsNames.size();

				ImGui::ListBox("Object components", &subItemCurent, 
					curentModel->subModelsNames.data(), subitemsCount);

			}
			
			ImGui::NewLine();


			if(!models.empty() && itemCurrent < items.size())
			{
				auto curentModel = renderer.getObjectData(models[itemCurrent].obj);

				ImGui::NewLine();

				ImGui::Text("Object transform");
				ImGui::DragFloat3("position", &models[itemCurrent].position[0], 0.1);
				ImGui::DragFloat3("rotation", &models[itemCurrent].rotation[0], 0.1);
				ImGui::DragFloat3("scale", &models[itemCurrent].scale[0], 0.1);
				float s = 0;
				ImGui::InputFloat("sameScale", &s);

				if (s > 0)
				{
					models[itemCurrent].scale = glm::vec3(s);
				}

				if(subItemCurent < curentModel->models.size())
				{
					auto &material = *renderer.getMaterialData(curentModel->models[subItemCurent].material);


					ImGui::Text("Object material");
					ImGui::ColorEdit3("difuse", &material.kd[0]);
					ImGui::SliderFloat("roughness", &material.roughness, 0, 1);
					ImGui::SliderFloat("metallic", &material.metallic, 0, 1);
					ImGui::SliderFloat("ambient oclusion", &material.ao, 0, 1);

					auto drawImage = [io](const char *c, GLuint id, int w, int h, int imguiId)
					{
						ImGui::PushID(imguiId);
						ImGui::Text(c, id);
						ImGui::SameLine();

						int my_tex_w = 20;
						int my_tex_h = 20;
						ImVec2 pos = ImGui::GetCursorScreenPos();
						ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
						ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
						ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
						ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
						ImGui::Image((void *)(id),
							ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);

						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							float region_sz = 64.0f * 4;
							ImGui::Image((void *)(id)
								, ImVec2(region_sz, region_sz ));
							ImGui::EndTooltip();
						}

						gl3d::Texture t;
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

						static const char const *items[] =
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

					drawImage("Object albedo, id: %d", curentModel->models[subItemCurent].albedoTexture.id, 20, 20, 1);
					drawImage("Object normal map, id: %d", curentModel->models[subItemCurent].normalMapTexture.id, 20, 20, 2);
					drawImage("Object RMA map, id: %d", curentModel->models[subItemCurent].RMA_Texture.id, 20, 20, 3);

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

			ImGui::PlotHistogram("Frame duration graph", deltaTimeArr, DELTA_TIME_ARR_SIZE, 0, 0,
			0, 0.32);
			ImGui::Text("Frame duration (ms) %f", deltaTime * 1000);

			//ImGui::PlotHistogram("Render duration graph", profileeArr, Profiler_ARR_SIZE, 0, 0,
			//0, 0.32);
			//ImGui::Text("Render duration (ms) %f", lastProfilerRezult.timeSeconds * 1000);
			renderDurationProfilerFine.imguiPlotValues();

			renderDurationProfiler.imguiPlotValues();

			imguiRenderDuration.imguiPlotValues();

			swapBuffersDuration.imguiPlotValues();

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

	#pragma region light cube
		for(auto &i : renderer.pointLights)
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
	#pragma endregion

		renderDurationProfiler.start();
		renderDurationProfilerFine.start();

		for (int i = 0; i < models.size(); i++)
		{
			//models[i].models[0].position = models[i].position;
			//models[i].models[0].scale = models[i].scale;
			//models[i].models[0].rotation = models[i].rotation;

			renderer.renderObject(models[i].obj, models[i].position, models[i].rotation, models[i].scale);
			renderer.renderObjectNormals(models[i].obj, models[i].position, models[i].rotation,
				models[i].scale, 0.2);

		}

		//renderer.renderObject(objectTest, { 0,0,0 });
		//renderer.renderObject(objectTest2, { 3,0,0 });


		//lastProfilerRezult = renderDurationProfiler.end();
		renderDurationProfiler.end();
		renderDurationProfilerFine.end();
		
		
		//todo border item routine
		//if (itemCurrent < models.size() && borderItem && 
		//	models[itemCurrent].models.size() > subItemCurent)
		//{
		//	
		//	glEnable(GL_STENCIL_TEST);
		//	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		//	glStencilFunc(GL_ALWAYS, 1, 0xFF);
		//	glStencilMask(0xFF);	
		//
		//	auto projMat = renderer.camera.getProjectionMatrix();
		//	auto viewMat = renderer.camera.getWorldToViewMatrix();
		//	auto transformMat = models[0].getTransformMatrix();
		//
		//	auto viewProjMat = projMat * viewMat * transformMat;
		//
		//	//todo here also change the uniform
		//	lightShader.bind(viewProjMat, transformMat,
		//		lightCubeModel.position, renderer.camera.position, gamaCorection,
		//		models[itemCurrent].models[subItemCurent].material, renderer.pointLights);
		//
		//	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//	models[itemCurrent].models[subItemCurent].draw();
		//	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//
		//	glDisable(GL_STENCIL_TEST);
		//
		//	glEnable(GL_STENCIL_TEST);
		//	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		//	glDepthFunc(GL_ALWAYS);
		//	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		//	glStencilMask(0x00);
		//
		//	auto &m = models[itemCurrent].models[subItemCurent];
		//	projMat = renderer.camera.getProjectionMatrix();
		//	viewMat = renderer.camera.getWorldToViewMatrix();
		//
		//	auto rotation = models[itemCurrent].rotation;
		//	auto scale = models[itemCurrent].scale;
		//	scale *= 1.05;
		//	auto position = models[itemCurrent].position;
		//
		//
		//	auto s = glm::scale(scale);
		//	auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
		//		glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
		//		glm::rotate(rotation.z, glm::vec3(0, 0, 1));
		//	auto t = glm::translate(position);
		//
		//	transformMat = t * r * s;
		//
		//	viewProjMat = projMat * viewMat * transformMat;
		//
		//	shader.bind();
		//	glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);
		//
		//	glBindBuffer(GL_ARRAY_BUFFER, m.vertexBuffer);
		//
		//	glEnableVertexAttribArray(0);
		//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		//	glVertexAttrib3f(1, 98 / 255.f, 24 / 255.f, 201 / 255.f);
		//
		//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.indexBuffer);
		//	glDrawElements(GL_TRIANGLES, m.primitiveCount, GL_UNSIGNED_INT, 0);
		//
		//	glDisable(GL_STENCIL_TEST);
		//	glDepthFunc(GL_LESS);
		//}

		{

			auto projMat = renderer.camera.getProjectionMatrix();
			auto viewMat = renderer.camera.getWorldToViewMatrix();
			viewMat = glm::mat4(glm::mat3(viewMat));

			auto viewProjMat = projMat * viewMat;

			renderer.skyBox.draw(viewProjMat, gamaCorection);

		}


	#pragma region render and events


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

		swapBuffersDuration.start();
		glViewport(0, 0, w, h);
		glfwSwapBuffers(wind);
		glfwPollEvents();
		swapBuffersDuration.end();

	#pragma endregion

	}
	

	
	return 0;
}