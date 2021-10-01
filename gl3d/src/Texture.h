#pragma once
#include <GL/glew.h>
#include <glm/vec2.hpp>

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
		void loadTextureFromMemory(void *data, int w, int h, int chanels = 4, int quality = maxQuality);

		//one if there is alpha data
		int loadTextureFromFileAndCheckAlpha(const char* file, int quality = maxQuality, int channels = 4);

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
			unsigned int flags = 0; //
		};
	};

	void gausianBlurRGB(unsigned char *data, int w, int h, int kernel);


};