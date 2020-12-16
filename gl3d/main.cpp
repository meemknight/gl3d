#include <Windows.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "src/Core.h"
#include "src/Texture.h"
#include "src/Camera.h"
#include "src/Shader.h"
#include "src/GraphicModel.h"

#include <ctime>

int w = 840;
int h = 640;

int main()
{

#pragma region init

	if (!glfwInit())
	{
		std::cout << "err initializing glfw";
	}

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


	glEnable(GL_CULL_FACE);
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

	gl3d::Shader normalShader;
	normalShader.loadShaderProgramFromFile("shaders/normals.vert", "shaders/normals.frag");
	normalShader.bind();
	GLint normalShaderLocation = glGetUniformLocation(normalShader.id, "u_transform");
	if (normalShaderLocation == -1)
	{
		std::cout << "uniform error u_transform\n";
	}

	GLint normalShaderNormalTransformLocation = glGetUniformLocation(normalShader.id, "u_modelTransform");
	if (normalShaderNormalTransformLocation == -1)
	{
		std::cout << "uniform error u_modelTransform\n";
	}

	GLint normalShaderLightposLocation = glGetUniformLocation(normalShader.id, "u_lightPosition");
	if (normalShaderLightposLocation == -1)
	{
		std::cout << "uniform error u_lightPosition\n";
	}

	GLint eyePositionLocation = glGetUniformLocation(normalShader.id, "u_eyePosition");
	if (eyePositionLocation == -1)
	{
		std::cout << "uniform error u_eyePosition\n";
	}
	GLint textureSamplerLocation = glGetUniformLocation(normalShader.id, "u_albedoSampler");
	if (textureSamplerLocation == -1)
	{
		std::cout << "uniform error u_albedoSampler\n";
	}
	GLint normalMapSamplerLocation = glGetUniformLocation(normalShader.id, "u_normalSampler");
	if (normalMapSamplerLocation == -1)
	{
		std::cout << "uniform error u_normalSampler\n";
	}




#pragma endregion

#pragma region texture

	gl3d::Texture texture;
	texture.loadTextureFromFile("resources/crate.png");

	gl3d::Texture normalTexture;
	normalTexture.loadTextureFromFile("resources/crateNormal.png");


#pragma endregion



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

	float uv = 0.5;
	float cubePositionsNormals[] = {
		-1.0f, +1.0f, +1.0f, // 0
		+0.0f, +1.0f, +0.0f, // Normal
		0, 0,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		+1.0f, +1.0f, +1.0f, // 1
		+0.0f, +1.0f, +0.0f, // Normal
		1* uv, 0,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent
		
		+1.0f, +1.0f, -1.0f, // 2
		+0.0f, +1.0f, +0.0f, // Normal
		1* uv, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent
		
		-1.0f, +1.0f, -1.0f, // 3
		+0.0f, +1.0f, +0.0f, // Normal
		0, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent



		-1.0f, +1.0f, -1.0f, // 4
		 0.0f, +0.0f, -1.0f, // Normal
		 0, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		+1.0f, +1.0f, -1.0f, // 5
		 0.0f, +0.0f, -1.0f, // Normal
		 1* uv, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		 +1.0f, -1.0f, -1.0f, // 6
		 0.0f, +0.0f, -1.0f, // Normal
		 1* uv, 0,				 //uv
		 0,0,0,				 //tangent
		 0,0,0,				 //btangent

		-1.0f, -1.0f, -1.0f, // 7
		 0.0f, +0.0f, -1.0f, // Normal
		 0, 0,				 //uv
		 0,0,0,				 //tangent
		0,0,0,				 //btangent

		+1.0f, +1.0f, -1.0f, // 8
		+1.0f, +0.0f, +0.0f, // Normal
		1* uv, 0,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		+1.0f, +1.0f, +1.0f, // 9
		+1.0f, +0.0f, +0.0f, // Normal
		1* uv, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		+1.0f, -1.0f, +1.0f, // 10
		+1.0f, +0.0f, +0.0f, // Normal
		0, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		+1.0f, -1.0f, -1.0f, // 11
		+1.0f, +0.0f, +0.0f, // Normal
		0, 0,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		-1.0f, +1.0f, +1.0f, // 12
		-1.0f, +0.0f, +0.0f, // Normal
		1* uv, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		-1.0f, +1.0f, -1.0f, // 13
		-1.0f, +0.0f, +0.0f, // Normal
		1* uv, 0,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		-1.0f, -1.0f, -1.0f, // 14
		-1.0f, +0.0f, +0.0f, // Normal
		0, 0,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		-1.0f, -1.0f, +1.0f, // 15
		-1.0f, +0.0f, +0.0f, // Normal
		0, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent


		+1.0f, +1.0f, +1.0f, // 16
		+0.0f, +0.0f, +1.0f, // Normal
		1* uv, 1* uv,				 //uv
		0,0,0,				 //tangent
		0,0,0,				 //btangent

		-1.0f, +1.0f, +1.0f, // 17
		+0.0f, +0.0f, +1.0f, // Normal
		0, 1* uv,				 //uv
		0, 0, 0,				 //tangent
		0, 0, 0,				 //btangent

		-1.0f, -1.0f, +1.0f, // 18
		+0.0f, +0.0f, +1.0f, // Normal
		0, 0,				 //uv
		0, 0, 0,				 //tangent
		0, 0, 0,				 //btangent

		+1.0f, -1.0f, +1.0f, // 19
		+0.0f, +0.0f, +1.0f, // Normal
		1* uv, 0,				 //uv
		0, 0, 0,				 //tangent
		0, 0, 0,				 //btangent


		+1.0f, -1.0f, -1.0f, // 20
		+0.0f, -1.0f, +0.0f, // Normal
		1* uv, 0,				 //uv
			0, 0, 0,				 //tangent
			0, 0, 0,				 //btangent

		-1.0f, -1.0f, -1.0f, // 21
		+0.0f, -1.0f, +0.0f, // Normal
		0, 0,				 //uv
		0, 0, 0,				 //tangent
		0, 0, 0,				 //btangent

		-1.0f, -1.0f, +1.0f, // 22
		+0.0f, -1.0f, +0.0f, // Normal
		0, 1* uv,				 //uv
		0, 0, 0,				 //tangent
		0, 0, 0,				 //btangent

		+1.0f, -1.0f, +1.0f, // 23
		+0.0f, -1.0f, +0.0f, // Normal
		1* uv, 1* uv,				 //uv
		 0, 0, 0,				 //tangent
		 0, 0, 0,				 //btangent

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
	lightCube.loadFromData(sizeof(cubePositions), cubePositions,
		sizeof(cubeIndices), cubeIndices, true);
	lightCube.scale = glm::vec3(0.1);
	lightCube.position = glm::vec3(0, 1.6, 0.5);

	gl3d::GraphicModel cube;
	cube.loadFromData(sizeof(cubePositionsNormals), cubePositionsNormals,
		sizeof(cubeIndices), cubeIndices);


	gl3d::Camera camera((float)w / h, glm::radians(100.f));
	camera.position = { 0.f,0.f,2.f };

	int timeBeg = clock();
	 
	while (!glfwWindowShouldClose(wind))
	{
		glfwGetWindowSize(wind, &w, &h);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int timeEnd = clock();
		float deltaTime = (timeEnd - timeBeg) / 1000.f;
		timeBeg = clock();

	#pragma region imgui

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
	
		{
			ImGui::Begin("Menu");
			
			ImGui::Checkbox("Light Editor##check", &lightEditor);
			ImGui::Checkbox("Cube Editor##check", &cubeEditor);
		
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
			ImGui::SliderFloat3("position", &lightCube.position[0], -10, 10);

			ImGui::End();
		}

		if (cubeEditor)
		{
			ImGui::Begin("Cube Editor", &cubeEditor, flags);
			ImGui::SetWindowFontScale(1.2f);

			static glm::vec3 color;
			ImGui::ColorEdit3("Object Color", (float *)&color);
			ImGui::NewLine();

			ImGui::Text("Cube");
			ImGui::SliderFloat3("position", &cube.position[0], -10, 10);
			ImGui::SliderFloat3("rotation", &cube.rotation[0], 0, glm::radians(360.f));
			ImGui::SliderFloat3("scale", &cube.scale[0], 0.1, 5);
			ImGui::SameLine();
			float s = 0;
			ImGui::InputFloat("sameScale", &s);

			if (s > 0)
			{
				cube.scale = glm::vec3(s);
			}

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



		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = lightCube.getTransformMatrix();

		auto viewProjMat = projMat * viewMat * transformMat;
		shader.bind();
		glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);
		lightCube.draw();


		transformMat = cube.getTransformMatrix();

		viewProjMat = projMat * viewMat * transformMat;
		normalShader.bind();
		glUniformMatrix4fv(normalShaderLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniformMatrix4fv(normalShaderNormalTransformLocation, 1, GL_FALSE, &transformMat[0][0]);
		glUniform3fv(normalShaderLightposLocation, 1, &lightCube.position[0]);
		glUniform3fv(eyePositionLocation, 1, &camera.position[0]);
		glUniform1i(textureSamplerLocation, 0);
		glUniform1i(normalMapSamplerLocation, 1);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.id);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture.id);

		cube.draw();


	#pragma region render and events


		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(wind, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//glClear(GL_COLOR_BUFFER_BIT);
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