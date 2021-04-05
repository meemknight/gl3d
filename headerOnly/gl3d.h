////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on 2021-04-05
////////////////////////////////////////////////


////////////////////////////////////////////////
//Core.h
////////////////////////////////////////////////
#pragma region Core
#pragma once
#include <glm\vec4.hpp>
#include <glm\vec3.hpp>

namespace gl3d
{
	template <class T>
	struct InterfaceCheckId
	{
		bool isNotNull()
		{
			return static_cast<T*>(this)->id_;
		}

		bool isNull()
		{
			return !this->isNotNull();
		}

	};

	struct Material : public InterfaceCheckId< Material >
	{
		int id_ = {};

		Material(int id = 0):id_(id) {};
	};

	struct Object : public InterfaceCheckId< Object >
	{
		int id_ = {};

		Object(int id = 0):id_(id) {};
	};

	struct Model: public InterfaceCheckId< Model >
	{
		int id_ = {};

		Model(int id = 0):id_(id) {};
	};

	

	struct Texture : public InterfaceCheckId< Texture >
	{
		int id_ = {};
		
		Texture(int id = 0):id_(id) {};
	};


	struct TextureDataForModel
	{
		Texture albedoTexture = {};
		Texture normalMapTexture = {};

		Texture RMA_Texture = {}; //rough metalness ambient oclusion
		int RMA_loadedTextures = {};
	};

	struct GpuMaterial
	{
		glm::vec4 kd = glm::vec4(1);; //= 0.45;//w component not used
		float roughness = 0.5f;
		float metallic = 0.1;
		float ao = 1;
		float notUsed;

		GpuMaterial setDefaultMaterial()
		{
			*this = GpuMaterial();

			return *this;
		}
	};

	//todo move
	namespace internal
	{
		

		//todo move
		struct GpuPointLight
		{
			glm::vec4 position = {};
			glm::vec4 color = { 1,1,1,0 };
		};

	};



	void assertFunc(const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");

};

#define gl3dAssert(expression) (void)(											\
			(!!(expression)) ||													\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
		)

