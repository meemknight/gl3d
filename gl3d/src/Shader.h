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


		void bind();

		void clear();
	};

	GLint getUniform(GLuint id, const char *name);

	//todo this will probably dissapear
	struct LightShader
	{
		void create();
		void bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition, float gama
		, const Material &material);

		void getSubroutines();

		GLint normalShaderLocation = -1;
		GLint normalShaderNormalTransformLocation = -1;
		GLint normalShaderLightposLocation = -1;
		GLint textureSamplerLocation = -1; 
		GLint normalMapSamplerLocation = -1;
		GLint eyePositionLocation = -1;
		GLint skyBoxSamplerLocation = -1;
		GLint gamaLocation = -1;
		GLint roughnessSamplerLocation = -1;
		GLint ambientSamplerLocation = -1;
		GLint metallicSamplerLocation = -1;

		GLuint materialBlockLocation = -1;
		GLuint materialBlockBuffer = 0;

		GLint normalSubroutineLocation = -1;

		GLuint normalSubroutine_noMap = GL_INVALID_INDEX;
		GLuint normalSubroutine_normalMap = GL_INVALID_INDEX;


		Shader shader;

		bool normalMap = 1; //todo remove

		//todo clear
	};

};