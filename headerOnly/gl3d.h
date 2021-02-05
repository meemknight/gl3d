////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on 2021-02-05
////////////////////////////////////////////////


////////////////////////////////////////////////
//Core.h
////////////////////////////////////////////////
#pragma region Core
#pragma once
#include <glm\vec4.hpp>

namespace gl3d
{
	
	void assertFunc(const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");

	//todo move
	struct Material
	{
		glm::vec4 ka; //= 0.5; //w component not used
		glm::vec4 kd; //= 0.45;//w component not used
		glm::vec4 ks; //= 1;	 ;//w component is the specular exponent
		float roughness = 0.65f;
		float metallic = 0.1;
		float ao = 0.5;
		Material setDefaultMaterial()
		{
			ka = glm::vec4(0.2);
			kd = glm::vec4(0.45);
			ks = glm::vec4(1);
			ks.w = 32;
			roughness = 0.65f;
			metallic = 0.1;
			ao = 0.5;
			return *this;
		}
	};

};

#define gl3dAssert(expression) (void)(											\
			(!!(expression)) ||												\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
		)

#define gl3dAssertComment(expression, comment) (void)(								\
			(!!(expression)) ||														\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), comment)	\
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

		Texture() = default;
		Texture(const char *file) { loadTextureFromFile(file); };

		void loadTextureFromFile(const char *file);
		void loadTextureFromMemory(void *data, int w, int h);

		void clear();
	};


	void gausianBlurRGB(unsigned char *data, int w, int h, int kernel);


};
#pragma endregion


////////////////////////////////////////////////
//Shader.h
////////////////////////////////////////////////
#pragma region Shader
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

#include "Shader.h"
#include "Texture.h"

namespace gl3d
{

	struct LoadedModelData
	{
		LoadedModelData() = default;
		LoadedModelData(const char *file, float scale = 1.f) { load(file, scale); }

		void load(const char *file, float scale = 1.f);

		objl::Loader loader;
		std::string path;
	};
	

	//todo this will dissapear and become an struct of arrays or sthing
	struct GraphicModel
	{
		std::string name = {};

		//todo this might disapear
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

		void loadFromModelMesh(const LoadedModelData &model);

		//deprecated
		void loadFromFile(const char *fileName);

		void clear();

		void draw();

		//todo probably move this in the final version
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = {1,1,1};
		
		glm::mat4 getTransformMatrix();

		//todo probably teporarily add this things
		Texture albedoTexture;
		Texture normalMapTexture;
		Material material;

	};

	
	//todo this will defenetly dissapear it is just for qucik render
	struct MultipleGraphicModels
	{
		std::vector < GraphicModel >models;
		std::vector < char *> subModelsNames;

		void loadFromModel(const LoadedModelData &model);

		void clear();

		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = { 1,1,1 };


		//todo move into a standalone function
		glm::mat4 getTransformMatrix()
		{
			auto s = glm::scale(scale);
			auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
				glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
				glm::rotate(rotation.z, glm::vec3(0, 0, 1));
			auto t = glm::translate(position);

			return t * r * s;

		}

	};


	struct SkyBox
	{
		GLuint vertexArray = 0;
		GLuint vertexBuffer = 0;

		void createGpuData();
		void loadTexture(const char *names[6]);
		void loadTexture(const char *name, int format = 0); //todo add enum
		void clearGpuData();
		void draw(const glm::mat4 &viewProjMat, float gama);

		void bindCubeMap();

		Shader shader;
		GLuint texture;

		GLuint samplerUniformLocation;
		GLuint modelViewUniformLocation;
		GLuint gamaUniformLocation;

	};

	/*
	
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"

	*/


};;;
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
		Texture texture, Texture normalTexture, GLuint skyBoxTexture, float gama,
		const Material &material);

	void renderLightModel(MultipleGraphicModels &model, Camera camera, glm::vec3 lightPos, LightShader lightShader,
		GLuint skyBoxTexture, float gama);
	

};
#pragma endregion


