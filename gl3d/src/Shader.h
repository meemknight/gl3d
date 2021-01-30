#pragma once
#include "GL/glew.h"
#include <glm\mat4x4.hpp>
#include <Core.h>

namespace gl3d
{

	struct Shader
	{

		GLuint id = 0;

		bool loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader);
		bool loadShaderProgramFromFile(const char *vertexShader, 
			const char *geometryShader, const char *fragmentShader);

		void deleteShaderProgram();

		void bind();


		//todo clear
	};

	GLint getUniform(GLuint id, const char *name);

	//todo this will probably dissapear
	struct LightShader
	{
		void create();
		void bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition, float gama
		, const Material &material);


		GLint normalShaderLocation = -1;
		GLint normalShaderNormalTransformLocation = -1;
		GLint normalShaderLightposLocation = -1;
		GLint textureSamplerLocation = -1; 
		GLint normalMapSamplerLocation = -1;
		GLint eyePositionLocation = -1;
		GLint skyBoxSamplerLocation = -1;
		GLint gamaLocation = -1;

		GLuint materialBlockLocation = -1;
		GLuint materialBlockBuffer = 0;

		Shader shader;


		//todo clear
	};

};