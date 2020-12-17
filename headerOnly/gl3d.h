////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on 2020-12-17
////////////////////////////////////////////////


////////////////////////////////////////////////
//Core.h
////////////////////////////////////////////////
#pragma region Core
#pragma once

namespace gl3d
{
	
	void assertFunc(const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");


};

#define gl3dAssert(expression) (void)(											\
			(!!(expression)) ||												\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
		)

#define gl3dAssertComment(expression, comment) (void)(								\
			(!!(expression)) ||														\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), 0, comment)	\
		)

#pragma endregion


////////////////////////////////////////////////
//Texture.h
////////////////////////////////////////////////
#pragma region Texture
#pragma once
#include <GL/glew.h>

namespace gl3d
{

	struct Texture
	{
		GLuint id = 0;

		void loadTextureFromFile(const char *file);
		void loadTextureFromMemory(void *data, int w, int h);


	};

};
#pragma endregion


////////////////////////////////////////////////
//Shader.h
////////////////////////////////////////////////
#pragma region Shader
#pragma once
#include "GL/glew.h"
#include <glm\mat4x4.hpp>

namespace gl3d
{

	struct Shader
	{

		GLuint id = 0;

		bool loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader);

		void deleteShaderProgram();

		void bind();


		//todo clear
	};

	//todo this will probably dissapear
	struct LightShader
	{
		void create();
		void bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition);

		Shader shader;

		GLint normalShaderLocation = -1;
		GLint normalShaderNormalTransformLocation = -1;
		GLint normalShaderLightposLocation = -1;
		GLint textureSamplerLocation = -1; 
		GLint normalMapSamplerLocation = -1;
		GLint eyePositionLocation = -1;

		//todo clear
	};

};
#pragma endregion


////////////////////////////////////////////////
//Camera.h
////////////////////////////////////////////////
#pragma region Camera
#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include <cmath>

namespace gl3d
{

	constexpr float PI = 3.1415926535897932384626433;

	struct Camera
	{
		Camera() = default;
		Camera(float aspectRatio, float fovRadians)
			:aspectRatio(aspectRatio),
			fovRadians(fovRadians)
		{}

		glm::vec3 up = { 0.f,1.f,0.f };

		float aspectRatio = 1;
		float fovRadians = glm::radians(100.f);

		float closePlane = 0.01f;
		float farPlane = 100.f;


		glm::vec3 position = {};
		glm::vec3 viewDirection = {0,0,-1};

		glm::mat4x4 getProjectionMatrix();

		glm::mat4x4 getWorldToViewMatrix();

		void rotateCamera(const glm::vec2 delta);

		void moveFPS(glm::vec3 direction);


	};

};
#pragma endregion


////////////////////////////////////////////////
//GraphicModel.h
////////////////////////////////////////////////
#pragma region GraphicModel
#pragma once
#include "GL/glew.h"
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <OBJ_Loader.h>

namespace gl3d
{

	struct LoadedModelData
	{
		LoadedModelData() = default;
		LoadedModelData(const char *file) { load(file); }

		void load(const char *file);

		objl::Loader loader;
	};


	//todo this will dissapear and become an struct of arrays or sthing
	struct GraphicModel
	{

		//todo probably this will disapear
		GLuint vertexArray = 0;

		GLuint vertexBuffer = 0;
		GLuint indexBuffer = 0;

		GLsizei primitiveCount = 0;

		//todo check if indexes can be uint
		void loadFromComputedData(size_t vertexSize, const float * vercies, size_t indexSize = 0, const unsigned int * indexes = nullptr, bool noTexture = false);

		//deprecated
		void loadFromData(size_t vertexCount, float *vertices, float *normals, float *textureUV,
		size_t indexesCount = 0, unsigned int *indexes = nullptr);

		void loadFromModelMeshIndex(const LoadedModelData &model, int index);

		//deprecated
		void loadFromFile(const char *fileName);

		void clear();

		void draw();

		//todo probably move this in the final version
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = {1,1,1};
		
		glm::mat4 getTransformMatrix();
	};

};
#pragma endregion


////////////////////////////////////////////////
//gl3d.h
////////////////////////////////////////////////
#pragma region gl3d
#pragma once

#include <Core.h>
#include <Texture.h>
#include <Shader.h>
#include <Camera.h>
#include <GraphicModel.h>

namespace gl3d
{
	void renderLightModel(GraphicModel &model, Camera  camera, glm::vec3 lightPos, LightShader lightShader,
		Texture texture, Texture normalTexture);

};
#pragma endregion


