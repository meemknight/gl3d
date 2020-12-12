#pragma once
#include "GL/glew.h"

struct Shader
{

	GLuint id = 0;

	bool loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader);

	void deleteShaderProgram();


};

