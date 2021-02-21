#pragma once

#include <Core.h>
#include <Texture.h>
#include <Shader.h>
#include <Camera.h>
#include <GraphicModel.h>
#include <list>

namespace gl3d
{

	

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
		void copyMaterial(Material dest, Material source);

		GpuMaterial *getMaterialData(Material m, std::string *s = nullptr);
		bool setMaterialData(Material m, const GpuMaterial &data, std::string *s = nullptr);

		GpuMultipleGraphicModel *getObjectData(Object o);

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

		//internal
		int getMaterialIndex(Material m);
		int getObjectIndex(Object o);


	};


	void renderLightModel(GraphicModel &model, Camera  camera, glm::vec3 lightPos, LightShader lightShader,
		Texture texture, Texture normalTexture, GLuint skyBoxTexture, float gama,
		const GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights);

	void renderLightModel(MultipleGraphicModels &model, Camera camera, glm::vec3 lightPos, LightShader lightShader,
		GLuint skyBoxTexture, float gama, std::vector<internal::GpuPointLight> &pointLights);
	
};