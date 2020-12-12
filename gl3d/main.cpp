#include <Windows.h>
#include <iostream>

#include <GLFW/glfw3.h>
#include <GL/glew.h>


#include "Shader.h"


int main()
{

	if (!glfwInit())
	{
		std::cout << "err initializing glfw";
	}

	GLFWwindow *wind = glfwCreateWindow(840, 640, "geam", nullptr, nullptr);
	glfwMakeContextCurrent(wind);

	if(glewInit() != GLEW_OK)
	{
		std::cout << "err initializing glew";
	}


	glEnable(GL_CULL_FACE);

	Shader shader;
	shader.loadShaderProgramFromFile("shaders/main.vert", "shaders/main.frag");


	while (!glfwWindowShouldClose(wind))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBegin(GL_TRIANGLES);


		glVertex2f(0, 0.5);
		glVertex2f(-0.5, -0.5);
		glVertex2f(0.5, -0.5);


		glEnd();


		glfwSwapBuffers(wind);
		glfwPollEvents();
	}




	return 0;
}