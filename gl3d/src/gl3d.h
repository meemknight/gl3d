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
		

		//todo add texture data overloads
		Material createMaterial(glm::vec3 kd = glm::vec3(1), 
			float roughness = 0.5f, float metallic = 0.1, float ao = 1, std::string name = "");
		
		Material createMaterial(Material m);

		Material loadMaterial(std::string file);

		bool deleteMaterial(Material m);  
		bool copyMaterialData(Material dest, Material source);

		MaterialValues getMaterialValues(Material m);
		void setMaterialValues(Material m, MaterialValues values);

		std::string getMaterialName(Material m);
		void setMaterialName(Material m, const std::string& name);
		
		TextureDataForMaterial getMaterialTextures(Material m);
		void setMaterialTextures(Material m, TextureDataForMaterial textures);

		bool isMaterial(Material& m);

		//returns true if succeded
		//bool setMaterialData(Material m, const MaterialValues &data, std::string *s = nullptr);

	#pragma endregion

	#pragma region Texture

		//GpuTexture defaultTexture; //todo refactor this so it doesn't have an index or sthing

		Texture loadTexture(std::string path);
		GLuint getTextureOpenglId(Texture& t);
		bool isTexture(Texture& t);

		void deleteTexture(Texture& t);

		GpuTexture* getTextureData(Texture& t);

		//internal
		Texture createIntenralTexture(GpuTexture t, int alphaData);
		Texture createIntenralTexture(GLuint id_, int alphaData);

		PBRTexture createPBRTexture(Texture& roughness, Texture& metallic,
			Texture& ambientOcclusion);
		void deletePBRTexture(PBRTexture &t);

	#pragma endregion

	#pragma region skyBox

		void renderSkyBox(); //todo this thing will dissapear after the render function will do everything
		void renderSkyBoxBefore(); //todo this thing will dissapear after the render function will do everything
		SkyBox loadSkyBox(const char* names[6]);
		SkyBox loadSkyBox(const char* name, int format = 0);
		SkyBox loadHDRSkyBox(const char* name);
		void deleteSkyBoxTextures(SkyBox& skyBox);

		SkyBox atmosfericScattering(glm::vec3 sun, float g, float g2);

	#pragma endregion

	#pragma region model

		//todo implement stuff here

		Model loadModel(std::string path, float scale = 1);
		bool isModel(Model& m);
		void deleteModel(Model &m);

		void clearModelData(Model& m);
		int getModelMeshesCount(Model& m);
		std::string getModelMeshesName(Model& m, int index);

		//for apis like imgui
		std::vector<char*>* getModelMeshesNames(Model& m);

	#pragma endregion
	
	#pragma region point lights
		PointLight createPointLight(glm::vec3 position, glm::vec3 color = glm::vec3(1.f),
			float dist = 20, float attenuation = 1);
		void detletePointLight(PointLight& l);

		glm::vec3 getPointLightPosition(PointLight& l);
		void setPointLightPosition(PointLight& l, glm::vec3 position);
		bool isPointLight(PointLight& l);
		glm::vec3 getPointLightColor(PointLight& l);
		void setPointLightColor(PointLight& l, glm::vec3 color);
		float getPointLightDistance(PointLight& l); //light distance
		void setPointLightDistance(PointLight& l, float distance); //light distance
		float getPointLightAttenuation(PointLight& l); //light distance
		void setPointLightAttenuation(PointLight& l, float attenuation); //light distance
		bool getPointLightShadows(PointLight& l);
		void setPointLightShadows(PointLight& l, bool castShadows = true);
		float getPointLightHardness(PointLight& l);
		void setPointLightHardness(PointLight& l, float hardness);
		int getPointLightShadowSize();
		void setPointLightShadowSize(int size);

	#pragma endregion

	#pragma region directional light

		DirectionalLight createDirectionalLight(glm::vec3 direction, 
			glm::vec3 color = glm::vec3(1.f), float hardness = 1, bool castShadows = 1);
		void deleteDirectionalLight(DirectionalLight& l);
		bool isDirectionalLight(DirectionalLight& l);
		
		glm::vec3 getDirectionalLightDirection(DirectionalLight& l);
		void setDirectionalLightDirection(DirectionalLight& l, glm::vec3 direction);

		glm::vec3 getDirectionalLightColor(DirectionalLight& l);
		void setDirectionalLightColor(DirectionalLight& l, glm::vec3 color);

		float getDirectionalLightHardness(DirectionalLight& l);
		void setDirectionalLightHardness(DirectionalLight& l, float hardness);
		
		bool getDirectionalLightShadows(DirectionalLight& l);
		void setDirectionalLightShadows(DirectionalLight& l, bool castShadows);
		
		int getDirectionalLightShadowSize();
		void setDirectionalLightShadowSize(int size);

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
		int getSpotLightShadowSize();
		void setSpotLightShadowSize(int size);

	#pragma endregion


	#pragma region Entity
	
		Entity createEntity(Model m, Transform transform = {}, 
			bool staticGeometry = 1, bool visible = 1, bool castShadows = 1);

		Entity duplicateEntity(Entity &e);

		void setEntityModel(Entity& e, Model m);
		void clearEntityModel(Entity& e);
		CpuEntity* getEntityData(Entity &e); //todo this will probably dissapear
		Transform getEntityTransform(Entity &e);
		void setEntityTransform(Entity &e, Transform transform);
		bool isEntityStatic(Entity &e);
		void setEntityStatic(Entity &e, bool s = true);
		void deleteEntity(Entity& e);
		bool isEntity(Entity& e);
		bool isEntityVisible(Entity& e);
		void setEntityVisible(Entity& e, bool v = true);
		void setEntityCastShadows(Entity& e, bool s = true);
		bool getEntityCastShadows(Entity& e);
		
		//this is used for apis like imgui.
		std::vector<char*> *getEntityMeshesNames(Entity& e);

		int getEntityMeshesCount(Entity& e);
		MaterialValues getEntityMeshMaterialValues(Entity& e, int meshIndex);
		void setEntityMeshMaterialValues(Entity& e, int meshIndex, MaterialValues mat);

		std::string getEntityMeshMaterialName(Entity& e, int meshIndex);
		void setEntityMeshMaterialName(Entity& e, int meshIndex, const std::string &name);
		
		void setEntityMeshMaterial(Entity& e, int meshIndex, Material mat);
		
		TextureDataForMaterial getEntityMeshMaterialTextures(Entity& e, int meshIndex);
		void setEntityMeshMaterialTextures(Entity& e, int meshIndex, TextureDataForMaterial texture);

	#pragma endregion

	#pragma region settings

		void setExposure(float exposure);
		float getExposure();

		//cheap
		void enableNormalMapping(bool normalMapping = 1);
		bool isNormalMappingEnabeled();

		//cheap
		void enableLightSubScattering(bool lightSubScatter = 1);
		bool isLightSubScatteringEnabeled();

		//rather expensive
		void enableSSAO(bool ssao = 1);
		bool isSSAOenabeled();
		float getSSAOBias();
		void setSSAOBias(float bias);
		float getSSAORadius();
		void setSSAORadius(float radius);
		int getSSAOSampleCount();
		void setSSAOSampleCount(int samples);
		float getSSAOExponent();
		void setSSAOExponent(float exponent);

		//very little performance penalty
		void enableFXAA(bool fxaa = 1);
		bool isFXAAenabeled();


	#pragma endregion

		struct VAO
		{
			//this is not used yet
			GLuint posNormalTexture;
			void createVAOs();
		}vao;

		Camera camera;
		SkyBox skyBox;

		void renderModelNormals(Model o, glm::vec3 position, glm::vec3 rotation = {},
			glm::vec3 scale = { 1,1,1 }, float normalSize = 0.5, glm::vec3 normalColor = {0.7, 0.7, 0.1});
		void renderSubModelNormals(Model o, int index, glm::vec3 position, glm::vec3 rotation = {},
			glm::vec3 scale = { 1,1,1 }, float normalSize = 0.5, glm::vec3 normalColor = { 0.7, 0.7, 0.1 });

		void renderSubModelBorder(Model o, int index, glm::vec3 position, glm::vec3 rotation = {},
			glm::vec3 scale = { 1,1,1 }, float borderSize = 0.5, glm::vec3 borderColor = { 0.7, 0.7, 0.1 });


		struct InternalStruct
		{
			Camera lastFrameCamera;
			LightShader lightShader;

			int w; int h;
			int adaptiveW; int adaptiveH;

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
			int getPointLightIndex(PointLight l);
			int getDirectionalLightIndex(DirectionalLight l);

			//material
			std::vector<MaterialValues> materials;
			std::vector<int> materialIndexes;
			std::vector<std::string> materialNames;
			std::vector<TextureDataForMaterial> materialTexturesData;

			bool getMaterialData(Material m, MaterialValues* gpuMaterial,
				std::string* name, TextureDataForMaterial* textureData);

			//texture
			std::vector <internal::GpuTextureWithFlags> loadedTextures;
			std::vector<int> loadedTexturesIndexes;
			std::vector<std::string> loadedTexturesNames;
		
			//models
			std::vector< ModelData > graphicModels;
			std::vector<int> graphicModelsIndexes;

			ModelData* getModelData(Model o);

			//entities
			std::vector<CpuEntity> cpuEntities;
			std::vector<int> entitiesIndexes;

			//spot lights
			std::vector<internal::GpuSpotLight> spotLights;
			std::vector<int> spotLightIndexes;

			//point lights
			std::vector<internal::GpuPointLight> pointLights;
			std::vector<int> pointLightIndexes;

			//directional lights
			std::vector<internal::GpuDirectionalLight> directionalLights;
			std::vector<int> directionalLightIndexes;


			struct PerFrameFlags
			{
				bool staticGeometryChanged = 0;
				bool shouldUpdateSpotShadows = 0;
				bool shouldUpdatePointShadows = 0;
				bool shouldUpdateDirectionalShadows = 0;

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
	
		struct GBuffer
		{
			void create(int w, int h);
			void resize(int w, int h);

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

			glm::ivec2 currentDimensions = {};

		}gBuffer;

		struct PostProcess
		{
			struct
			{
				Shader shader;
				GLint u_texture;
				GLint u_exposure;
				GLint u_tresshold;
			}filterShader;

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
			GLuint filterFbo;
			GLuint blurFbo[2];

			GLuint colorBuffers[2]; // 0 for color, 1 for bloom
			GLuint bluredColorBuffer[2];
			void create(int w, int h);
			void resize(int w, int h);
			glm::ivec2 currentDimensions = {};

			//for post process shader
			float bloomIntensty = 1;

		}postProcess;

		//used for adaptive resolution or fxaa or both
		struct AdaptiveResolution
		{
			void create(int w, int h);
			void resize(int w, int h);

			static constexpr int timeSamplesCount = 15;
			float msSampled[timeSamplesCount] = {};
			int timeSample = 0;

			float stepDownSecTarget = 17.f;
			float stepUpSecTarget = 12.f;

			glm::ivec2 currentDimensions = {};
			float rezRatio = 1.f;
			float maxScaleDown = 0.6;
			bool useAdaptiveResolution = true;
			bool shouldUseAdaptiveResolution = false;

			GLuint texture;
			GLuint fbo;

		}adaptiveResolution;

		struct AntiAlias
		{
			Shader shader;
			Shader noAAshader;
			void create(int w, int h);

			GLuint u_texture;
			GLuint noAAu_texture;
			bool usingFXAA = true;
		}antiAlias;

		struct SSAO
		{
			//https://learnopengl.com/Advanced-Lighting/SSAO

			void create(int w, int h);
			void resize(int w, int h);
			
			glm::ivec2 currentDimensions = {};

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

			float ssao_finalColor_exponent = 5.f;

		}ssao;

		struct DirectionalShadows
		{
			void create();
			void allocateTextures(int count);

			constexpr static int CASCADES = 3;

			int textureCount = 0;

			GLuint cascadesTexture;
			GLuint staticGeometryTexture;
			GLuint cascadesFbo;
			GLuint staticGeometryFbo;
			int shadowSize = 2048;
			int currentShadowSize = 2048;

			float frustumSplits[CASCADES] = { 0.01,0.03,0.1 };


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

			int shadowSize = 1024;
			int currentShadowSize = 1024;

		}spotShadows;

		struct PointShadows
		{
			void create();
			void allocateTextures(int count);
			int textureCount = 0;

			int shadowSize = 1024;
			int currentShadowSize = 1024;


			GLuint shadowTextures;
			GLuint staticGeometryTextures;
			GLuint fbo;
			GLuint staticGeometryFbo;

		}pointShadows;

		//todo remove
		struct RenderDepthMap
		{
			void create();

			Shader shader;
			GLint u_depth = -1;

			GLuint fbo;
			GLuint texture;

		}renderDepthMap;

		void renderADepthMap(GLuint texture);

		void render(float deltaTime);
		void updateWindowMetrics(int x, int y);


	};




};