#pragma once
#include <GL/glew.h>

namespace gl3d
{

	struct Texture
	{
		GLuint id = 0;

		Texture() = default;
		Texture(const char *file) { loadTextureFromFile(file); };

		void loadTextureFromFile(const char *file);
		void loadTextureFromMemory(void *data, int w, int h);


	};

};