#pragma once
#include <GL/glew.h>

namespace gl3d
{

	enum TextureLoadQuality
	{
		dontSet = -1, //won't create mipmap
		leastPossible = 0,
		nearestMipmap,
		linearMipmap,
		maxQuality
	};

	struct GpuTexture
	{
		GLuint id = 0;

		GpuTexture() = default;
		GpuTexture(const char *file) { loadTextureFromFile(file); };

		void loadTextureFromFile(const char *file, int quality = maxQuality, int channels = 4);
		void loadTextureFromMemory(void *data, int w, int h, int chanels = 4, int quality = maxQuality);

		void clear();

		void setTextureQuality(int quality);
		int getTextureQuality();

	};


	void gausianBlurRGB(unsigned char *data, int w, int h, int kernel);


};