#pragma once
#include <GL/glew.h>
#include <glm/vec2.hpp>
#include "Core.h"

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
		//GpuTexture(const char *file) { loadTextureFromFile(file); };

		void loadTextureFromFile(const char *file, int quality = maxQuality, int channels = 4);
		void loadTextureFromMemory(void* data, int w, int h, int chanels = 4, int quality = maxQuality);
		void loadTextureFromMemoryAndCheckAlpha
			(void *data, int w, int h, int &alpha, int &alphaWithData, int chanels = 4, int quality = maxQuality);

		//one if there is alpha data
		void loadTextureFromFileAndCheckAlpha(const char* file, int& alpha, int& alphaData,
			int quality = maxQuality, int channels = 4);

		void clear();

		void setTextureQuality(int quality);
		int getTextureQuality();
		glm::ivec2 getTextureSize();

	};

	namespace internal
	{
		struct GpuTextureWithFlags
		{
			GpuTextureWithFlags() = default;
			GpuTexture texture;

			unsigned int flags = {};

			GL3D_ADD_FLAG(alphaExists, setAlphaExists, 0);	//has alpha
			GL3D_ADD_FLAG(alphaWithData, setAlphaWithData, 1);		//will contribute to transparency else just discard fragments with alpha 0

		};
	};

	void gausianBlurRGB(unsigned char *data, int w, int h, int kernel);


};