#pragma once
#include <GL/glew.h>

namespace gl3d
{

	enum TextureLoadQuality
	{
		leastPossible = 0,
		nearestMipmap,
		linearMipmap,
		maxQuality
	};

	struct Texture
	{
		GLuint id = 0;

		Texture() = default;
		Texture(const char *file) { loadTextureFromFile(file); };

		void loadTextureFromFile(const char *file, int quality = maxQuality);
		void loadTextureFromMemory(void *data, int w, int h, int chanels = 4, int quality = maxQuality);

		void clear();

		void setTextureQuality(int quality);
		int getTextureQuality();

	};


	void gausianBlurRGB(unsigned char *data, int w, int h, int kernel);


};