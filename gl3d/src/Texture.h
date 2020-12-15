#pragma once
#include <GL/glew.h>

namespace gl3d
{

	struct Texture
	{
		GLuint id = 0;

		void loadTextureFromFile(const char *file);
		void loadTextureFromMemory(void *data, int w, int h);


	};

};