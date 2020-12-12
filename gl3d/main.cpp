#include <Windows.h>
#include <iostream>

#include <GLFW/glfw3.h>
#include <GL/glew.h>


#include "Shader.h"
#include "Camera.h"

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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int timeEnd = clock();
		float deltaTime = (timeEnd - timeBeg) / 1000.f;
		timeBeg = clock();
		
		float speed = 1;

		if(GetAsyncKeyState('W'))
		{
			camera.position.z -= speed * deltaTime;
		}
		if (GetAsyncKeyState('S'))
		{
			camera.position.z += speed * deltaTime;
		}

		if (GetAsyncKeyState('A'))
		{
			camera.position.x -= speed * deltaTime;
		}
		if (GetAsyncKeyState('D'))
		{
			camera.position.x += speed * deltaTime;
		}

		
		auto projMat = camera.getProjectionMatrix();

		auto viewMat = camera.getWorldToViewMatrix();

		auto viewProjMat = projMat * viewMat;

		glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);

		glDrawArrays(GL_TRIANGLES, 0, 3);


		glfwSwapBuffers(wind);
		glfwPollEvents();
	}




	return 0;
}