#pragma once
#include "GL/glew.h"
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "OBJ_Loader.h"

#include "Shader.h"
#include "Texture.h"
#include "Core.h"

namespace gl3d
{

	glm::mat4 getTransformMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

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
		GpuTexture albedoTexture;
		GpuTexture normalMapTexture;

		GpuTexture RMA_Texture; //rough metalness ambient oclusion
		int RMA_loadedTextures;

		GpuMaterial material;

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

		glm::mat4 getTransformMatrix()
		{
			return gl3d::getTransformMatrix(position, rotation, scale);
		}

	};
	

	struct GpuGraphicModel
	{
		std::string name;

		GLuint vertexArray = 0;

		GLuint vertexBuffer = 0;
		GLuint indexBuffer = 0;

		GLsizei primitiveCount = 0;

		void loadFromComputedData(size_t vertexSize, const float *vercies, size_t indexSize = 0,
			const unsigned int *indexes = nullptr, bool noTexture = false);


		void clear();


		//todo probably teporarily add this things
		//Texture albedoTexture;
		//Texture normalMapTexture;
		//Texture RMA_Texture; //rough metalness ambient oclusion
		//int RMA_loadedTextures;

		Material material;
	
	};


	struct GpuMultipleGraphicModel
	{

		std::vector < GpuGraphicModel >models;
		std::vector < char* > subModelsNames;

		void clear();
	
	};

	struct LoadedTextures
	{
		std::string name;
		GpuTexture t;
	};

	struct SkyBox
	{
		GLuint vertexArray = 0;
		GLuint vertexBuffer = 0;

		void createGpuData();
		void loadTexture(const char *names[6]);
		void loadTexture(const char *name, int format = 0); //todo add enum, also it is not working yet
		void loadHDRtexture(const char *name, int w, int h); 
		void createConvolutedTexture(int w, int h); //screen w, h

		void clearGpuData();
		void draw(const glm::mat4 &viewProjMat);

		void bindCubeMap();

		struct
		{
			Shader shader;
			GLuint samplerUniformLocation;
			GLuint modelViewUniformLocation;

		}normalSkyBox;

		struct
		{
			Shader shader;
			GLuint u_equirectangularMap;
			GLuint modelViewUniformLocation;

		}hdrtoCubeMap;

		struct
		{
			Shader shader;
			GLuint u_environmentMap;
			GLuint modelViewUniformLocation;

		}convolute;


		GLuint texture;
		GLuint convolutedTexture;

		

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