#pragma once
#include "GL/glew.h"

namespace gl3d
{

	struct Shader
	{

		GLuint id = 0;

		bool loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader);

		void deleteShaderProgram();

		void bind();

	};

};