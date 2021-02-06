#include "gl3d.h"
namespace gl3d
{
	void renderLightModel(GraphicModel &model, Camera camera, glm::vec3 lightPos, LightShader lightShader, Texture texture, Texture normalTexture, GLuint skyBoxTexture, float gama, const Material &material)
	{

		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = model.getTransformMatrix();

		auto viewProjMat = projMat * viewMat * transformMat;

		lightShader.bind(viewProjMat, transformMat, lightPos, camera.position, gama, material);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.id);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture.id);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

		model.draw();


	}


	void renderLightModel(MultipleGraphicModels &model, Camera camera, glm::vec3 lightPos, 
		LightShader lightShader, GLuint skyBoxTexture, float gama)
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

		for(auto &i : model.models)
		{
		
			lightShader.bind(modelViewProjMat, transformMat, lightPos, camera.position, gama, i.material);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, i.albedoTexture.id);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, i.normalMapTexture.id);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, i.roughnessMapTexture.id);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, i.ambientMapTexture.id);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, i.metallicMapTexture.id);

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, i.RMA_Texture.id);

			lightShader.getSubroutines();

			GLsizei n;
			//glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &n);
			glGetProgramStageiv(lightShader.shader.id,
			GL_FRAGMENT_SHADER,
			GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
			&n);


			GLuint *indices = new GLuint[n];

			if (i.normalMapTexture.id && lightShader.normalMap)
			{
				indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_normalMap;
			}else
			{
				indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_noMap;
			}

			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, n, indices);

			i.draw();

			delete[] indices;

		}

		
	}

};