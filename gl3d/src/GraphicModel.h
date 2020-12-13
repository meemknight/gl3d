#pragma once
#include "GL/glew.h"

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

		void loadFromData(size_t vertexSize,
			float *vercies, size_t indexSize = 0, unsigned int *indexes = nullptr);

		void clear();

		void draw();

	};

};