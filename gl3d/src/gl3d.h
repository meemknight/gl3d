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

		std::vector< internal::GpuMaterial > materials;
		std::vector<int> materialIndexes;
		
		Material createMaterial(glm::vec3 kd = glm::vec3(1), 
			float roughness = 0.5f, float metallic = 0.1, float ao = 1);
		
		void deleteMaterial(Material m);
	
	#pragma endregion


		std::vector< GpuMultipleGraphicModel > graphicModels;
		std::vector<int> graphicModelsIndexes;

		Object loadObject(std::string path);
		void deleteObject(Object o);

		LightShader lightShader;
		Camera camera;
		

		void renderObject(Object o, glm::vec3 position, glm::vec3 rotation = {}, glm::vec3 scale = {1,1,1});

	};


	void renderLightModel(GraphicModel &model, Camera  camera, glm::vec3 lightPos, LightShader lightShader,
		Texture texture, Texture normalTexture, GLuint skyBoxTexture, float gama,
		const internal::GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights);

	void renderLightModel(MultipleGraphicModels &model, Camera camera, glm::vec3 lightPos, LightShader lightShader,
		GLuint skyBoxTexture, float gama, std::vector<internal::GpuPointLight> &pointLights);
	
};