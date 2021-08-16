////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on 2021-08-16
////////////////////////////////////////////////


////////////////////////////////////////////////
//Core.h
////////////////////////////////////////////////
#pragma region Core
#pragma once
#include <glm\vec4.hpp>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <gl\glew.h>

namespace gl3d
{
	//todo optimization also hold the last found position

#define CREATE_RENDERER_OBJECT_HANDLE(x)	\
	struct x								\
	{										\
		int id_ = {};						\
		x (int id=0):id_(id){};				\
	}

	CREATE_RENDERER_OBJECT_HANDLE(Material);
	CREATE_RENDERER_OBJECT_HANDLE(Entity);
	CREATE_RENDERER_OBJECT_HANDLE(Model);
	CREATE_RENDERER_OBJECT_HANDLE(Texture);
	CREATE_RENDERER_OBJECT_HANDLE(SpotLight);

#undef CREATE_RENDERER_OBJECT_HANDLE(x)



	struct TextureDataForModel
	{
		Texture albedoTexture = {};
		Texture normalMapTexture = {};

		Texture RMA_Texture = {}; //rough metalness ambient oclusion
		Texture emissiveTexture= {};
		int RMA_loadedTextures = {};
	};

	struct GpuMaterial
	{
		glm::vec4 kd = glm::vec4(1); //w component not used
		
		//rma
		float roughness = 0.5f;
		float metallic = 0.1;
		float ao = 1;
		float emmisive = 0;
		//rma

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
			glm::vec3 position = {};
			float dist = 20;
			glm::vec3 color = { 1,1,1 };
			float attenuation = 2;

		};

		struct GpuDirectionalLight
		{
			glm::vec4 direction = {0,-1,0, 0};
			glm::vec3 color = { 1,1,1 };
			float hardness = 1;
			glm::mat4 lightSpaceMatrix[3]; //todo magic number
		
		};

