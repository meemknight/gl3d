#pragma once
#include "GL/glew.h"
#include <glm\mat4x4.hpp>
#include <Core.h>
#include <vector>

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
		, const GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights);

		void setData(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition, float gama
		, const GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights);

		void setMaterial(const GpuMaterial &material);

		void getSubroutines();

		GLint normalShaderLocation = -1;
		GLint normalShaderNormalTransformLocation = -1;
		GLint normalShaderLightposLocation = -1;
		GLint textureSamplerLocation = -1; 
		GLint normalMapSamplerLocation = -1;
		GLint eyePositionLocation = -1;
		GLint skyBoxSamplerLocation = -1;
		GLint gamaLocation = -1;
		GLint RMASamplerLocation = -1;
		GLint pointLightCountLocation = -1;
		GLint pointLightBufferLocation = -1;
		GLint materialIndexLocation = -1;

		GLuint materialBlockLocation = GL_INVALID_INDEX;
		GLuint materialBlockBuffer = 0;

		GLuint pointLightsBlockLocation = GL_INVALID_INDEX;
		GLuint pointLightsBlockBuffer = 0;


		GLint normalSubroutineLocation = -1;
		GLint materialSubroutineLocation = -1;

		GLuint normalSubroutine_noMap = GL_INVALID_INDEX;
		GLuint normalSubroutine_normalMap = GL_INVALID_INDEX;
		
		
		GLuint materialSubroutine_functions[8] = {
			GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX,
			GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX,
		};


		Shader shader;

		bool normalMap = 1; //todo remove

		//todo clear
	};



};