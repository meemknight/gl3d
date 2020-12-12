#include <Windows.h>
#include <iostream>

#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <gl3d.h>

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

	if (glewInit() != GLEW_OK)
	{
		std::cout << "err initializing glew";
	}

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(wind, true);
	ImGui::StyleColorsDark();


#pragma endregion


	//glEnable(GL_CULL_FACE);

#pragma region shader
	gl3d::Shader shader;
	shader.loadShaderProgramFromFile("shaders/main.vert", "shaders/main.frag");
	shader.bind();
	GLint location = glGetUniformLocation(shader.id, "u_transform");

	if (location == -1)
	{
		std::cout << "uniform error u_transform\n";
	}

#pragma endregion

	float bufferData[] =
	{
		0.f, 0.5f, 0.f,
		-0.5f, -0.5f, 0.f,
		0.5f, -0.5f, 0.f,
	};


	GLuint vertexBuffer = 0;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


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

		ImGui_ImplGlfwGL3_NewFrame();

		ImGui::Begin("gl3d");
		ImGui::SetWindowFontScale(1.2f);


		static float f;
		static glm::vec3 color;
		ImGui::Text("Editor");                  
		ImGui::SliderFloat("f", &f, -10.0f, 10.0f);       
		ImGui::ColorEdit3("color", (float *)&color); 
		ImGui::NewLine();

		ImGui::End();


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

				float speed = 0.4;

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

		auto viewProjMat = projMat * viewMat;

		glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);

		glDrawArrays(GL_TRIANGLES, 0, 3);


	#pragma region render and events

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(wind);
		glfwPollEvents();

	#pragma endregion

	}




	return 0;
}