		struct GpuSpotLight
		{
			glm::vec3 position = {};
			float cosHalfAngle = std::cos(3.14159/4.f);
			glm::vec3 direction = { 0,-1,0 };
			float dist = 20;
			glm::vec3 color = { 1, 1, 1 };
			float attenuation = 2;
			float hardness = 1;
			int shadowIndex = 0;
			int castShadows = 1;	//todo implement
			int	changedThisFrame = true; //this is sent to the gpu but not used there
			glm::mat4 lightSpaceMatrix;
		};


	};

	void GLAPIENTRY glDebugOutput(GLenum source,
								GLenum type,
								unsigned int id,
								GLenum severity,
								GLsizei length,
								const char *message,
								const void *userParam);

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
		dontSet = -1, //won't create mipmap
		leastPossible = 0,
		nearestMipmap,
		linearMipmap,
		maxQuality
	};

	struct GpuTexture
	{
		GLuint id = 0;

		GpuTexture() = default;
		//GpuTexture(const char *file) { loadTextureFromFile(file); };

		void loadTextureFromFile(const char *file, int quality = maxQuality, int channels = 4);
		void loadTextureFromMemory(void *data, int w, int h, int chanels = 4, int quality = maxQuality);

		//one if there is alpha data
		int loadTextureFromFileAndCheckAlpha(const char* file, int quality = maxQuality, int channels = 4);

		void clear();

		void setTextureQuality(int quality);
		int getTextureQuality();

	};

	namespace internal
	{
		struct GpuTextureWithFlags
		{
			GpuTextureWithFlags() = default;
			GpuTexture texture;
			unsigned int flags = 0; //
		};
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
#include "Texture.h"

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

		struct
		{
			GLuint quadBuffer = 0;
			GLuint quadVAO = 0;
		}quadDrawer;

		GLint u_transform = -1;
		GLint u_modelTransform = -1;
		GLint u_motelViewTransform = -1;
		GLint normalShaderLightposLocation = -1;
		GLint textureSamplerLocation = -1; 
		GLint normalMapSamplerLocation = -1;
		GLint eyePositionLocation = -1;
		GLint skyBoxSamplerLocation = -1;
		GLint gamaLocation = -1;
		GLint RMASamplerLocation = -1;
		GLint u_emissiveTexture = -1;
		GLint pointLightCountLocation = -1;
		GLint pointLightBufferLocation = -1;
		GLint materialIndexLocation = -1;

		GLint light_u_albedo = -1;
		GLint light_u_normals = -1;
		GLint light_u_skyboxFiltered = -1;
		GLint light_u_positions = -1;
		GLint light_u_materials = -1;
		GLint light_u_eyePosition = -1;
		GLint light_u_pointLightCount = -1;
		GLint light_u_directionalLightCount = -1;
		GLint light_u_spotLightCount = -1;
		GLint light_u_ssao = -1;
		GLint light_u_view = -1;
		GLint light_u_skyboxIradiance = -1;
		GLint light_u_brdfTexture = -1;
		GLint light_u_emmisive = -1;
		GLint light_u_cascades = -1;
		GLint light_u_spotShadows = -1;
		

		GLuint materialBlockLocation = GL_INVALID_INDEX;
		GLuint materialBlockBuffer = 0;

		GLuint pointLightsBlockLocation = GL_INVALID_INDEX;
		GLuint pointLightsBlockBuffer = 0;

		GLuint directionalLightsBlockLocation = GL_INVALID_INDEX;
		GLuint directionalLightsBlockBuffer = 0;

		GLuint spotLightsBlockLocation = GL_INVALID_INDEX;
		GLuint spotLightsBlockBuffer = 0;


		GLint normalSubroutineLocation = -1;
		GLint materialSubroutineLocation = -1;
		GLint getAlbedoSubroutineLocation = -1;
		GLint getEmmisiveSubroutineLocation = -1;

		GLuint normalSubroutine_noMap = GL_INVALID_INDEX;
		GLuint normalSubroutine_normalMap = GL_INVALID_INDEX;
		
		GLuint albedoSubroutine_sampled = GL_INVALID_INDEX;
		GLuint albedoSubroutine_notSampled = GL_INVALID_INDEX;
		
		GLuint emissiveSubroutine_sampled = GL_INVALID_INDEX;
		GLuint emissiveSubroutine_notSampled = GL_INVALID_INDEX;

		
		GLuint materialSubroutine_functions[8] = {
			GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX,
			GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX,
		};

		//todo refactor and move things here
		struct
		{
			//the uniform block stuff
			GLuint u_lightPassData;
			GLuint lightPassDataBlockBuffer;
			//

		}lightPassShaderData;


		//to pass to the shader as an uniform block (light pass shader)
		struct LightPassData
		{
			glm::vec4 ambientLight = glm::vec4(1, 1, 1, 0); //last value is not used
			float bloomTresshold = 1.f;
			int lightSubScater = 1;
			float exposure = 1;

		}lightPassUniformBlockCpuData;

		struct
		{
			Shader shader;
			GLint u_transform;
			GLint u_hasTexture;
			GLint u_albedoSampler;
		}prePass;

		Shader geometryPassShader;
		Shader lightingPassShader;

		bool normalMap = 1; 
		bool useSSAO = 1;
		
		//todo split stuff into separate things
		bool bloom = 1;
		int bloomBlurPasses = 4;

		GpuTexture brdfTexture;

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

	void generateTangentSpace(glm::vec3 v, glm::vec3& outUp, glm::vec3& outRight);

	//near w,h   far w, h     center near, far
	void computeFrustumDimensions(glm::vec3 position, glm::vec3 viewDirection,
		float fovRadians, float aspectRatio, float nearPlane, float farPlane,
		glm::vec2& nearDimensions, glm::vec2& farDimensions, glm::vec3& centerNear,
		glm::vec3& centerFar);

	void computeFrustumSplitCorners(glm::vec3 directionVector, 
		glm::vec2 nearDimensions, glm::vec2 farDimensions, glm::vec3 centerNear, glm::vec3 centerFar,
		glm::vec3& nearTopLeft, glm::vec3& nearTopRight, glm::vec3& nearBottomLeft, glm::vec3& nearBottomRight,
		glm::vec3& farTopLeft, glm::vec3& farTopRight, glm::vec3& farBottomLeft, glm::vec3& farBottomRight
		);

	glm::vec3 fromAnglesToDirection(float zenith, float azimuth);
	glm::vec2 fromDirectionToAngles(glm::vec3 direction);

	struct Camera
	{
		Camera() = default;
		Camera(float aspectRatio, float fovRadians)
			:aspectRatio(aspectRatio),
			fovRadians(fovRadians)
		{}

		glm::vec3 up = { 0.f,1.f,0.f };

		float aspectRatio = 1;
		float fovRadians = glm::radians(60.f);

		float closePlane = 0.01f;
		float farPlane = 200.f;


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



	struct Transform
	{
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = { 1,1,1 };

		glm::mat4 getTransformMatrix();

		bool operator==(const Transform& other)
		{
			return 
				(position == other.position)
				&&(rotation == other.rotation)
				&&(scale == other.scale)
				;
		};

		bool operator!=(const Transform& other)
		{
			return !(*this == other);
		};
	};

	glm::mat4 getTransformMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	glm::mat4 getTransformMatrix(const Transform &t);

	struct LoadedModelData
	{
		LoadedModelData() = default;
		LoadedModelData(const char *file, float scale = 1.f) { load(file, scale); }

		void load(const char *file, float scale = 1.f);

		objl::Loader loader;
		std::string path;
	};
	

	struct DebugGraphicModel
	{
		std::string name = {};

		GLuint vertexArray = 0;

		GLuint vertexBuffer = 0;
		GLuint indexBuffer = 0;

		GLsizei primitiveCount = 0;

		void loadFromComputedData(size_t vertexSize, const float * vercies, size_t indexSize = 0, const unsigned int * indexes = nullptr, bool noTexture = false);

		void clear();

		void draw();

		//todo probably move this in the final version
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = {1,1,1};
		
		glm::mat4 getTransformMatrix();

	};


	struct GraphicModel
	{
		std::string name;

		GLuint vertexArray = 0;

		GLuint vertexBuffer = 0;
		GLuint indexBuffer = 0;

		GLsizei primitiveCount = 0;

		void loadFromComputedData(size_t vertexSize, const float *vercies, size_t indexSize = 0,
			const unsigned int *indexes = nullptr, bool noTexture = false);

		void clear();

		Material material;
	
	};


	struct MultipleGraphicModel
	{

		std::vector < GraphicModel >models;
		std::vector < char* > subModelsNames; //for imgui

		void clear();
	
	};

	//the data for an entity
	struct CpuEntity
	{
		Transform transform;

		Model model;

		unsigned char flags = {}; // lsb -> 1 static

		bool isStatic() { return (flags & 0b0000'0001); }
		void setStatic(bool s)
		{
			if (s)
			{
				flags = flags | 0b0000'0001;
			}
			else
			{
				flags = flags & ~(0b0000'0001);
			}
		}


	};

	struct LoadedTextures
	{
		std::string name;
		GpuTexture t;
	};

#pragma region skyBox

	struct SkyBox
	{
		GLuint texture = 0;				//environment cubemap
		GLuint convolutedTexture = 0;	//convoluted environment (used for difuse iradiance)
		GLuint preFilteredMap = 0;		//multiple mipmaps used for speclar 

		void clearData();
	};

	struct SkyBoxLoaderAndDrawer
	{
		GLuint vertexArray = 0;
		GLuint vertexBuffer = 0;
		GLuint captureFBO;

		void createGpuData();

		struct
		{
			Shader shader;
			GLuint samplerUniformLocation;
			GLuint modelViewUniformLocation;
			GLuint u_exposure;
			GLuint u_ambient;

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

		struct
		{
			Shader shader;
			GLuint u_environmentMap;
			GLuint u_roughness;
			GLuint modelViewUniformLocation;

		}preFilterSpecular;

		struct
		{
			Shader shader;
			//GLuint u_lightPos;
			//GLuint u_g;
			//GLuint u_g2;
			GLuint modelViewUniformLocation;

		}atmosphericScatteringShader;

		enum CrossskyBoxFormats
		{
			BottomOfTheCrossRight,
			BottomOfTheCrossDown,
			BottomOfTheCrossLeft,
		};

		void loadTexture(const char *names[6], SkyBox &skyBox);
		void loadTexture(const char *name, SkyBox &skyBox, int format = 0);
		void loadHDRtexture(const char *name, SkyBox &skyBox);
		void atmosphericScattering(glm::vec3 sun, float g, float g2, SkyBox& skyBox);

		void createConvolutedAndPrefilteredTextureData(SkyBox &skyBox);

		//void clearGpuData();
		void draw(const glm::mat4& viewProjMat, SkyBox& skyBox, float exposure,
			glm::vec3 ambient);
		void drawBefore(const glm::mat4 &viewProjMat, SkyBox &skyBox, float exposure,
			glm::vec3 ambient);

	};

	/*
	
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"

	*/

#pragma endregion


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
#include <algorithm>

namespace gl3d
{
	namespace internal
	{

		//todo probably just keep a counter and get the next one
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

		MultipleGraphicModel *getModelData(Model o);

	#pragma endregion

	#pragma region Texture


		//GpuTexture defaultTexture; //todo refactor this so it doesn't have an index or sthing

		Texture loadTexture(std::string path);
		GLuint getTextureOpenglId(Texture t);

		void deleteTexture(Texture t);

		GpuTexture* getTextureData(Texture t);

		//internal
		Texture createIntenralTexture(GpuTexture t, int alphaData);
		Texture createIntenralTexture(GLuint id_, int alphaData);

	#pragma endregion

	#pragma region skyBox

		void renderSkyBox(); //todo this thing will dissapear after the render function will do everything
		void renderSkyBoxBefore(); //todo this thing will dissapear after the render function will do everything
		SkyBox loadSkyBox(const char* names[6]);
		SkyBox loadSkyBox(const char* name, int format = 0);
		SkyBox loadHDRSkyBox(const char* name);

		SkyBox atmosfericScattering(glm::vec3 sun, float g, float g2);

	#pragma endregion

	#pragma region model

		Model loadModel(std::string path, float scale = 1);
		void deleteModel(Model o);

	#pragma endregion
	
	#pragma region spot light

		SpotLight createSpotLight(glm::vec3 position, float fov,
			glm::vec3 direction, float dist = 20, float attenuation = 1, 
			glm::vec3 color = glm::vec3(1), float hardness = 1, int castShadows = 1);

		//angles is the angle from zenith and azimuth
		SpotLight createSpotLight(glm::vec3 position, float fov,
			glm::vec2 angles, float dist = 20, float attenuation = 1,
			glm::vec3 color = glm::vec3(1), float hardness = 1, int castShadows = 1);

		void deleteSpotLight(SpotLight& l);

		glm::vec3 getSpotLightPosition(SpotLight& l);
		void setSpotLightPosition(SpotLight& l, glm::vec3 position);
		bool isSpotLight(SpotLight& l);
		glm::vec3 getSpotLightColor(SpotLight& l);
		void setSpotLightColor(SpotLight& l, glm::vec3 color);
		float getSpotLightFov(SpotLight& l);
		void setSpotLightFov(SpotLight& l, float fov);
		glm::vec3 getSpotLightDirection(SpotLight& l);
		void setSpotLightDirection(SpotLight& l, glm::vec3 direction);
		float getSpotLightDistance(SpotLight& l); //light distance
		void setSpotLightDistance(SpotLight& l, float distance); //light distance
		float getSpotLightAttenuation(SpotLight& l); //light distance
		void setSpotLightAttenuation(SpotLight& l, float attenuation); //light distance
		float getSpotLightHardness(SpotLight& l);
		void setSpotLightHardness(SpotLight& l, float hardness);
		void setSpotLightShadows(SpotLight& l, bool castShadows);
		bool getSpotLightShadows(SpotLight& l);

	#pragma endregion


	#pragma region Entity
	
		Entity createEntity(Model m, Transform transform = {}, bool staticGeometry = 0);
		CpuEntity* getEntityData(Entity &e); //todo this will probably dissapear
		Transform getEntityTransform(Entity &e);
		void setEntityTransform(Entity &e, Transform transform);
		bool isEntityStatic(Entity &e);
		void setEntityStatic(Entity &e, bool s = true);
		void deleteEntity(Entity& e);
		int getEntitySubModelCount(Entity& e);
		bool isEntity(Entity& e);

	#pragma endregion

		struct VAO
		{
			//this is not used yet
			GLuint posNormalTexture;
			void createVAOs();
		}vao;


		LightShader lightShader;
		Camera camera;
		SkyBox skyBox;

		std::vector<gl3d::internal::GpuPointLight> pointLights;
		std::vector<gl3d::internal::GpuDirectionalLight> directionalLights;


		void renderModelNormals(Model o, glm::vec3 position, glm::vec3 rotation = {},
			glm::vec3 scale = { 1,1,1 }, float normalSize = 0.5, glm::vec3 normalColor = {0.7, 0.7, 0.1});
		void renderSubModelNormals(Model o, int index, glm::vec3 position, glm::vec3 rotation = {},
			glm::vec3 scale = { 1,1,1 }, float normalSize = 0.5, glm::vec3 normalColor = { 0.7, 0.7, 0.1 });

		void renderSubModelBorder(Model o, int index, glm::vec3 position, glm::vec3 rotation = {},
			glm::vec3 scale = { 1,1,1 }, float borderSize = 0.5, glm::vec3 borderColor = { 0.7, 0.7, 0.1 });


		struct InternalStruct
		{
			struct PBRtextureMaker
			{
				Shader shader;
				GLuint fbo;

				void init();

				GLuint createRMAtexture(int w, int h,
					GpuTexture roughness, GpuTexture metallic, GpuTexture ambientOcclusion, 
					GLuint quadVAO);

			}pBRtextureMaker;

			SkyBoxLoaderAndDrawer skyBoxLoaderAndDrawer;

			int getMaterialIndex(Material m);
			int getModelIndex(Model o);
			int getTextureIndex(Texture t);
			int getEntityIndex(Entity t);
			int getSpotLightIndex(SpotLight l);

			//material
			std::vector<GpuMaterial> materials;
			std::vector<int> materialIndexes;
			std::vector<std::string> materialNames;
			std::vector<TextureDataForModel> materialTexturesData;

			//texture
			std::vector <internal::GpuTextureWithFlags> loadedTextures;
			std::vector<int> loadedTexturesIndexes;
			std::vector<std::string> loadedTexturesNames;
		
			//models
			std::vector< MultipleGraphicModel > graphicModels;
			std::vector<int> graphicModelsIndexes;

			//entities
			std::vector<CpuEntity> cpuEntities;
			std::vector<int> entitiesIndexes;

			//spot lights
			std::vector<internal::GpuSpotLight> spotLights;
			std::vector<int> spotLightIndexes;

			struct PerFrameFlags
			{
				bool staticGeometryChanged = 0;
				bool shouldUpdateSpotShadows = 0;

			}perFrameFlags;


		}internal;

	
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
				positionViewSpace,
				emissive,
				bufferCount,
			};

			unsigned int gBuffer;
			unsigned int buffers[bufferCount];
			unsigned int depthBuffer;

		}gBuffer;

		struct PostProcess
		{
			Shader postProcessShader;
			Shader gausianBLurShader;
			GLint u_colorTexture;	//post process shader
			GLint u_bloomTexture;	//post process shader
			GLint u_bloomNotBluredTexture;	//post process shader
			GLint u_bloomIntensity;	//post process shader
			GLint u_exposure;		//post process shader
			GLint u_useSSAO;	//post process shader
			GLint u_ssaoExponent;	//post process shader
			GLint u_ssao;	//post process shader


			GLint u_toBlurcolorInput;
			GLint u_horizontal;


			GLuint fbo;
			GLuint blurFbo[2];

			GLuint colorBuffers[2]; // 0 for color, 1 for bloom
			GLuint bluredColorBuffer[2];
			void create(int w, int h);

			//for post process shader
			float bloomIntensty = 1;

		}postProcess;

		struct SSAO
		{
			//https://learnopengl.com/Advanced-Lighting/SSAO

			void create(int w, int h);
		
			GLuint noiseTexture;
			GLuint ssaoFBO;
			GLuint ssaoColorBuffer;

			Shader shader;
			
			GLuint ssaoUniformBlockBuffer;
			struct SsaoShaderUniformBlockData
			{
				float radius = 0.2;
				float bias = 0.025;
				int samplesTestSize = 16; // should be less than kernelSize (64)

			}ssaoShaderUniformBlockData;

			GLint u_projection = -1;
			GLint u_view = -1;
			GLint u_gPosition = -1;
			GLint u_gNormal = -1;
			GLint u_texNoise = -1;
			GLint u_samples = -1;
			GLuint u_SSAODATA;

			std::vector<glm::vec3> ssaoKernel;

			GLuint blurBuffer;
			GLuint blurColorBuffer;
			GLint u_ssaoInput;
			Shader blurShader;

		}ssao;

		struct DirectionalShadows
		{
			void create();
			void allocateTextures(int count);

			constexpr static int CASCADES = 3;

			int textureCount = 0;

			GLuint cascadesTexture;
			GLuint cascadesFbo;
			static constexpr int shadowSize = 2048;

			float frustumSplits[CASCADES] = { 0.01,0.03,0.1};


		}directionalShadows;

		struct SpotShadows
		{
			void create();
			void allocateTextures(int count);
			int textureCount = 0;

			GLuint shadowTextures;
			GLuint staticGeometryTextures;
			GLuint fbo;
			GLuint staticGeometryfbo;

			static constexpr int shadowSize = 1024;

			

		}spotShadows;


		struct RenderDepthMap
		{
			void create();

			Shader shader;
			GLint u_depth = -1;

			GLuint fbo;
			GLuint texture;

		}renderDepthMap;

		void renderADepthMap(GLuint texture);

		void render();
		void updateWindowMetrics(int x, int y);

		float ssao_finalColor_exponent = 5.f;

		int w; int h;

	};




};
#pragma endregion


