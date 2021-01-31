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

int w = 840;
int h = 640;

gl3d::Material material = gl3d::Material().setDefaultMaterial();


int main()
{

#pragma region init

	if (!glfwInit())
	{
		std::cout << "err initializing glfw";
	}

	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow *wind = glfwCreateWindow(w, h, "geam", nullptr, nullptr);
	glfwMakeContextCurrent(wind);
	glfwSwapInterval(1);

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



#pragma endregion

	glEnable(GL_DEPTH_TEST);

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


	gl3d::Texture crateTexture("resources/other/crate.png");
	gl3d::Texture crateNormalTexture("resources/other/crateNormal.png");


	gl3d::Texture rockTexture("resources/other/boulder.png");
	gl3d::Texture rockNormalTexture("resources/other/boulderNormal.png");

	//gl3d::Texture levelTexture("resources/obj/level.png");


#pragma endregion

	gl3d::SkyBox skyBox;
	{
		const char *names[6] = 
		{	"resources/skyBoxes/ocean/right.jpg",
			"resources/skyBoxes/ocean/left.jpg",
			"resources/skyBoxes/ocean/top.jpg",
			"resources/skyBoxes/ocean/bottom.jpg",
			"resources/skyBoxes/ocean/front.jpg",
			"resources/skyBoxes/ocean/back.jpg" };

		skyBox.createGpuData();
		skyBox.loadTexture(names);
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

	

	gl3d::GraphicModel lightCube;
	lightCube.loadFromComputedData(sizeof(cubePositions),
 cubePositions,
		sizeof(cubeIndices), cubeIndices, true);
	lightCube.scale = glm::vec3(0.05);
	lightCube.position = glm::vec3(0, 1.6, 0.5);

	//gl3d::GraphicModel cube;
	//cube.loadFromComputedData(sizeof(cubePositionsNormals), cubePositionsNormals,
	//	sizeof(cubeIndices), cubeIndices);
	//cube.loadFromFile("resources/obj/sphere.obj");
	//cube.loadFromFile("resources/other/barrel.obj");


	gl3d::LoadedModelData barelModel("resources/other/barrel.obj", 0.1);
	gl3d::LoadedModelData rockModel("resources/other/boulder.obj", 0.1);
	//gl3d::LoadedModelData levelModel("resources/sponza/sponza.obj");
	gl3d::LoadedModelData levelModel("resources/sponza2/sponza.obj", 0.008);
	//gl3d::LoadedModelData levelModel("resources/other/crate.obj", 0.01);
	gl3d::LoadedModelData sphereModel("resources/obj/sphere2.obj");
	//cube.loadFromModelMeshIndex(barelModel, 0);
	//cube.scale = glm::vec3(0.1);

	std::vector< gl3d::MultipleGraphicModels > models;
	
	static std::vector < const char* > items = {};

	gl3d::Camera camera((float)w / h, glm::radians(100.f));
	camera.position = { 0.f,0.f,2.f };

	static int itemCurrent = 0;
	static int subItemCurent = 0;
	static bool borderItem = 0;
	static bool showNormals = 0;

	static int currnetFps = 0;
	const int FPS_RECORD_ARR_SIZE = 20;
	int recordPos = 0;
	static float fpsArr[FPS_RECORD_ARR_SIZE] = { };

	const int DELTA_TIME_ARR_SIZE = 120;
	int recordPosDeltaTime = 0;
	static float deltaTimeArr[DELTA_TIME_ARR_SIZE] = { };

	int timeBeg = clock();

	static PL::AverageProfiler renderProfiler;
	static PL::ProfileRezults lastProfilerRezult = {};

	const int Profiler_ARR_SIZE = 120;
	int profilePos = 0;
	static float profileeArr[Profiler_ARR_SIZE] = { };

	const int ProfilerAverage_ARR_SIZE = 20;
	int profileAveragePos = 0;
	static float profileAverageArr[ProfilerAverage_ARR_SIZE] = { };

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

		static float fpsCounterFloat;
		static float currentFpsCounter;

		fpsCounterFloat += deltaTime;
		currentFpsCounter += 1;
		if(fpsCounterFloat >= 1)
		{
			fpsCounterFloat -= 1;
			currnetFps = currentFpsCounter;
			currentFpsCounter = 0;

			if(recordPos<FPS_RECORD_ARR_SIZE)
			{
				fpsArr[recordPos] = currnetFps;
				recordPos++;
			}else
			{
				for(int i=0;i< FPS_RECORD_ARR_SIZE-1;i++)
				{
					fpsArr[i] = fpsArr[i + 1];
				}
				fpsArr[FPS_RECORD_ARR_SIZE-1] = currnetFps;
			}

			if (profileAveragePos < ProfilerAverage_ARR_SIZE)
			{

				profileAverageArr[profileAveragePos] = renderProfiler.getAverageAndResetData().timeSeconds * 1000;
				profileAveragePos++;
			}
			else
			{
				for (int i = 0; i < ProfilerAverage_ARR_SIZE - 1; i++)
				{
					profileAverageArr[i] = profileAverageArr[i + 1];
				}
				profileAverageArr[ProfilerAverage_ARR_SIZE - 1] = renderProfiler.getAverageAndResetData().timeSeconds * 1000;
			}

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

		if (profilePos < Profiler_ARR_SIZE)
		{
			profileeArr[profilePos] = lastProfilerRezult.timeSeconds;
			profilePos++;
		}
		else
		{
			for (int i = 0; i < Profiler_ARR_SIZE - 1; i++)
			{
				profileeArr[i] = profileeArr[i + 1];
			}
			profileeArr[Profiler_ARR_SIZE - 1] = lastProfilerRezult.timeSeconds;
		}


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

			ImGui::Checkbox("Display item border", &borderItem);
			ImGui::Checkbox("Display normals", &showNormals);

			ImGui::End();
		}


		ImGuiWindowFlags flags = {};
			
		if(lightEditor)
		{
			ImGui::Begin("Light Editor", &lightEditor, flags);
			ImGui::SetWindowFontScale(1.2f);
		
			static glm::vec3 color;
			ImGui::ColorEdit3("Light Color", (float *)&color);
			ImGui::NewLine();
		
			ImGui::Text("Light 1");
			ImGui::SliderFloat3("position", &lightCube.position[0], -15, 15);

			ImGui::End();
		}

		if (cubeEditor)
		{
		
			ImGui::Begin("Object Editor", &cubeEditor, flags);
			ImGui::SetWindowFontScale(1.2f);

			ImGui::ListBox("World objects", &itemCurrent, items.data(), models.size(), 4);

			if (ImGui::Button("Add barrel"))
			{
				items.push_back("Barrel");
				gl3d::MultipleGraphicModels model;
				model.loadFromModel(barelModel);
				models.push_back(model);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add rock"))
			{
				items.push_back("Rock");
				gl3d::MultipleGraphicModels model;
				model.loadFromModel(rockModel);
				models.push_back(model);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add crate"))
			{
				items.push_back("Crate");
				gl3d::MultipleGraphicModels model;
				model.loadFromModel(levelModel);
				models.push_back(model);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add sphere"))
			{
				items.push_back("Sphere");
				gl3d::MultipleGraphicModels model;
				model.loadFromModel(sphereModel);
				models.push_back(model);
			}
			if (ImGui::Button("Remove object") && itemCurrent < items.size() )
			{
				items.erase(items.begin() + itemCurrent);
				models[itemCurrent].clear();
				models.erase(models.begin() + itemCurrent);
				if(itemCurrent) itemCurrent--;
			}
			
			if (itemCurrent < models.size())
			{
			
				int subitemsCount = models[itemCurrent].subModelsNames.size();

				ImGui::ListBox("Object components", &subItemCurent, 
					models[itemCurrent].subModelsNames.data(), subitemsCount);

			}
			
			ImGui::NewLine();

			

			if(!models.empty() && itemCurrent < items.size())
			{
				ImGui::NewLine();

				ImGui::Text("Object transform");
				ImGui::SliderFloat3("position", &models[itemCurrent].position[0], -10, 10);
				ImGui::SliderFloat3("rotation", &models[itemCurrent].rotation[0], 0, glm::radians(360.f));
				ImGui::SliderFloat3("scale", &models[itemCurrent].scale[0], 0.01, 5);
				float s = 0;
				ImGui::InputFloat("sameScale", &s);

				if (s > 0)
				{
					models[itemCurrent].scale = glm::vec3(s);
				}

				if(subItemCurent < models[itemCurrent].models.size())
				{
					auto &material = models[itemCurrent].models[subItemCurent].material;

					ImGui::Text("Object material");
					ImGui::ColorEdit3("difuse", &material.kd[0]);
					ImGui::ColorEdit3("specular", &material.ks[0]);
					ImGui::ColorEdit3("ambience", &material.ka[0]);
					ImGui::SliderFloat("roughness", &material.roughness, 0, 1);
					ImGui::SliderFloat("metallic", &material.metallic, 0, 1);
					ImGui::SliderFloat("ambient oclusion", &material.ao, 0, 1);
					ImGui::SliderFloat("specular exponent", &material.ks[3], 0, 100);
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

			ImGui::PlotHistogram("Render duration graph", profileeArr, Profiler_ARR_SIZE, 0, 0,
			0, 0.32);
			ImGui::Text("Render duration (ms) %f", lastProfilerRezult.timeSeconds * 1000);

			ImGui::PlotHistogram("Render duration average graph", profileAverageArr, ProfilerAverage_ARR_SIZE, 0, 0,
			0, 32);

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

		camera.moveFPS(dir);

		{
			static glm::dvec2 lastMousePos = {};
			if (glfwGetMouseButton(wind, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				glm::dvec2 currentMousePos = {};
				glfwGetCursorPos(wind, &currentMousePos.x, &currentMousePos.y);

				float speed = 0.4f;

				glm::vec2 delta = lastMousePos - currentMousePos;
				delta *= speed * deltaTime;

				camera.rotateCamera(delta);

				lastMousePos = currentMousePos;
			}
			else
			{
				glfwGetCursorPos(wind, &lastMousePos.x, &lastMousePos.y);

			}
		}

		camera.aspectRatio = (float)w / h;

	#pragma endregion

		//cube.rotation.y += (glm::radians(360.f)/ 5 )* deltaTime;


		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = lightCube.getTransformMatrix();

		auto viewProjMat = projMat * viewMat * transformMat;
		shader.bind();
		glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);
		lightCube.draw();

		renderProfiler.start();

		for (int i = 0; i < models.size(); i++)
		{
			//models[i].models[0].position = models[i].position;
			//models[i].models[0].scale = models[i].scale;
			//models[i].models[0].rotation = models[i].rotation;
			//if (items[i] == "Barrel")
			//{	
			//	//gl3d::renderLightModel(models[i].models[0], camera, lightCube.position, lightShader, texture, normalTexture,
			//	//	skyBox.texture, gamaCorection, material);
			//	gl3d::renderLightModel(models[i], camera, lightCube.position, lightShader,
			//	skyBox.texture, gamaCorection);
			//
			//}else if(items[i] == "Rock")
			//{
			//	//gl3d::renderLightModel(models[i].models[0], camera, lightCube.position, lightShader, rockTexture, rockNormalTexture,
			//	//	skyBox.texture, gamaCorection, material);
			//	gl3d::renderLightModel(models[i], camera, lightCube.position, lightShader,
			//	skyBox.texture, gamaCorection);
			//}
			//else if (items[i] == "Crate")
			//{
			//	//todo fix normal here
			//	gl3d::renderLightModel(models[i], camera, lightCube.position, lightShader,
			//		skyBox.texture, gamaCorection);
			//}


			gl3d::renderLightModel(models[i], camera, lightCube.position, lightShader,
				skyBox.texture, gamaCorection);

		
		}

		lastProfilerRezult = renderProfiler.end();

		if (itemCurrent	< models.size() &&
			!models[itemCurrent].models.empty() && showNormals 
			&& subItemCurent < models[itemCurrent].models.size())
		{
			showNormalsShader.bind();

			auto projMat = camera.getProjectionMatrix();
			auto viewMat = camera.getWorldToViewMatrix();
			auto transformMat = models[itemCurrent].getTransformMatrix();

			auto viewTransformMat = viewMat * transformMat;

			glUniformMatrix4fv(normalsModelTransformLocation,
				1, GL_FALSE, &viewTransformMat[0][0]);

			glUniformMatrix4fv(normalsProjectionLocation,
				1, GL_FALSE, &projMat[0][0]);

			models[itemCurrent].models[subItemCurent].draw();

		}
		
		if (itemCurrent < models.size() && borderItem && 
			models[itemCurrent].models.size() > subItemCurent)
		{
			
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);	

			auto projMat = camera.getProjectionMatrix();
			auto viewMat = camera.getWorldToViewMatrix();
			auto transformMat = models[0].getTransformMatrix();

			auto viewProjMat = projMat * viewMat * transformMat;


			lightShader.bind(viewProjMat, transformMat,
				lightCube.position, camera.position, gamaCorection,
				models[itemCurrent].models[subItemCurent].material);

			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			models[itemCurrent].models[subItemCurent].draw();
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			glDisable(GL_STENCIL_TEST);


			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
			glDepthFunc(GL_ALWAYS);
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);

			auto &m = models[itemCurrent].models[subItemCurent];
			projMat = camera.getProjectionMatrix();
			viewMat = camera.getWorldToViewMatrix();

			auto rotation = models[itemCurrent].rotation;
			auto scale = models[itemCurrent].scale;
			scale *= 1.05;
			auto position = models[itemCurrent].position;


			auto s = glm::scale(scale);
			auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
				glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
				glm::rotate(rotation.z, glm::vec3(0, 0, 1));
			auto t = glm::translate(position);

			transformMat = t * r * s;

			viewProjMat = projMat * viewMat * transformMat;

			shader.bind();
			glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);

			glBindBuffer(GL_ARRAY_BUFFER, m.vertexBuffer);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
			glVertexAttrib3f(1, 98 / 255.f, 24 / 255.f, 201 / 255.f);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.indexBuffer);
			glDrawElements(GL_TRIANGLES, m.primitiveCount, GL_UNSIGNED_INT, 0);

			glDisable(GL_STENCIL_TEST);
			glDepthFunc(GL_LESS);
		}


		
		{

			auto projMat = camera.getProjectionMatrix();
			auto viewMat = camera.getWorldToViewMatrix();
			viewMat = glm::mat4(glm::mat3(viewMat));

			auto viewProjMat = projMat * viewMat;

			skyBox.draw(viewProjMat, gamaCorection);

		}


	#pragma region render and events


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

		glfwSwapBuffers(wind);
		glfwPollEvents();
		glViewport(0, 0, w, h);

	#pragma endregion

	}
	

	
	return 0;
}