#define gl3dAssertComment(expression, comment) (void)(								\
			(!!(expression)) ||														\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), comment)\
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

	enum TextureLoadQuality
	{
		leastPossible = 0,
		nearestMipmap,
		linearMipmap,
		maxQuality
	};

	struct GpuTexture
	{
		GLuint id = 0;

		GpuTexture() = default;
		GpuTexture(const char *file) { loadTextureFromFile(file); };

		void loadTextureFromFile(const char *file, int quality = maxQuality);
		void loadTextureFromMemory(void *data, int w, int h, int chanels = 4, int quality = maxQuality);

		void clear();

		void setTextureQuality(int quality);
		int getTextureQuality();

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

		GLuint quadBuffer = 0;
		GLuint quadVAO = 0;

		GLint u_transform = -1;
		GLint u_modelTransform = -1;
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

		GLint light_u_albedo = -1;
		GLint light_u_normals = -1;
		GLint light_u_skybox = -1;
		GLint light_u_positions = -1;
		GLint light_u_materials = -1;
		GLint light_u_eyePosition = -1;
		GLint light_u_pointLightCount = -1;
		GLint light_u_ssao = -1;
		GLint light_u_view = -1;

		GLuint materialBlockLocation = GL_INVALID_INDEX;
		GLuint materialBlockBuffer = 0;

		GLuint pointLightsBlockLocation = GL_INVALID_INDEX;
		GLuint pointLightsBlockBuffer = 0;


		GLint normalSubroutineLocation = -1;
		GLint materialSubroutineLocation = -1;
		GLint getAlbedoSubroutineLocation = -1;

		GLuint normalSubroutine_noMap = GL_INVALID_INDEX;
		GLuint normalSubroutine_normalMap = GL_INVALID_INDEX;
		
		GLuint albedoSubroutine_sampled = GL_INVALID_INDEX;
		GLuint albedoSubroutine_notSampled = GL_INVALID_INDEX;

		
		GLuint materialSubroutine_functions[8] = {
			GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX,
			GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX,
		};


		Shader geometryPassShader;
		Shader lightingPassShader;

		bool normalMap = 1; //todo remove

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
		void loadTexture(const char *name, int format = 0); //todo add enum, also it is not working yer
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
#include <algorithm>

namespace gl3d
{
	namespace internal
	{
		template <class T>
		int generateNewIndex(T indexesVec)
		{
			int id = 0;

			auto indexesCopy = indexesVec;
			std::sort(indexesCopy.begin(), indexesCopy.end());

			if (indexesCopy.empty())
			{
				id = 1;
			}
			else
			{
				id = 1;

				for (int i = 0; i < indexesCopy.size(); i++)
				{
					if (indexesCopy[i] != id)
					{
						break;
					}
					else
					{
						id++;
					}
				}

			}
			
			return id;
		};

	};

	struct Renderer3D

	{
		void init(int x, int y);
		
	#pragma region material

		std::vector<GpuMaterial> materials;
		std::vector<int> materialIndexes;
		std::vector<std::string> materialNames;
		std::vector<TextureDataForModel> materialTexturesData;
		

		//todo add texture data function
		Material createMaterial(glm::vec3 kd = glm::vec3(1), 
			float roughness = 0.5f, float metallic = 0.1, float ao = 1, std::string name = "");
		
		Material createMaterial(Material m);

		Material loadMaterial(std::string file);

		void deleteMaterial(Material m);  
		void copyMaterialData(Material dest, Material source);

		//returns 0 if not found
		GpuMaterial *getMaterialData(Material m);
		std::string *getMaterialName(Material m);

		//probably move this to internal
		TextureDataForModel *getMaterialTextures(Material m);
		bool getMaterialData(Material m, GpuMaterial *gpuMaterial,
			std::string *name, TextureDataForModel *textureData);


		//returns true if succeded
		bool setMaterialData(Material m, const GpuMaterial &data, std::string *s = nullptr);

		GpuMultipleGraphicModel *getObjectData(Object o);

	#pragma endregion

	#pragma region Texture


		std::vector <GpuTexture> loadedTextures;
		std::vector<int> loadedTexturesIndexes;
		std::vector<std::string> loadedTexturesNames;

		GpuTexture defaultTexture;

		Texture loadTexture(std::string path, bool defaultToDefaultTexture = true);
		GLuint getTextureOpenglId(Texture t);

		void deleteTexture(Texture t);

		GpuTexture *getTextureData(Texture t);

		//internal
		Texture createIntenralTexture(GpuTexture t);

	#pragma endregion



		std::vector< GpuMultipleGraphicModel > graphicModels;
		std::vector<int> graphicModelsIndexes;


		Object loadObject(std::string path, float scale = 1);
		void deleteObject(Object o);



		LightShader lightShader;
		Camera camera;
		SkyBox skyBox;

		std::vector<gl3d::internal::GpuPointLight> pointLights;

		void renderObject(Object o, glm::vec3 position, glm::vec3 rotation = {}, glm::vec3 scale = {1,1,1});
		void renderObjectNormals(Object o, glm::vec3 position, glm::vec3 rotation = {},
			glm::vec3 scale = { 1,1,1 }, float normalSize = 0.5, glm::vec3 normalColor = {0.7, 0.7, 0.1});
		void renderSubObjectNormals(Object o, int index, glm::vec3 position, glm::vec3 rotation = {}, 
			glm::vec3 scale = { 1,1,1 }, float normalSize = 0.5, glm::vec3 normalColor = { 0.7, 0.7, 0.1 });

		void renderSubObjectBorder(Object o, int index, glm::vec3 position, glm::vec3 rotation = {},
			glm::vec3 scale = { 1,1,1 }, float borderSize = 0.5, glm::vec3 borderColor = { 0.7, 0.7, 0.1 });

		//internal //todo add internal namespace
		int getMaterialIndex(Material m);
		int getObjectIndex(Object o);
		int getTextureIndex(Texture t);

		struct
		{
			Shader shader;
			GLint modelTransformLocation;
			GLint projectionLocation;
			GLint sizeLocation;
			GLint colorLocation;
		}showNormalsProgram;
	
		struct
		{
			enum bufferTargers
			{
				position = 0,
				normal,
				albedo,
				material,
				bufferCount,
			};

			unsigned int gBuffer;
			unsigned int buffers[bufferCount];
			unsigned int depthBuffer;

		}gBuffer;

		struct SSAO
		{
			//https://learnopengl.com/Advanced-Lighting/SSAO

			void create(int w, int h);
		
			GLuint noiseTexture;
			GLuint ssaoFBO;
			GLuint ssaoColorBuffer;

			Shader shader;
			
			GLint u_projection = -1;
			GLint u_view = -1;
			GLint u_gPosition = -1;
			GLint u_gNormal = -1;
			GLint u_texNoise = -1;
			GLint u_samples = -1;

			std::vector<glm::vec3> ssaoKernel;

			GLuint blurBuffer;
			GLuint blurColorBuffer;
			GLint u_ssaoInput;
			Shader blurShader;

		}ssao;



		void render();
		void updateWindowMetrics(int x, int y);

		int w; int h;

	};


	void renderLightModel(GraphicModel &model, Camera  camera, glm::vec3 lightPos, LightShader lightShader,
		GpuTexture texture, GpuTexture normalTexture, GLuint skyBoxTexture, float gama,
		const GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights);

	void renderLightModel(MultipleGraphicModels &model, Camera camera, glm::vec3 lightPos, LightShader lightShader,
		GLuint skyBoxTexture, float gama, std::vector<internal::GpuPointLight> &pointLights);
	


};
#pragma endregion


