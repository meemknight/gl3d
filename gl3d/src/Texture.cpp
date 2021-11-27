#include "Texture.h"
#include <stb_image.h>
#include <iostream>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include "Core.h"
#include <algorithm>

namespace gl3d
{

	void GpuTexture::loadTextureFromFile(const char *file, int quality, int channels)
	{
	
		int w, h, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load(file, &w, &h, &nrChannels, channels);
	
		if (!data)
		{
			//todo err messages
			std::cout << "err: " << file << "\n";
			id = 0;
		}
		else
		{
			loadTextureFromMemory(data, w, h, channels, quality);
			stbi_image_free(data);
		}
	
	
	}
	
	void GpuTexture::loadTextureFromMemory(void *data, int w, int h, int chanels,
		int quality)
	{
	
		gl3dAssertComment(chanels == 1 || chanels == 3 || chanels == 4, "invalid chanel number");
	
		GLenum format = GL_RGBA;
		GLenum internalFormat = GL_RGBA8;
	
		if(chanels == 3)
		{
			format = GL_RGB;
			internalFormat = GL_RGB8;
	
		}else if(chanels == 1)
		{
			format = GL_RED;
			internalFormat = GL_R8;
		}
	
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
	
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		if (quality < 0)
			return;
	
		setTextureQuality(quality);
		glGenerateMipmap(GL_TEXTURE_2D);
	
	}

	void GpuTexture::loadTextureFromMemoryAndCheckAlpha(void* data, int w, int h, int& alpha, int& alphaWithData,
		int chanels, int quality)
	{
		alpha = 0;
		alphaWithData = 0;

		if (chanels == 4)
		{
			for (int i = 0; i < w * h; i++)
			{
				if (((char*)data)[4 * i + 3] != UCHAR_MAX)
				{
					alpha = 1;
				}

				if (((char*)data)[4 * i + 3] != 0 && ((char*)data)[4 * i + 3] != UCHAR_MAX)
				{
					alphaWithData = 1;
				}

				if (alpha && alphaWithData)
				{
					break;
				}
			}

			if (!alpha)
			{
				//cut the last channel
				int writePos = 0;
				int readPos = 0;
				for (int i = 0; i < w * h; i++)
				{
					((char*)data)[writePos++] = ((char*)data)[readPos++];
					((char*)data)[writePos++] = ((char*)data)[readPos++];
					((char*)data)[writePos++] = ((char*)data)[readPos++];
					readPos++;//skip alpha
				}
				chanels = 3;
			}
		}

		loadTextureFromMemory(data, w, h, chanels, quality);

	}
	

	void GpuTexture::loadTextureFromFileAndCheckAlpha(const char* file, int& alpha, int& alphaData, int quality, int channels)
	{
		int w, h, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(file, &w, &h, &nrChannels, channels);

		alpha = 0;
		alphaData = 0;

		if (!data)
		{
			//todo err messages
			std::cout << "err: " << file << "\n";
			id = 0;
		}
		else
		{
			//first look if there is alpha data in the file or if it is wanted at all
			if(nrChannels != 4 || channels != 4) 
			{
				alpha = 0;
			}
			else
			{
				for (int i = 0; i < w * h; i++)
				{
					if (((char*)data)[4 * i + 3] != UCHAR_MAX)
					{
						alpha = 1;
					}

					if (((char*)data)[4 * i + 3] != 0 && ((char*)data)[4 * i + 3] != UCHAR_MAX)
					{
						alphaData = 1;
					}

					if (alpha && alphaData)
					{
						break;
					}
				}
			}

			// if there is no alpha channel in file clamp channels to max 3
			if (!alpha && channels == 4)
			{
				int writePos = 0;
				int readPos = 0;
				for (int i = 0; i < w * h; i++)
				{
					data[writePos++] = data[readPos++];
					data[writePos++] = data[readPos++];
					data[writePos++] = data[readPos++];
					readPos++;//skip alpha
				}

				channels = 3;

			}

			loadTextureFromMemory(data, w, h, channels, quality);
			stbi_image_free(data);
		}

	}

	void GpuTexture::clear()
	{
		glDeleteTextures(1, &id);
		id = 0;
	}

