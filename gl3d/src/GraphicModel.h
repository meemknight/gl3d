#pragma once
#include "GL/glew.h"
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace gl3d
{

	//todo this will dissapear and become an struct of arrays or sthing
	struct GraphicModel
	{

		//todo probably this will disapear
		GLuint vertexArray = 0;

		GLuint vertexBuffer = 0;
		GLuint indexBuffer = 0;

		GLsizei primitiveCount = 0;

		//todo check if indexes can be uint
		void loadFromData(size_t vertexSize,
			float *vercies, size_t indexSize = 0, unsigned int *indexes = nullptr);

		void clear();

		void draw();

		//todo probably move this in the final version
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = {1,1,1};
		
		glm::mat4 getTransformMatrix();
	};

};