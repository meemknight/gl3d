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
				positionViewSpace,
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