	void GpuTexture::setTextureQuality(int quality)
	{
		if (!id)
			return;
		if (quality < 0)
			return;

		glBindTexture(GL_TEXTURE_2D, id);

		switch (quality)
		{
			case leastPossible:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 1.f);
			}
			break;
			case nearestMipmap:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 1.f);
			}
			break;
			case linearMipmap:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 4.f);
			}
			break;
			case maxQuality:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.f);
			}
			break;
			default:
			gl3dAssertComment(0, "invalid quality");
			break;
		}
	}

	int GpuTexture::getTextureQuality()
	{
		if(id == leastPossible)
		{
			return 0;
		}

		glBindTexture(GL_TEXTURE_2D, id);

		int param = 0;

		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &param);

		switch (param)
		{
			case GL_NEAREST:
			{
				return leastPossible;
			}
			break;
			case GL_NEAREST_MIPMAP_NEAREST:
			{
				return nearestMipmap;
			}
			break;
			case GL_LINEAR_MIPMAP_NEAREST:
			{
				return linearMipmap;
			}
			break;
			case GL_LINEAR_MIPMAP_LINEAR:
			{
				return maxQuality;
			}
			break;
			
		}

		return leastPossible;

	}

	glm::ivec2 GpuTexture::getTextureSize()
	{
		if (!id)
		{
			return glm::ivec2();
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, id);
			int w=0, h=0;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
			return { w, h };
		}


	}

	void gausianBlurRGB(unsigned char *data, int w, int h, int kernel)
	{
		unsigned char *newImage = new unsigned char[w * h * 3];


		//actually compute this on the gpu if really needed

		auto horiz = [&](int kernel)
		{
		//horizontal blur
			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					glm::tvec3<int> colors = {};

					int beg = std::max(0, x - kernel);
					int end = std::min(x + kernel + 1, w);

					for (int i = beg; i < end; i++)
					{
						colors.r += data[(i + y * w) * 3 + 0];
						colors.g += data[(i + y * w) * 3 + 1];
						colors.b += data[(i + y * w) * 3 + 2];
					}

					if (x - kernel < 0)
						for (int i = kernel - x - 1; i >= 0; i--)
						{
							colors.r += data[(i + y * w) * 3 + 0];
							colors.g += data[(i + y * w) * 3 + 1];
							colors.b += data[(i + y * w) * 3 + 2];
						}

					if (x + kernel >= w)
						for (int i = w - 1; i >= w - (x + kernel - w + 1); i--)
						{
							colors.r += data[(i + y * w) * 3 + 0];
							colors.g += data[(i + y * w) * 3 + 1];
							colors.b += data[(i + y * w) * 3 + 2];
						}

					colors /= kernel * 2 + 1;


					//colors /= end - beg;

					newImage[(x + y * w) * 3 + 0] = colors.r;
					newImage[(x + y * w) * 3 + 1] = colors.g;
					newImage[(x + y * w) * 3 + 2] = colors.b;

				}

			}
		};

		auto vert = [&](int kernel)
		{
			//vertical blur
			for (int x = 0; x < w; x++)
			{
				for (int y = 0; y < h; y++)
				{
					glm::tvec3<int> colors = {};

					int beg = std::max(0, y - kernel);
					int end = std::min(y + kernel + 1, h);

					for (int j = beg; j < end; j++)
					{
						colors.r += data[(x + j * w) * 3 + 0];
						colors.g += data[(x + j * w) * 3 + 1];
						colors.b += data[(x + j * w) * 3 + 2];
					}

					if (y - kernel < 0)
						for (int j = kernel - y - 1; j >= 0; j--)
						{
							colors.r += data[(x + j * w) * 3 + 0];
							colors.g += data[(x + j * w) * 3 + 1];
							colors.b += data[(x + j * w) * 3 + 2];
						}

					if (y + kernel >= h)
						for (int j = h - 1; j >= h - (y + kernel - h + 1); j--)
						{
							colors.r += data[(x + j * w) * 3 + 0];
							colors.g += data[(x + j * w) * 3 + 1];
							colors.b += data[(x + j * w) * 3 + 2];
						}

					colors /= kernel * 2 + 1;

					//colors /= end - beg;

					newImage[(x + y * w) * 3 + 0] = colors.r;
					newImage[(x + y * w) * 3 + 1] = colors.g;
					newImage[(x + y * w) * 3 + 2] = colors.b;

				}

			}

		};

		int iterations = 2;

		for(int i=0;i<iterations;i++)
		{
			horiz(kernel);
			vert(kernel);
		}
		
		for (int i = 0; i < w * h * 3; i++)
		{
			data[i] = newImage[i];
		}

		delete newImage;
	}

	

};