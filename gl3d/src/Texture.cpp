#include "Texture.h"
#include <stb_image.h>
#include <iostream>
#include <glm\vec3.hpp>
#include "Core.h"

namespace gl3d
{

	void Texture::loadTextureFromFile(const char *file, int quality)
	{

		int w, h, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load(file, &w, &h, &nrChannels, 4);

		if (!data)
		{
			//todo err messages
			std::cout << "err: " << file << "\n";
			id = 0;
		}
		else
		{
			loadTextureFromMemory(data, w, h, 4, quality);
			stbi_image_free(data);
		}


	}

	//todo add srgb
	//todo add quality enum
	


	void Texture::loadTextureFromMemory(void *data, int w, int h, int chanels,
		int quality)
	{
		GLenum format = GL_RGBA;

		if(chanels == 3)
		{
			format = GL_RGB;
		}
		gl3dAssertComment(chanels == 3 || chanels == 4, "invalid chanel number");

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		setTextureQuality(quality);
		glGenerateMipmap(GL_TEXTURE_2D);

	}

	void Texture::clear()
	{
		glDeleteTextures(1, &id);
		id = 0;
	}

	void Texture::setTextureQuality(int quality)
	{
		if (!id)
			return;

		glBindTexture(GL_TEXTURE_2D, id);

		switch (quality)
		{
			case leastPossible:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 0);
			}
			break;
			case nearestMipmap:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 0);
			}
			break;
			case linearMipmap:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 4);
			}
			break;
			case maxQuality:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 4);
			}
			break;
			default:
			gl3dAssertComment(0, "invalid quality");
			break;
		}
	}

	int Texture::getTextureQuality()
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

	void gausianBlurRGB(unsigned char *data, int w, int h, int kernel)
	{
		unsigned char *newImage = new unsigned char[w * h * 3];


		//todo refactor refactor refactor
		//todo actually compute this on the gpu

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