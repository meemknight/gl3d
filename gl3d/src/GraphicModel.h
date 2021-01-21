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