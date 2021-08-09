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

		GpuMultipleGraphicModel *getModelData(Model o);

	#pragma endregion

	#pragma region Texture


		//GpuTexture defaultTexture; //todo refactor this so it doesn't have an index or sthing

		Texture loadTexture(std::string path);
		GLuint getTextureOpenglId(Texture t);

		void deleteTexture(Texture t);

		GpuTexture *getTextureData(Texture t);

		//internal
		Texture createIntenralTexture(GpuTexture t, int alphaData);
		Texture createIntenralTexture(GLuint id_, int alphaData);

	#pragma endregion

	#pragma region skyBox

		void renderSkyBox(); //todo this thing will dissapear after the render function will do everything
		void renderSkyBoxBefore(); //todo this thing will dissapear after the render function will do everything
		SkyBox loadSkyBox(const char *names[6]);
		SkyBox loadSkyBox(const char *name, int format = 0);
		SkyBox loadHDRSkyBox(const char *name);

		SkyBox atmosfericScattering(glm::vec3 sun, float g, float g2);

	#pragma endregion

	#pragma region model

		Model loadModel(std::string path, float scale = 1);
		void deleteModel(Model o);

	#pragma endregion

	#pragma region Entity
	
		Entity createEntity(Model m, Transform transform = {});
		CpuEntity* getEntityData(Entity e);
		void deleteEntity(Entity e);

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

		void renderModel(Model o, glm::vec3 position, glm::vec3 rotation = {}, glm::vec3 scale = {1,1,1});


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
			std::vector< GpuMultipleGraphicModel > graphicModels;
			std::vector<int> graphicModelsIndexes;

			//entities
			std::vector<CpuEntity> cpuEntities;
			std::vector<int> entitiesIndexes;


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
			constexpr static int CASCADES = 3;

			GLuint cascadesTexture;
			GLuint cascadesFbo;
			static constexpr int shadowSize = 1024;

			float frustumSplits[CASCADES] = { 0.01,0.03,0.1};

			GLuint varianceShadowFBO;
			GLuint varianceShadowTexture;
			GLuint depthForVarianceTexture;

			Shader varianceShadowShader;
			GLint u_transform;
			GLint u_hasTexture;
			GLint u_albedoSampler;

		}directionalShadows;


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