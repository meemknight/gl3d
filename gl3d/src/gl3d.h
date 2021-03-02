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
		void init();
		
	#pragma region material

		std::vector< GpuMaterial > materials;
		std::vector<int> materialIndexes;
		std::vector<std::string> materialNames;
		
		Material createMaterial(glm::vec3 kd = glm::vec3(1), 
			float roughness = 0.5f, float metallic = 0.1, float ao = 1, std::string name = "");
		
		Material createMaterial(Material m);

		void deleteMaterial(Material m);
		void copyMaterialData(Material dest, Material source);

		GpuMaterial *getMaterialData(Material m, std::string *s = nullptr);

		//returns true if succeded
		bool setMaterialData(Material m, const GpuMaterial &data, std::string *s = nullptr);

		GpuMultipleGraphicModel *getObjectData(Object o);

	#pragma endregion

	//todo remove pragma in python script
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

		//todo probably move into separate struct or sthing
		Shader showNormalsShader;
		GLint normalsModelTransformLocation;
		GLint normalsProjectionLocation;
		GLint normalsSizeLocation;
		GLint normalColorLocation;

	};


	void renderLightModel(GraphicModel &model, Camera  camera, glm::vec3 lightPos, LightShader lightShader,
		GpuTexture texture, GpuTexture normalTexture, GLuint skyBoxTexture, float gama,
		const GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights);

	void renderLightModel(MultipleGraphicModels &model, Camera camera, glm::vec3 lightPos, LightShader lightShader,
		GLuint skyBoxTexture, float gama, std::vector<internal::GpuPointLight> &pointLights);
	
};