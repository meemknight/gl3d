#pragma once

#include <Core.h>
#include <Texture.h>
#include <Shader.h>
#include <Camera.h>
#include <GraphicModel.h>

namespace gl3d
{
	void renderLightModel(GraphicModel &model, Camera  camera, glm::vec3 lightPos, LightShader lightShader,
		Texture texture, Texture normalTexture, GLuint skyBoxTexture, float gama,
		const Material &material);

	

};