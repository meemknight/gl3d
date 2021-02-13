#include "gl3d.h"
namespace gl3d
{
	void renderLightModel(GraphicModel &model, Camera camera, glm::vec3 lightPos, LightShader lightShader, Texture texture, Texture normalTexture, GLuint skyBoxTexture, 
		float gama, const Material &material, std::vector<PointLight> &pointLights)
	{

		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = model.getTransformMatrix();

		auto viewProjMat = projMat * viewMat * transformMat;

		lightShader.bind(viewProjMat, transformMat, lightPos, camera.position, gama, material,
			pointLights);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.id);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture.id);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

		model.draw();


	}


	void renderLightModel(MultipleGraphicModels &model, Camera camera, glm::vec3 lightPos, 
		LightShader lightShader, GLuint skyBoxTexture, float gama, std::vector<PointLight> &pointLights)
	{
		if(model.models.empty())
		{
			return;
		}

		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = model.getTransformMatrix();

		auto modelViewProjMat = projMat * viewMat * transformMat;
		//auto modelView = viewMat * transformMat;

		lightShader.shader.bind();

		lightShader.getSubroutines();
		lightShader.setData(modelViewProjMat, transformMat, lightPos, camera.position, gama, Material(),
			pointLights);

		GLsizei n;
		//glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &n);
		glGetProgramStageiv(lightShader.shader.id,
		GL_FRAGMENT_SHADER,
		GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
		&n);

		GLuint *indices = new GLuint[n]{ 0 };
		bool changed = 1;

		for(auto &i : model.models)
		{
			lightShader.setMaterial(i.material);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, i.albedoTexture.id);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, i.normalMapTexture.id);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, i.RMA_Texture.id);


			if (i.normalMapTexture.id && lightShader.normalMap)
			{
				if(indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_normalMap)
				{
					changed = 1;
				}
				indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_normalMap;
			}else
			{
				if (indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_normalMap)
				{
					changed = 1;
				}
				indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_noMap;
			}

			if(indices[lightShader.materialSubroutineLocation] != lightShader.materialSubroutine_functions[i.RMA_loadedTextures])
			{ 
				changed = 1;
			}

			indices[lightShader.materialSubroutineLocation] = lightShader.materialSubroutine_functions[i.RMA_loadedTextures];

			if(changed)
			{
				glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, n, indices);
			}
			changed = 0;

			i.draw();


		}

		delete[] indices;

		
	}

};