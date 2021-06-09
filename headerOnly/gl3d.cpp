////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on 2021-06-09
////////////////////////////////////////////////

#include "gl3d.h"

////////////////////////////////////////////////
//Core.cpp
////////////////////////////////////////////////
#pragma region Core

#include <stdio.h>
#include <Windows.h>
#include <signal.h>
#include <iostream>

namespace gl3d 
{

	void assertFunc(const char *expression,
		const char *file_name,
		unsigned const line_number,
		const char *comment)
	{
		
		char c[1024] = {};
	
		sprintf(c,
			"Assertion failed\n\n"
			"File:\n"
			"%s\n\n"
			"Line:\n"
			"%u\n\n"
			"Expresion:\n"
			"%s\n\n"
			"Comment:\n"
			"%s",
			file_name,
			line_number,
			expression,
			comment
		);
	
		int const action = MessageBox(0, c, "Platform Layer", MB_TASKMODAL
			| MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);
	
		switch (action)
		{
			case IDABORT: // Abort the program:
			{
				raise(SIGABRT);
	
				// We won't usually get here, but it's possible that a user-registered
				// abort handler returns, so exit the program immediately.  Note that
				// even though we are "aborting," we do not call abort() because we do
				// not want to invoke Watson (the user has already had an opportunity
				// to debug the error and chose not to).
				_exit(3);
			}
			case IDRETRY: // Break into the debugger then return control to caller
			{
				__debugbreak();
				return;
			}
			case IDIGNORE: // Return control to caller
			{
				return;
			}
			default: // This should not happen; treat as fatal error:
			{
				abort();
			}
		}
		
	
	}

	//https://learnopengl.com/In-Practice/Debugging
	//todo probably remove iostream
	void GLAPIENTRY glDebugOutput(GLenum source,
								GLenum type,
								unsigned int id,
								GLenum severity,
								GLsizei length,
								const char *message,
								const void *userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
		if (type == GL_DEBUG_TYPE_PERFORMANCE) return;

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " << message << std::endl;
	
		switch (source)
		{
			case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;
	
		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
			case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;
	
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;

	}


};

#pragma endregion


////////////////////////////////////////////////
//Texture.cpp
////////////////////////////////////////////////
#pragma region Texture

#include <stb_image.h>
#include <iostream>
#include <glm\vec3.hpp>

#include <algorithm>

namespace gl3d
{

	void GpuTexture::loadTextureFromFile(const char *file, int quality)
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



	void GpuTexture::loadTextureFromMemory(void *data, int w, int h, int chanels,
		int quality)
	{

		gl3dAssertComment(chanels == 3 || chanels == 4, "invalid chanel number");

		GLenum format = GL_RGBA;

		if(chanels == 3)
		{
			format = GL_RGB;
		}

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		setTextureQuality(quality);
		glGenerateMipmap(GL_TEXTURE_2D);

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
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.f);
			}
			break;
			case maxQuality:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.f);
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
#pragma endregion


////////////////////////////////////////////////
//Shader.cpp
////////////////////////////////////////////////
#pragma region Shader

#include <fstream>
#include <iostream>

namespace gl3d
{

	GLint createShaderFromFile(const char *source, GLenum shaderType)
	{
		std::ifstream file;
		file.open(source);

		if (!file.is_open())
		{
			std::cout << "Error openning file: " << source << "\n";
			return 0;
		}

		GLint size = 0;
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);

		char *fileContent = new char[size] {};

		file.read(fileContent, size);


		file.close();

		GLuint shaderId = glCreateShader(shaderType);
		glShaderSource(shaderId, 1, &fileContent, &size);
		glCompileShader(shaderId);

		delete[] fileContent;

		GLint rezult = 0;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &rezult);

		if (!rezult)
		{
			char *message = 0;
			int   l = 0;

			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &l);

			if(l)
			{
				message = new char[l];

				glGetShaderInfoLog(shaderId, l, &l, message);

				message[l - 1] = 0;

				std::cout << source << ": " << message << "\n";

				delete[] message;
				
			}else
			{
				std::cout << source << ": " << "unknown error"<< "\n";
			}

			glDeleteShader(shaderId);

			shaderId = 0;
			return shaderId;
		}

		return shaderId;
	}

	bool Shader::loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader)
	{

		auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
		auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);


		if (vertexId == 0 || fragmentId == 0)
		{
			return 0;
		}

		id = glCreateProgram();

		glAttachShader(id, vertexId);
		glAttachShader(id, fragmentId);

		glLinkProgram(id);

		glDeleteShader(vertexId);
		glDeleteShader(fragmentId);

		GLint info = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &info);

		if (info != GL_TRUE)
		{
			char *message = 0;
			int   l = 0;

			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

			message = new char[l];

			glGetProgramInfoLog(id, l, &l, message);

			std::cout << "Link error: " << message << "\n";

			delete[] message;

			glDeleteProgram(id);
			id = 0;
			return 0;
		}

		glValidateProgram(id);

		return true;
	}

	bool Shader::loadShaderProgramFromFile(const char *vertexShader, const char *geometryShader, const char *fragmentShader)
	{

		auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
		auto geometryId = createShaderFromFile(geometryShader, GL_GEOMETRY_SHADER);
		auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);

		if (vertexId == 0 || fragmentId == 0 || geometryId == 0)
		{
			return 0;
		}

		id = glCreateProgram();

		glAttachShader(id, vertexId);
		glAttachShader(id, geometryId);
		glAttachShader(id, fragmentId);

		glLinkProgram(id);

		glDeleteShader(vertexId);
		glDeleteShader(geometryId);
		glDeleteShader(fragmentId);

		GLint info = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &info);

		if (info != GL_TRUE)
		{
			char *message = 0;
			int   l = 0;

			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

			message = new char[l];

			glGetProgramInfoLog(id, l, &l, message);

			std::cout << "Link error: " << message << "\n";

			delete[] message;

			glDeleteProgram(id);
			id = 0;
			return 0;
		}

		glValidateProgram(id);

		return true;
	}

	void Shader::bind()
	{
		glUseProgram(id);
	}

	void Shader::clear()
	{
		glDeleteProgram(id);
		id = 0;
	}

	GLint getUniformSubroutine(GLuint id, GLenum shaderType, const char *name)
	{
		GLint uniform = glGetSubroutineUniformLocation(id, shaderType, name);
		if (uniform == -1)
		{
			std::cout << "uniform subroutine error " << name << "\n";
		}
		return uniform;
	};

	GLint getUniform(GLuint id, const char *name)
	{
		GLint uniform = glGetUniformLocation(id, name);
		if (uniform == -1)
		{
			std::cout << "uniform error " << name << "\n";
		}
		return uniform;
	};

	GLuint getUniformBlock(GLuint id, const char *name)
	{
		GLuint uniform = glGetUniformBlockIndex(id, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "uniform block error " << name << "\n";
		}
		return uniform;
	};

	GLuint getUniformSubroutineIndex(GLuint id, GLenum shaderType, const char *name)
	{
		GLuint uniform = glGetSubroutineIndex(id, shaderType, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "uniform subroutine index error " << name << "\n";
		}
		return uniform;
	};

	GLuint getStorageBlockIndex(GLuint id, const char *name)
	{
		GLuint uniform = glGetProgramResourceIndex(id, GL_SHADER_STORAGE_BLOCK, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "storage block index error " << name << "\n";
		}
		return uniform;
	};



	void LightShader::create()
	{
	#pragma region brdf texture
		brdfTexture.loadTextureFromFile("resources/BRDFintegrationMap.png", TextureLoadQuality::leastPossible);
		glBindTexture(GL_TEXTURE_2D, brdfTexture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	#pragma endregion

		geometryPassShader.loadShaderProgramFromFile("shaders/deferred/geometryPass.vert", "shaders/deferred/geometryPass.frag");
		geometryPassShader.bind();

		u_transform = getUniform(geometryPassShader.id, "u_transform");
		u_modelTransform = getUniform(geometryPassShader.id, "u_modelTransform");
		u_motelViewTransform = getUniform(geometryPassShader.id, "u_motelViewTransform");
		//normalShaderLightposLocation = getUniform(shader.id, "u_lightPosition");
		textureSamplerLocation = getUniform(geometryPassShader.id, "u_albedoSampler");
		normalMapSamplerLocation = getUniform(geometryPassShader.id, "u_normalSampler");
		//eyePositionLocation = getUniform(shader.id, "u_eyePosition");
		//skyBoxSamplerLocation = getUniform(textureSamplerLocation.id, "u_skybox");
		//gamaLocation = getUniform(shader.id, "u_gama");
		RMASamplerLocation = getUniform(geometryPassShader.id, "u_RMASampler");
		//pointLightCountLocation = getUniform(shader.id, "u_pointLightCount");
		materialIndexLocation = getUniform(geometryPassShader.id, "u_materialIndex");
		//pointLightBufferLocation = getUniform(shader.id, "u_pointLights");
		
		materialBlockLocation = getStorageBlockIndex(geometryPassShader.id, "u_material");
		glShaderStorageBlockBinding(geometryPassShader.id, materialBlockLocation, 0);
		glGenBuffers(1, &materialBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);


		lightingPassShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/deferred/lightingPass.frag");
		lightingPassShader.bind();

		light_u_albedo = getUniform(lightingPassShader.id, "u_albedo");
		light_u_normals = getUniform(lightingPassShader.id, "u_normals");
		light_u_skyboxFiltered = getUniform(lightingPassShader.id, "u_skyboxFiltered");
		light_u_positions = getUniform(lightingPassShader.id, "u_positions");
		light_u_materials = getUniform(lightingPassShader.id, "u_materials");
		light_u_eyePosition = getUniform(lightingPassShader.id, "u_eyePosition");
		light_u_pointLightCount = getUniform(lightingPassShader.id, "u_pointLightCount");
		light_u_ssao = getUniform(lightingPassShader.id, "u_ssao");
		light_u_view = getUniform(lightingPassShader.id, "u_view");
		light_u_skyboxIradiance = getUniform(lightingPassShader.id, "u_skyboxIradiance");
		u_useSSAO = getUniform(lightingPassShader.id, "u_useSSAO");
		light_u_brdfTexture = getUniform(lightingPassShader.id, "u_brdfTexture");
		

	#pragma region uniform buffer

		lightPassShaderData.u_lightPassData = glGetUniformBlockIndex(lightingPassShader.id, "u_lightPassData");
		glGenBuffers(1, &lightPassShaderData.lightPassDataBlockBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, lightPassShaderData.lightPassDataBlockBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(LightPassData), &lightPassUniformBlockCpuData, GL_DYNAMIC_DRAW);
		
		glUniformBlockBinding(lightingPassShader.id, lightPassShaderData.u_lightPassData, 1);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightPassShaderData.lightPassDataBlockBuffer);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	#pragma endregion


		pointLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_pointLights");
		glShaderStorageBlockBinding(lightingPassShader.id, pointLightsBlockLocation, 1);
		glGenBuffers(1, &pointLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsBlockBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightsBlockBuffer);


		glGenVertexArrays(1, &quadDrawer.quadVAO);
		glBindVertexArray(quadDrawer.quadVAO);

		glGenBuffers(1, &quadDrawer.quadBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quadDrawer.quadBuffer);

		float quadVertices[] = {
		   // positions        // texture Coords
		   -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		   -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

		glBindVertexArray(0);

	}

	void LightShader::bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition, float gama,
		const GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights)
	{
		geometryPassShader.bind();
		
		this->setData(viewProjMat, transformMat, lightPosition, eyePosition, gama, 
			material, pointLights);
	
	}

	void LightShader::setData(const glm::mat4 &viewProjMat, 
		const glm::mat4 &transformMat, const glm::vec3 &lightPosition, const glm::vec3 &eyePosition,
		float gama, const GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights)
	{
		glUniformMatrix4fv(u_transform, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniformMatrix4fv(u_modelTransform, 1, GL_FALSE, &transformMat[0][0]);
		glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(textureSamplerLocation, 0);
		glUniform1i(normalMapSamplerLocation, 1);
		glUniform1i(skyBoxSamplerLocation, 2);
		glUniform1i(RMASamplerLocation, 3);

		if(pointLights.size())
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsBlockBuffer);

			glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(internal::GpuPointLight)
				,&pointLights[0], GL_STREAM_DRAW); 

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightsBlockBuffer);

		}

		//glUniform1fv(pointLightBufferLocation, pointLights.size() * 8, (float*)pointLights.data());

		glUniform1i(pointLightCountLocation, pointLights.size());

		setMaterial(material);
	}

	void LightShader::setMaterial(const GpuMaterial &material)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(material)
			, &material, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, materialBlockBuffer);
		glUniform1i(materialIndexLocation, 0);
	}



	void LightShader::getSubroutines()
	{


		normalSubroutine_noMap = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"noNormalMapped");

		normalSubroutine_normalMap = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"normalMapped");

		
		//
		albedoSubroutine_sampled = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"sampledAlbedo");

		albedoSubroutine_notSampled = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"notSampledAlbedo");


		//	
		normalSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"getNormalMapFunc");

		materialSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"u_getMaterialMapped");

		getAlbedoSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"u_getAlbedo");

		const char *materiaSubroutineFunctions[8] = { 
			"materialNone",
			"materialR",
			"materialM",
			"materialA",
			"materialRM",
			"materialRA",
			"materialMA",
			"materialRMA" };

		for(int i=0; i<8; i++)
		{
			materialSubroutine_functions[i] = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				materiaSubroutineFunctions[i]);
		}


	}

};

#pragma endregion


////////////////////////////////////////////////
//Camera.cpp
////////////////////////////////////////////////
#pragma region Camera



#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>


namespace gl3d
{

	glm::mat4x4 Camera::getProjectionMatrix()
	{
		auto mat = glm::perspective(this->fovRadians, this->aspectRatio, this->closePlane,
		this->farPlane);

		return mat;
	}

	glm::mat4x4 Camera::getWorldToViewMatrix()
	{
		glm::vec3 lookingAt = this->position;
		lookingAt += viewDirection;


		auto mat = glm::lookAt(this->position, lookingAt, this->up);
		return mat;
	}

	void Camera::rotateCamera(const glm::vec2 delta)
	{

		glm::vec3 rotateYaxe = glm::cross(viewDirection, up);

		viewDirection = glm::mat3(glm::rotate(delta.x, up)) * viewDirection;

		if (delta.y < 0)
		{	//down
			if (viewDirection.y < -0.99)
				goto noMove;
		}
		else
		{	//up
			if (viewDirection.y > 0.99)
				goto noMove;
		}

		viewDirection = glm::mat3(glm::rotate(delta.y, rotateYaxe)) * viewDirection;
	noMove:

		viewDirection = glm::normalize(viewDirection);
	}

	void Camera::moveFPS(glm::vec3 direction)
	{
		viewDirection = glm::normalize(viewDirection);

		//forward
		float forward = -direction.z;
		float leftRight = direction.x;
		float upDown = direction.y;

		glm::vec3 move = {};

		move += up * upDown;
		move += glm::normalize(glm::cross(viewDirection, up)) * leftRight;
		move += viewDirection * forward;

		this->position += move;
	
	}


	
};
#pragma endregion


////////////////////////////////////////////////
//GraphicModel.cpp
////////////////////////////////////////////////
#pragma region GraphicModel


#include "OBJ_Loader.h"
#include <stb_image.h>

#include <algorithm>

namespace gl3d 
{

	void GraphicModel::loadFromComputedData(size_t vertexSize, const float * vercies, size_t indexSize,
		const unsigned int * indexes, bool noTexture)
	{

		gl3dAssertComment(indexSize % 3 == 0, "Index count must be multiple of 3");
		if (indexSize % 3 != 0)return;


		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexSize, vercies, GL_STATIC_DRAW);

		if(noTexture)
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
		}else
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
		
		}


		if (indexSize && indexes)
		{
			glGenBuffers(1, &indexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexes, GL_STATIC_DRAW);

			primitiveCount = indexSize / sizeof(*indexes);

		}
		else
		{
			primitiveCount = vertexSize / sizeof(float);
		}
	
		glBindVertexArray(0);

	}

	//deprecated
	void GraphicModel::loadFromData(size_t vertexCount, float *vertices, float *normals, float *textureUV, size_t indexesCount, unsigned int *indexes)
	{
		gl3dAssertComment(vertices, "Vertices are not optional");
		gl3dAssertComment(normals, "Normals are not optional"); //todo compute
		if (!vertices || !normals) { return; }

		std::vector<float> dataForModel;

		dataForModel.reserve(vertexCount * 8);
		for (unsigned int i = 0; i < vertexCount; i++)
		{
			//positions normals uv

			dataForModel.push_back(vertices[(8*i)+0]);
			dataForModel.push_back(vertices[(8*i)+1]);
			dataForModel.push_back(vertices[(8*i)+2]);

			dataForModel.push_back(normals[(8 * i) + 3]);
			dataForModel.push_back(normals[(8 * i) + 4]);
			dataForModel.push_back(normals[(8 * i) + 5]);

			if (textureUV)
			{
				dataForModel.push_back(normals[(8 * i) + 6]);
				dataForModel.push_back(normals[(8 * i) + 7]);
			}
			else
			{
				dataForModel.push_back(0.f);
				dataForModel.push_back(0.f);
			}

		}

		this->loadFromComputedData(vertexCount * 4,
		&dataForModel[0],
			indexesCount * 4, &indexes[0], (textureUV == nullptr));
	}

	int max(int x, int y, int z)
	{
		return std::max(std::max(x, y), z);
	}

	void GraphicModel::loadFromModelMeshIndex(const LoadedModelData &model, int index)
	{
		auto &mesh = model.loader.LoadedMeshes[index];
		loadFromComputedData(mesh.Vertices.size() * 8 * 4,
			 (float *)&mesh.Vertices[0],
			mesh.Indices.size() * 4, &mesh.Indices[0]);


		auto &mat = model.loader.LoadedMeshes[index].MeshMaterial;
		material.setDefaultMaterial();

		material.kd = glm::vec4(glm::vec3(mat.Kd), 1);
		//material.ks = glm::vec4(glm::vec3(mat.Ks), mat.Ns);
		//material.ka = glm::vec4(glm::vec3(mat.Ka), 0);
		material.metallic = mat.metallic;
		material.roughness = mat.roughness;
		//material.ao = mat.ao;


		albedoTexture.clear();
		normalMapTexture.clear();
		RMA_Texture.clear();

		if (!mat.map_Kd.empty())
		{
			albedoTexture.loadTextureFromFile(std::string(model.path + mat.map_Kd).c_str());
		}

		if (!mat.map_Kn.empty())
		{
			normalMapTexture.loadTextureFromFile(std::string(model.path + mat.map_Kn).c_str(),
				TextureLoadQuality::linearMipmap);
		}

		RMA_loadedTextures = 0;
		
		auto rmaQuality = TextureLoadQuality::linearMipmap;

		if(!mat.map_RMA.empty()) //todo not tested
		{
			RMA_Texture.loadTextureFromFile(mat.map_RMA.c_str(),
			rmaQuality);

			if(RMA_Texture.id)
			{
				RMA_loadedTextures = 7; //all textures loaded
			}

		}

		if (!mat.map_ORM.empty() && RMA_loadedTextures == 0)
		{
			stbi_set_flip_vertically_on_load(true);

			int w = 0, h = 0;
			unsigned char *data = 0;

			
			{
				data = stbi_load(std::string(model.path + mat.map_ORM).c_str(),
				&w, &h, 0, 4);
				if (!data)
				{ std::cout << "err loading " << std::string(model.path + mat.map_ORM) << "\n"; }
				else
				{
					//convert from ORM ro RMA

					for (int j = 0; j < h; j++)
						for (int i = 0; i < w; i++)
						{
							unsigned char R = data[(i + j*w) * 4 + 1];
							unsigned char M = data[(i + j*w) * 4 + 2];
							unsigned char A = data[(i + j*w) * 4 + 0];

							data[(i + j * w) * 4 + 0] = R;
							data[(i + j * w) * 4 + 1] = M;
							data[(i + j * w) * 4 + 2] = A;
						}

					RMA_Texture.loadTextureFromMemory(data, w, h, 4, rmaQuality);
				
					RMA_loadedTextures = 7; //all textures loaded

					stbi_image_free(data);
				}
			}
			

		}

		//RMA trexture
		if(RMA_loadedTextures == 0)
		{
			stbi_set_flip_vertically_on_load(true);

			int w1=0, h1=0;
			unsigned char *data1 = 0;
			unsigned char *data2 = 0;
			unsigned char *data3 = 0;

			if(!mat.map_Pr.empty())
			{
				data1 = stbi_load(std::string(model.path + mat.map_Pr).c_str(),
				&w1, &h1, 0, 1);
				if (!data1) { std::cout << "err loading " << std::string(model.path + mat.map_Pr) << "\n"; }
			}
			
			int w2=0, h2=0;
			if(!mat.map_Pm.empty())
			{
				data2 = stbi_load(std::string(model.path + mat.map_Pm).c_str(),
				&w2, &h2, 0, 1);
				if (!data2) { std::cout << "err loading " << std::string(model.path + mat.map_Pm) << "\n"; }
			}
		

			int w3=0, h3=0;
			if(!mat.map_Ka.empty())
			{
			data3 = stbi_load(std::string(model.path + mat.map_Ka).c_str(),
				&w3, &h3, 0, 1);
				if (!data3) { std::cout << "err loading " << std::string(model.path + mat.map_Ka) << "\n"; }
			}

			int w = max(w1, w2, w3);
			int h = max(h1, h2, h3);

			//calculate which function to use
			if(data1 && data2 && data3){ RMA_loadedTextures = 7;}else
			if(			data2 && data3){ RMA_loadedTextures = 6;}else
			if(data1 		  && data3){ RMA_loadedTextures = 5;}else
			if(data1 && data2		  ){ RMA_loadedTextures = 4;}else
			if(					 data3){ RMA_loadedTextures = 3;}else
			if(			data2		  ){ RMA_loadedTextures = 2;}else
			if(data1				  ){ RMA_loadedTextures = 1;}else
									   { RMA_loadedTextures = 0;};
			if (RMA_loadedTextures)
			{

				unsigned char *finalData = new unsigned char[w * h * 4];

				//todo mabe add bilinear filtering
				//todo load less chanels if necessary
				for (int j = 0; j < h; j++)
				{
					for (int i = 0; i < w; i++)
					{

						if (data1)	//rough
						{
							int texelI = (i / (float)w) * w1;
							int texelJ = (j / float(h)) * h1;

							finalData[((j * w) + i) * 4 + 0] =
								data1[(texelJ * w1) + texelI];

						}
						else
						{
							finalData[((j * w) + i) * 4 + 0] = 0;
						}

						if (data2)	//metalic
						{

							int texelI = (i / (float)w) * w2;
							int texelJ = (j / float(h)) * h2;

							finalData[((j * w) + i) * 4 + 1] =
								data2[(texelJ * w2) + texelI];
						}
						else
						{
							finalData[((j * w) + i) * 4 + 1] = 0;
						}

						if (data3)	//ambient
						{
							int texelI = (i / (float)w) * w3;
							int texelJ = (j / float(h)) * h3;

							finalData[((j * w) + i) * 4 + 2] =
								data3[(texelJ * w3) + texelI];
						}
						else
						{
							finalData[((j * w) + i) * 4 + 2] = 0;
						}

						finalData[((j * w) + i) * 4 + 3] = 255; //todo used only for imgui, remove later 
					}
				}

				RMA_Texture.loadTextureFromMemory(finalData, w, h, 4,
					rmaQuality);

				stbi_image_free(data1);
				stbi_image_free(data2);
				stbi_image_free(data3);
				delete[] finalData;

			}

		}


	}

	void GraphicModel::loadFromModelMesh(const LoadedModelData &model)
	{
		auto &mesh = model.loader.LoadedVertices;
		loadFromComputedData(mesh.size() * 8 * 4,
			 (float *)&mesh[0],
			model.loader.LoadedIndices.size() * 4,
			&model.loader.LoadedIndices[0]);
	}

	//deprecated
	void GraphicModel::loadFromFile(const char *fileName)
	{
		objl::Loader loader;
		loader.LoadFile(fileName);


		std::vector<float> dataForModel;

		auto &mesh = loader.LoadedMeshes[0];

		dataForModel.reserve(mesh.Vertices.size() * 8);
		for (unsigned int i = 0; i < mesh.Vertices.size(); i++)
		{
			//positions normals uv

			dataForModel.push_back(mesh.Vertices[i].Position.X);
			dataForModel.push_back(mesh.Vertices[i].Position.Y);
			dataForModel.push_back(mesh.Vertices[i].Position.Z);
			
			dataForModel.push_back(mesh.Vertices[i].Normal.X);
			dataForModel.push_back(mesh.Vertices[i].Normal.Y);
			dataForModel.push_back(mesh.Vertices[i].Normal.Z);

			dataForModel.push_back(mesh.Vertices[i].TextureCoordinate.X);
			dataForModel.push_back(mesh.Vertices[i].TextureCoordinate.Y);
		}

		std::vector<unsigned int> indicesForModel;
		indicesForModel.reserve(mesh.Indices.size());

		for (unsigned int i = 0; i < mesh.Indices.size(); i++)
		{
			indicesForModel.push_back(mesh.Indices[i]);
		}

		this->loadFromComputedData(dataForModel.size() * 4,
			&dataForModel[0],
			indicesForModel.size() * 4, &indicesForModel[0]);


		//vb = vertexBuffer(dataForModel.data(), dataForModel.size() * sizeof(float), GL_STATIC_DRAW);
		//ib = indexBuffer(indicesForModel.data(), indicesForModel.size() * sizeof(unsigned int));
		//va = std::move(vertexAttribute{ 3, 2, 3 });
		//
		//
		//if (model.m.LoadedMaterials.size() > 0)
		//{
		//
		//	material.ka = glm::vec3(model.m.LoadedMaterials[0].Ka.X, model.m.LoadedMaterials[0].Ka.Y, model.m.LoadedMaterials[0].Ka.Z);
		//	material.kd = glm::vec3(model.m.LoadedMaterials[0].Kd.X, model.m.LoadedMaterials[0].Kd.Y, model.m.LoadedMaterials[0].Kd.Z);
		//	material.ks = glm::vec3(model.m.LoadedMaterials[0].Ks.X, model.m.LoadedMaterials[0].Ks.Y, model.m.LoadedMaterials[0].Ks.Z);
		//	material.shiny = model.m.LoadedMaterials[0].Ns;
		//	if (material.shiny == 0) { material.shiny = 1; }
		//
		//	if (model.m.LoadedMaterials[0].map_Kd != "")
		//	{
		//
		//		texture = manager->getData(model.m.LoadedMaterials[0].map_Kd.c_str());
		//
		//	}
		//}

	
	}

	void GraphicModel::clear()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);

		glDeleteVertexArrays(1, &vertexArray);

		albedoTexture.clear();
		normalMapTexture.clear();
		RMA_Texture.clear();

		vertexBuffer = 0;
		indexBuffer = 0;
		primitiveCount = 0;
		vertexArray = 0;
	}

	void GraphicModel::draw()
	{
		glBindVertexArray(vertexArray);

		//glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)(3 * sizeof(float)));

		if (indexBuffer)
		{
			glDrawElements(GL_TRIANGLES, primitiveCount, GL_UNSIGNED_INT, 0);

		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, primitiveCount);
		}

		glBindVertexArray(0);

	}

	glm::mat4 GraphicModel::getTransformMatrix()
	{
		return gl3d::getTransformMatrix(position, rotation, scale);

	}


	void LoadedModelData::load(const char *file, float scale)
	{
		loader.LoadFile(file);

		//parse path
		path = file;
		while (!path.empty() &&
			*(path.end() - 1) != '\\' &&
			*(path.end() - 1) != '/'
			)
		{
			path.pop_back();
		}

		for (auto &i : loader.LoadedMeshes)
		{
			for(auto &j : i.Vertices)
			{
				j.Position.X *= scale;
				j.Position.Y *= scale;
				j.Position.Z *= scale;
			}
		}

		for (auto &j : loader.LoadedVertices)
		{
			j.Position.X *= scale;
			j.Position.Y *= scale;
			j.Position.Z *= scale;
		}

		std::cout << "Loaded: " << loader.LoadedMeshes.size() << " meshes\n";
	}

	float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
	};

	
	static glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	static glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
	

	void MultipleGraphicModels::loadFromModel(const LoadedModelData &model)
	{
		clear();

		int s = model.loader.LoadedMeshes.size();
		models.reserve(s);

		for(int i=0;i<s;i++)
		{
			GraphicModel gm;
			gm.loadFromModelMeshIndex(model, i);
			gm.name = model.loader.LoadedMeshes[i].MeshName;

			char *c = new char[gm.name.size() + 1];
			strcpy(c, gm.name.c_str());

			subModelsNames.push_back(c);
			models.push_back(gm);

		}

	}

	void MultipleGraphicModels::clear()
	{
		for(auto &i : models)
		{
			i.clear();
		}

		for (auto &i : subModelsNames)
		{
			delete[] i;
		}

		subModelsNames.clear();
		models.clear();

		//todo clear material buffer ref cound and stuff
	}


	//todo optimize
	glm::mat4 getTransformMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		auto s = glm::scale(scale);
		auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(rotation.z, glm::vec3(0, 0, 1));
		auto t = glm::translate(position);

		return t * r * s;
	}



	void GpuMultipleGraphicModel::clear()
	{
		for (auto &i : models)
		{
			i.clear();
		}

		for (auto &i : subModelsNames)
		{
			delete[] i;
		}

		subModelsNames.clear();
		models.clear();

	}

	

	void GpuGraphicModel::loadFromComputedData(size_t vertexSize, const float *vercies, size_t indexSize, const unsigned int *indexes, bool noTexture)
	{

		gl3dAssertComment(indexSize % 3 == 0, "Index count must be multiple of 3");
		if (indexSize % 3 != 0)return;

		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexSize, vercies, GL_STATIC_DRAW);

		//todo this is only for rendering gizmos stuff
		if (noTexture)
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
		}
		else
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

		}


		if (indexSize && indexes)
		{
			glGenBuffers(1, &indexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexes, GL_STATIC_DRAW);

			primitiveCount = indexSize / sizeof(*indexes);

		}
		else
		{
			primitiveCount = vertexSize / sizeof(float);
		}

		glBindVertexArray(0);


	}

	void GpuGraphicModel::clear()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);

		glDeleteVertexArrays(1, &vertexArray);

		//albedoTexture.clear();
		//normalMapTexture.clear();
		//RMA_Texture.clear();

		vertexBuffer = 0;
		indexBuffer = 0;
		primitiveCount = 0;
		vertexArray = 0;
	}

	void SkyBoxLoaderAndDrawer::createGpuData()
	{
		normalSkyBox.shader.loadShaderProgramFromFile("shaders/skyBox/skyBox.vert", "shaders/skyBox/skyBox.frag");
		normalSkyBox.samplerUniformLocation = getUniform(normalSkyBox.shader.id, "u_skybox");
		normalSkyBox.modelViewUniformLocation = getUniform(normalSkyBox.shader.id, "u_viewProjection");

		hdrtoCubeMap.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/hdrToCubeMap.frag");
		hdrtoCubeMap.u_equirectangularMap = getUniform(hdrtoCubeMap.shader.id, "u_equirectangularMap");
		hdrtoCubeMap.modelViewUniformLocation = getUniform(hdrtoCubeMap.shader.id, "u_viewProjection");

		convolute.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/convolute.frag");
		convolute.u_environmentMap = getUniform(convolute.shader.id, "u_environmentMap");
		convolute.modelViewUniformLocation = getUniform(convolute.shader.id, "u_viewProjection");

		preFilterSpecular.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/preFilterSpecular.frag");
		preFilterSpecular.modelViewUniformLocation = getUniform(preFilterSpecular.shader.id, "u_viewProjection");
		preFilterSpecular.u_environmentMap = getUniform(preFilterSpecular.shader.id, "u_environmentMap");
		preFilterSpecular.u_roughness = getUniform(preFilterSpecular.shader.id, "u_roughness");


		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);


		glBindVertexArray(0);

		glGenFramebuffers(1, &captureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void SkyBoxLoaderAndDrawer::loadTexture(const char *names[6], SkyBox &skyBox)
	{
		skyBox = {};

		glGenTextures(1, &skyBox.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		for (unsigned int i = 0; i < 6; i++)
		{
			int w, h, nrChannels;
			unsigned char *data;

			stbi_set_flip_vertically_on_load(false);
			data = stbi_load(names[i], &w, &h, &nrChannels, 3);

			if (data)
			{

				glTexImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
							0, GL_SRGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);


				stbi_image_free(data);
			}
			else
			{
				std::cout << "err loading " << names[i] << "\n";
				glDeleteTextures(1, &skyBox.texture);
				return;
			}


		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		createConvolutedAndPrefilteredTextureData(skyBox);
	}

	void SkyBoxLoaderAndDrawer::loadTexture(const char *name, SkyBox &skyBox, int format)
	{
		skyBox = {};

		int width, height, nrChannels;
		unsigned char *data;


		stbi_set_flip_vertically_on_load(false);
		data = stbi_load(name, &width, &height, &nrChannels, 3);

		if (!data) { std::cout << "err loading " << name << "\n"; return; }

		glGenTextures(1, &skyBox.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		//right
		//left
		//top
		//bottom
		//front//?
		//back //?

		auto getPixel = [&](int x, int y, unsigned char *data)
		{
			return data + 3 * (x + y * width);
		};

		glm::ivec2 paddings[6];
		glm::ivec2 immageRatio = {};
		int flipX[6] = {};
		int flipY[6] = {};

		if (format == 0)
		{
			immageRatio = { 4, 3 };
			glm::ivec2 paddingscopy[6] =
			{
				{ (width / 4) * 2, (height / 3) * 1, },
				{ (width / 4) * 0, (height / 3) * 1, },
				{ (width / 4) * 1, (height / 3) * 0, },
				{ (width / 4) * 1, (height / 3) * 2, },
				{ (width / 4) * 1, (height / 3) * 1, },
				{ (width / 4) * 3, (height / 3) * 1, },
			};

			memcpy(paddings, paddingscopy, sizeof(paddings));


		}
		else if (format == 1)
		{
			immageRatio = { 3, 4 };
			glm::ivec2 paddingscopy[6] =
			{
				{ (width / 3) * 2, (height / 4) * 1, },
				{ (width / 3) * 0, (height / 4) * 1, },
				{ (width / 3) * 1, (height / 4) * 0, },
				{ (width / 3) * 1, (height / 4) * 2, },
				{ (width / 3) * 1, (height / 4) * 1, },
				{ (width / 3) * 1, (height / 4) * 3, },
			};

			memcpy(paddings, paddingscopy, sizeof(paddings));
			flipX[5] = 1;
			flipY[5] = 1;

		}else if(format == 2)
		{
			immageRatio = { 4, 3 };
			glm::ivec2 paddingscopy[6] =
			{
				{ (width / 4) * 3, (height / 3) * 1, },
				{ (width / 4) * 1, (height / 3) * 1, },
				{ (width / 4) * 2, (height / 3) * 0, },
				{ (width / 4) * 2, (height / 3) * 2, },
				{ (width / 4) * 2, (height / 3) * 1, },
				{ (width / 4) * 0, (height / 3) * 1, },

			};

			memcpy(paddings, paddingscopy, sizeof(paddings));

		}else
		{
			gl3dAssertComment(0, "invalid format for texture");
		}


		if (data)
		{
			for (unsigned int i = 0; i < 6; i++)
			{
				unsigned char *extractedData = new unsigned char[3 *
					(width / immageRatio.x) * (height / immageRatio.y)];

				int index = 0;

				int paddingX = paddings[i].x;
				int paddingY = paddings[i].y;

			#pragma region flip

				int jBeg = 0;
				int jAdvance = 1;
				if(flipY[i])
				{
					jBeg = height / immageRatio.y - 1;
					jAdvance = -1;
				}

				int xBeg = 0;
				int xAdvance = 1;
				if (flipX[i])
				{
					xBeg = width / immageRatio.x - 1;
					xAdvance = -1;
				}
			#pragma endregion


				for (int j = jBeg; j < height / immageRatio.y && j >= 0; j+= jAdvance)
					for (int i = xBeg; i < width / immageRatio.x && i >= 0; i+= xAdvance)
					{
						extractedData[index] = *getPixel(i + paddingX, j + paddingY, data);
						extractedData[index + 1] = *(getPixel(i + paddingX, j + paddingY, data) + 1);
						extractedData[index + 2] = *(getPixel(i + paddingX, j + paddingY, data) + 2);
						//extractedData[index] = 100;
						//extractedData[index + 1] = 100;
						//extractedData[index + 2] = 100;
						index += 3;
					}

				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_SRGB, width / immageRatio.x, height / immageRatio.y, 0,
					GL_RGB, GL_UNSIGNED_BYTE, extractedData
				);



				delete[] extractedData;
			}

			stbi_image_free(data);

		}
		else
		{
			std::cout << "err loading " << name << "\n";
		}


		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	
		createConvolutedAndPrefilteredTextureData(skyBox);

	}

	void SkyBoxLoaderAndDrawer::loadHDRtexture(const char *name, SkyBox &skyBox)
	{
		skyBox = {};

		int width, height, nrChannels;
		float *data;

		stbi_set_flip_vertically_on_load(true);
		data = stbi_loadf(name, &width, &height, &nrChannels, 0);
		if (!data) { std::cout << "err loading " << name << "\n"; return; }


		GLuint hdrTexture;

		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		//render into the cubemap
		//https://learnopengl.com/PBR/IBL/Diffuse-irradiance
		{
			GLuint captureFBO;
			glGenFramebuffers(1, &captureFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);


			glGenTextures(1, &skyBox.texture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
			for (unsigned int i = 0; i < 6; ++i)
			{
				// note that we store each face with 16 bit floating point values
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
							 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//rendering
			{

				hdrtoCubeMap.shader.bind();
				glUniform1i(hdrtoCubeMap.u_equirectangularMap, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, hdrTexture);

				glViewport(0, 0, 512, 512);

				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
				glBindVertexArray(vertexArray);

				for (unsigned int i = 0; i < 6; ++i)
				{
					glm::mat4 viewProjMat = captureProjection * captureViews[i];
					glUniformMatrix4fv(hdrtoCubeMap.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
										   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.texture, 0);
					glClear(GL_COLOR_BUFFER_BIT);

					glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube
				}

				glBindVertexArray(0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

			}

			glDeleteFramebuffers(1, &captureFBO);

		}

		glDeleteTextures(1, &hdrTexture);

		createConvolutedAndPrefilteredTextureData(skyBox);
	}

	void SkyBoxLoaderAndDrawer::createConvolutedAndPrefilteredTextureData(SkyBox &skyBox)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		GLint viewPort[4] = {};
		glGetIntegerv(GL_VIEWPORT, viewPort);


	#pragma region convoluted texture


		glGenTextures(1, &skyBox.convolutedTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.convolutedTexture);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
						 GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		convolute.shader.bind();
		glUniform1i(convolute.u_environmentMap, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glViewport(0, 0, 32, 32);

		glBindVertexArray(vertexArray);

		for (unsigned int i = 0; i < 6; ++i)
		{

			glm::mat4 viewProjMat = captureProjection * captureViews[i];
			glUniformMatrix4fv(convolute.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.convolutedTexture, 0);

			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube

		}
	#pragma endregion

	
	#pragma region prefiltered map

		constexpr int maxMipMap = 5;

		glGenTextures(1, &skyBox.preFilteredMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.preFilteredMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			//todo mabe be able to tweak rezolution
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, maxMipMap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		preFilterSpecular.shader.bind();
		glUniform1i(preFilterSpecular.u_environmentMap, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);


		for (int mip = 0; mip < maxMipMap; mip++)
		{
			unsigned int mipWidth = 128 * std::pow(0.5, mip);
			unsigned int mipHeight = 128 * std::pow(0.5, mip);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipMap - 1);
			roughness *= roughness;
			glUniform1f(preFilterSpecular.u_roughness, roughness);

			for (int i = 0; i < 6; i++)
			{
				glm::mat4 viewProjMat = captureProjection * captureViews[i];
				glUniformMatrix4fv(preFilterSpecular.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
									   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBox.preFilteredMap, mip);
				glClear(GL_COLOR_BUFFER_BIT);

				glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube
			}
		}



	#pragma endregion

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

		//texture = convolutedTexture; //visualize convolutex texture
		//texture = preFilteredMap;

		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//todo telete mipmaps
		//GLuint newTexture = 0;
		//glGenTextures(1, &newTexture);
		//int w, h;
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		//glCopyTexImage2D(GL_TEXTURE_CUBE_MAP, 0, GL_SRGB, 0, 0, w, h, 0);

	}

	void SkyBoxLoaderAndDrawer::draw(const glm::mat4 &viewProjMat, SkyBox &skyBox)
	{
		glBindVertexArray(vertexArray);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		normalSkyBox.shader.bind();

		glUniformMatrix4fv(normalSkyBox.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniform1i(normalSkyBox.samplerUniformLocation, 0);

		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
		glDepthFunc(GL_LESS);

		glBindVertexArray(0);
	}

};

#pragma endregion


////////////////////////////////////////////////
//gl3d.cpp
////////////////////////////////////////////////
#pragma region gl3d


#include <algorithm>
#include <stb_image.h>
#include <random>
#include <string>

namespace gl3d
{
	

	void Renderer3D::init(int x, int y)
	{
		w = x; h = y;

		lightShader.create();
		vao.createVAOs();
		internal.skyBoxLoaderAndDrawer.createGpuData();


		showNormalsProgram.shader.loadShaderProgramFromFile("shaders/showNormals.vert",
		"shaders/showNormals.geom", "shaders/showNormals.frag");


		showNormalsProgram.modelTransformLocation = glGetUniformLocation(showNormalsProgram.shader.id, "u_modelTransform");
		showNormalsProgram.projectionLocation = glGetUniformLocation(showNormalsProgram.shader.id, "u_projection");
		showNormalsProgram.sizeLocation = glGetUniformLocation(showNormalsProgram.shader.id, "u_size");
		showNormalsProgram.colorLocation = glGetUniformLocation(showNormalsProgram.shader.id, "u_color");
		
		unsigned char textureData[] =
		{
			20, 20, 20, 255,
			212, 0, 219, 255,
			212, 0, 219, 255,
			20, 20, 20, 255,
		};

		defaultTexture.loadTextureFromMemory(textureData, 2, 2, 4, TextureLoadQuality::leastPossible);


		//create gBuffer
		glGenFramebuffers(1, &gBuffer.gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);

		glGenTextures(gBuffer.bufferCount, gBuffer.buffers);


		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.position]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.position], 0);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal], 0);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo], 0);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material], 0);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.positionViewSpace]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.positionViewSpace], 0);


		unsigned int attachments[decltype(gBuffer)::bufferCount] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, 
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(decltype(gBuffer)::bufferCount, attachments);

		glGenRenderbuffers(1, &gBuffer.depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, gBuffer.depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, x, y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gBuffer.depthBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Gbuffer failed\n";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ssao.create(x, y);
		postProcess.create(x, y);

	}

	void Renderer3D::VAO::createVAOs()
	{
		glGenVertexArrays(1, &posNormalTexture);
		glBindVertexArray(posNormalTexture);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

		glBindVertexArray(0);

	}
	
	Material Renderer3D::createMaterial(glm::vec3 kd, float roughness, float metallic, float ao
	, std::string name)
	{

		int id = internal::generateNewIndex(materialIndexes);

		GpuMaterial gpuMaterial;
		gpuMaterial.kd = glm::vec4(kd, 0);
		gpuMaterial.roughness = roughness;
		gpuMaterial.metallic = metallic;
		gpuMaterial.ao = ao;

		materialIndexes.push_back(id);
		materials.push_back(gpuMaterial);
		materialNames.push_back(name);
		materialTexturesData.push_back({});

		Material m;
		m.id_ = id;
		return m;

	}

	Material Renderer3D::createMaterial(Material m)
	{
		auto newM = createMaterial();
		copyMaterialData(newM, m);

		return newM;
	}

	Material Renderer3D::loadMaterial(std::string file)
	{

		objl::Loader loader;
		loader.LoadFile(file);



		return Material();
	}

	void Renderer3D::deleteMaterial(Material m)
	{
		auto pos = std::find(materialIndexes.begin(), materialIndexes.end(), m.id_);

		if (pos == materialIndexes.end())
		{
			gl3dAssertComment(pos == materialIndexes.end(), "invalid delete material");
			return;
		}

		int index = pos - materialIndexes.begin();

		materialIndexes.erase(pos);
		materials.erase(materials.begin() + index);
		materialNames.erase(materialNames.begin() + index);
		materialTexturesData.erase(materialTexturesData.begin() + index);
		m.id_ = 0;
	}

	void Renderer3D::copyMaterialData(Material dest, Material source)
	{
		int destId = getMaterialIndex(dest);
		int sourceId = getMaterialIndex(source);

		if(destId == -1 || sourceId == -1)
		{
			gl3dAssertComment(destId != -1, "invaled dest material index");
			gl3dAssertComment(sourceId != -1, "invaled source material index");

			return;
		}

		materials[destId] = materials[sourceId];
		materialNames[destId] = materialNames[sourceId];
		materialTexturesData[destId] = materialTexturesData[destId];

	}

	GpuMaterial *Renderer3D::getMaterialData(Material m)
	{
		int id = getMaterialIndex(m);

		if(id == -1)
		{
			return nullptr;
		}
		
		auto data = &materials[id];

		return data;
	}

	TextureDataForModel *Renderer3D::getMaterialTextures(Material m)
	{
		int id = getMaterialIndex(m);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &materialTexturesData[id];

		return data;
	}

	std::string *Renderer3D::getMaterialName(Material m)
	{
		int id = getMaterialIndex(m);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &materialNames[id];

		return data;
	}

	bool Renderer3D::getMaterialData(Material m, GpuMaterial *gpuMaterial, std::string *name, TextureDataForModel *textureData)
	{
		int id = getMaterialIndex(m);

		if (id == -1)
		{
			return false;
		}

		if(gpuMaterial)
		{
			gpuMaterial = &materials[id];
		}

		if(name)
		{
			name = &materialNames[id];
		}

		if(textureData)
		{
			textureData = &materialTexturesData[id];
		}

		return true;
	}

	bool Renderer3D::setMaterialData(Material m, const GpuMaterial &data, std::string *s)
	{
		int id = getMaterialIndex(m);

		if (id == -1)
		{
			return 0;
		}

		materials[id] = data;
		
		if (s)
		{
			materialNames[id] = *s;
		}

		return 1;
	}

	GpuMultipleGraphicModel *Renderer3D::getObjectData(Object o)
	{
		int id = getObjectIndex(o);
	
		if (id == -1)
		{
			return nullptr;
		}
	
		auto data = &graphicModels[id];
	
		return data;
	}

	Texture Renderer3D::loadTexture(std::string path, bool defaultToDefaultTexture)
	{

		if(path == "")
		{
			return Texture{ 0 };
		}

		int pos = 0;
		for (auto &i : loadedTexturesNames)
		{
			if (i == path)
			{
				Texture t;
				t.id_ = loadedTexturesIndexes[pos];
				return t;
			}
			pos++;
		}

		GpuTexture t(path.c_str());

		if(t.id == 0 && defaultToDefaultTexture == false)
		{
			return Texture{ 0 };
		}


		int id = internal::generateNewIndex(loadedTexturesIndexes);

		//if texture is not loaded, set it to default
		if(t.id == 0)
		{
			t.id = defaultTexture.id;
		}

		loadedTexturesIndexes.push_back(id);
		loadedTextures.push_back(t);
		loadedTexturesNames.push_back(path);

		return Texture{ id };
	}

	GLuint Renderer3D::getTextureOpenglId(Texture t)
	{
		auto p = getTextureData(t);

		if(p == nullptr)
		{
			return 0;
		}else
		{
			return p->id;
		}
	}

	void Renderer3D::deleteTexture(Texture t)
	{
		int index = getTextureIndex(t);

		if(index < 0)
		{
			return;
		}

		auto gpuTexture = loadedTextures[index];

		if(gpuTexture.id != defaultTexture.id)
		{
			gpuTexture.clear();
		}

		loadedTexturesIndexes.erase(loadedTexturesIndexes.begin() + index);
		loadedTextures.erase(loadedTextures.begin() + index);
		loadedTexturesNames.erase(loadedTexturesNames.begin() + index);
		
		t.id_ = 0;

	}

	GpuTexture *Renderer3D::getTextureData(Texture t)
	{
		int id = getTextureIndex(t);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &loadedTextures[id];

		return data;
	}

	Texture Renderer3D::createIntenralTexture(GpuTexture t)
	{

		int id = internal::generateNewIndex(loadedTexturesIndexes);

		//if t is null initialize to default texture
		if (t.id == 0)
		{
			t.id = defaultTexture.id;
		}

		loadedTexturesIndexes.push_back(id);
		loadedTextures.push_back(t);
		loadedTexturesNames.push_back("");

		return Texture{ id };
	}

	static int max(int x, int y, int z)
	{
		return std::max(std::max(x, y), z);
	}

	Object Renderer3D::loadObject(std::string path, float scale)
	{

		gl3d::LoadedModelData model(path.c_str(), scale);
		if(model.loader.LoadedMeshes.empty())
		{
			std::cout << "err loading " + path + "\n";
			return { 0 };
		
		}

		int id = internal::generateNewIndex(graphicModelsIndexes);
	
		GpuMultipleGraphicModel returnModel;
		{

			int s = model.loader.LoadedMeshes.size();
			returnModel.models.reserve(s);


			std::vector<gl3d::Material> loadedMaterials;
			loadedMaterials.reserve(model.loader.LoadedMaterials.size());
			for(int i=0;i<model.loader.LoadedMaterials.size(); i++)
			{
				auto &mat = model.loader.LoadedMaterials[i];
				auto m = this->createMaterial(mat.Kd, mat.roughness,
				mat.metallic, mat.ao);
				

				{
					//load textures for materials
					TextureDataForModel *textureData = this->getMaterialTextures(m);

					

					//auto &mat = model.loader.LoadedMeshes[index].MeshMaterial;
					//gm.material = loadedMaterials[model.loader.LoadedMeshes[index].materialIndex];

					//gm.albedoTexture.clear();
					//gm.normalMapTexture.clear();
					//gm.RMA_Texture.clear();

					if (!mat.map_Kd.empty())
					{
						//gm.albedoTexture.loadTextureFromFile(std::string(model.path + mat.map_Kd).c_str());
						textureData->albedoTexture = this->loadTexture(std::string(model.path + mat.map_Kd), 0);
					}

					if (!mat.map_Kn.empty())
					{
						//todo add texture load quality options
						textureData->normalMapTexture = this->loadTexture(std::string(model.path + mat.map_Kn), 0);
						//gm.normalMapTexture.loadTextureFromFile(std::string(model.path + mat.map_Kn).c_str(),
						//	TextureLoadQuality::linearMipmap);
					}

					textureData->RMA_loadedTextures = 0;

					auto rmaQuality = TextureLoadQuality::linearMipmap;

					if (!mat.map_RMA.empty()) //todo not tested
					{
						//gm.RMA_Texture.loadTextureFromFile(mat.map_RMA.c_str(),
						//rmaQuality);

						textureData->RMA_Texture = this->loadTexture(mat.map_RMA.c_str());

						//todo add a function to check if a function is valid
						if (getTextureData(textureData->RMA_Texture)->id == defaultTexture.id)
						{
							textureData->RMA_loadedTextures = 7; //all textures loaded
						}

						//if (gm.RMA_Texture.id)
						//{
						//	gm.RMA_loadedTextures = 7; //all textures loaded
						//}

					}

					if (!mat.map_ORM.empty() && textureData->RMA_loadedTextures == 0)
					{
						stbi_set_flip_vertically_on_load(true);

						int w = 0, h = 0;
						unsigned char *data = 0;


						{
							data = stbi_load(std::string(model.path + mat.map_ORM).c_str(),
							&w, &h, 0, 4);
							if (!data)
							{
								std::cout << "err loading " << std::string(model.path + mat.map_ORM) << "\n";
							}
							else
							{
								//convert from ORM ro RMA

								for (int j = 0; j < h; j++)
									for (int i = 0; i < w; i++)
									{
										unsigned char R = data[(i + j * w) * 4 + 1];
										unsigned char M = data[(i + j * w) * 4 + 2];
										unsigned char A = data[(i + j * w) * 4 + 0];

										data[(i + j * w) * 4 + 0] = R;
										data[(i + j * w) * 4 + 1] = M;
										data[(i + j * w) * 4 + 2] = A;
									}

								//gm.RMA_Texture.loadTextureFromMemory(data, w, h, 4, rmaQuality);
								GpuTexture t;
								t.loadTextureFromMemory(data, w, h, 4, rmaQuality);
								textureData->RMA_Texture = this->createIntenralTexture(t);

								textureData->RMA_loadedTextures = 7; //all textures loaded

								stbi_image_free(data);
							}
						}


					}

					//RMA trexture
					if (textureData->RMA_loadedTextures == 0)
					{
						stbi_set_flip_vertically_on_load(true);

						int w1 = 0, h1 = 0;
						unsigned char *data1 = 0;
						unsigned char *data2 = 0;
						unsigned char *data3 = 0;

						if (!mat.map_Pr.empty())
						{
							data1 = stbi_load(std::string(model.path + mat.map_Pr).c_str(),
							&w1, &h1, 0, 1);
							if (!data1) { std::cout << "err loading " << std::string(model.path + mat.map_Pr) << "\n"; }
						}

						int w2 = 0, h2 = 0;
						if (!mat.map_Pm.empty())
						{
							data2 = stbi_load(std::string(model.path + mat.map_Pm).c_str(),
							&w2, &h2, 0, 1);
							if (!data2) { std::cout << "err loading " << std::string(model.path + mat.map_Pm) << "\n"; }
						}


						int w3 = 0, h3 = 0;
						if (!mat.map_Ka.empty())
						{
							data3 = stbi_load(std::string(model.path + mat.map_Ka).c_str(),
								&w3, &h3, 0, 1);
							if (!data3) { std::cout << "err loading " << std::string(model.path + mat.map_Ka) << "\n"; }
						}

						int w = max(w1, w2, w3);
						int h = max(h1, h2, h3);

						//calculate which function to use
						if (data1 && data2 && data3) { textureData->RMA_loadedTextures = 7; }
						else
						if (data2 && data3) { textureData->RMA_loadedTextures = 6; }
						else
						if (data1 && data3) { textureData->RMA_loadedTextures = 5; }
						else
						if (data1 && data2) { textureData->RMA_loadedTextures = 4; }
						else
						if (data3) { textureData->RMA_loadedTextures = 3; }
						else
						if (data2) { textureData->RMA_loadedTextures = 2; }
						else
						if (data1) { textureData->RMA_loadedTextures = 1; }
						else { textureData->RMA_loadedTextures = 0; }

						if (textureData->RMA_loadedTextures)
						{

							unsigned char *finalData = new unsigned char[w * h * 4];

							//todo mabe add bilinear filtering
							//todo load less chanels if necessary
							for (int j = 0; j < h; j++)
							{
								for (int i = 0; i < w; i++)
								{

									if (data1)	//rough
									{
										int texelI = (i / (float)w) * w1;
										int texelJ = (j / float(h)) * h1;

										finalData[((j * w) + i) * 4 + 0] =
											data1[(texelJ * w1) + texelI];

									}
									else
									{
										finalData[((j * w) + i) * 4 + 0] = 0;
									}

									if (data2)	//metalic
									{

										int texelI = (i / (float)w) * w2;
										int texelJ = (j / float(h)) * h2;

										finalData[((j * w) + i) * 4 + 1] =
											data2[(texelJ * w2) + texelI];
									}
									else
									{
										finalData[((j * w) + i) * 4 + 1] = 0;
									}

									if (data3)	//ambient
									{
										int texelI = (i / (float)w) * w3;
										int texelJ = (j / float(h)) * h3;

										finalData[((j * w) + i) * 4 + 2] =
											data3[(texelJ * w3) + texelI];
									}
									else
									{
										finalData[((j * w) + i) * 4 + 2] = 0;
									}

									finalData[((j * w) + i) * 4 + 3] = 255; //used only for imgui, remove later
								}
							}

							//gm.RMA_Texture.loadTextureFromMemory(finalData, w, h, 4,
							//	rmaQuality);

							GpuTexture t;
							t.loadTextureFromMemory(finalData, w, h, 4, rmaQuality);
							textureData->RMA_Texture = this->createIntenralTexture(t);

							stbi_image_free(data1);
							stbi_image_free(data2);
							stbi_image_free(data3);
							delete[] finalData;

						}

					}





					
				}

				loadedMaterials.push_back(m);
			}


			for (int i = 0; i < s; i++)
			{
				GpuGraphicModel gm;
				int index = i;
				GpuMaterial material;
				//TextureDataForModel textureData = {};

				
				auto &mesh = model.loader.LoadedMeshes[index];
				gm.loadFromComputedData(mesh.Vertices.size() * 8 * 4,
					 (float *)&mesh.Vertices[0],
					mesh.Indices.size() * 4, &mesh.Indices[0]);


				if(model.loader.LoadedMeshes[index].materialIndex > -1)
				{
					gm.material = loadedMaterials[model.loader.LoadedMeshes[index].materialIndex];
				}else
				{
					//if no material loaded for this object create a new default one
					gm.material = createMaterial(glm::vec3{ 0.8 }, 0.5, 0);
				}
				

				gm.name = model.loader.LoadedMeshes[i].MeshName;
				char *c = new char[gm.name.size() + 1];
				strcpy(c, gm.name.c_str());

				returnModel.subModelsNames.push_back(c);
				returnModel.models.push_back(gm);

			}


		}

		
		graphicModelsIndexes.push_back(id);
		graphicModels.push_back(returnModel);


		Object o;
		o.id_ = id;
		return o;

	}

	void Renderer3D::deleteObject(Object o)
	{
		auto pos = std::find(graphicModelsIndexes.begin(), graphicModelsIndexes.end(), o.id_);

		if (pos == graphicModelsIndexes.end())
		{
			gl3dAssertComment(pos == graphicModelsIndexes.end(), "invalid delete object");
			return;
		}

		int index = pos - graphicModelsIndexes.begin();

		graphicModelsIndexes.erase(pos);

		graphicModels[index].clear();
		graphicModels.erase(graphicModels.begin() + index);

		o.id_ = 0;
	}

	void Renderer3D::renderObject(Object o, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);

		auto found = std::find(graphicModelsIndexes.begin(), graphicModelsIndexes.end(), o.id_);
		if (found == graphicModelsIndexes.end())
		{
			gl3dAssertComment(found == graphicModelsIndexes.end(), "invalid render object");
			return;
		}
		int id = found - graphicModelsIndexes.begin();

		auto &model = graphicModels[id];


		if (model.models.empty())
		{
			return;
		}

		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = gl3d::getTransformMatrix(position, rotation, scale);

		auto modelViewProjMat = projMat * viewMat * transformMat;
		//auto modelView = viewMat * transformMat;

		lightShader.geometryPassShader.bind();


		lightShader.getSubroutines();


		glUniformMatrix4fv(lightShader.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);
		glUniformMatrix4fv(lightShader.u_modelTransform, 1, GL_FALSE, &transformMat[0][0]);
		glUniformMatrix4fv(lightShader.u_motelViewTransform, 1, GL_FALSE, &(viewMat * transformMat)[0][0]);
		//glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		//glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(lightShader.textureSamplerLocation, 0);
		glUniform1i(lightShader.normalMapSamplerLocation, 1);
		//glUniform1i(lightShader.skyBoxSamplerLocation, 2);
		glUniform1i(lightShader.RMASamplerLocation, 3);



		//material buffer
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightShader.materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuMaterial) * materials.size()
			, &materials[0], GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightShader.materialBlockBuffer);

		//glBindVertexArray(vao.posNormalTexture);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		for (auto &i : model.models)
		{
			
			glBindVertexArray(i.vertexArray);
			//glBindBuffer(GL_ARRAY_BUFFER, i.vertexBuffer);

			if (i.indexBuffer)
			{
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i.indexBuffer);
				glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
			}
		}

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthFunc(GL_EQUAL);

		GLsizei n;
		glGetProgramStageiv(lightShader.geometryPassShader.id,
		GL_FRAGMENT_SHADER,
		GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
		&n);

		GLuint *indices = new GLuint[n]{ 0 };
		bool changed = 1;	
	
		for (auto &i : model.models)
		{
			
			int materialId = getMaterialIndex(i.material);

			if (materialId == -1)
				{ continue; }

			glUniform1i(lightShader.materialIndexLocation, materialId);
			

			TextureDataForModel textureData = materialTexturesData[materialId];

			int rmaLoaded = 0;
			int albedoLoaded = 0;
			int normalLoaded = 0;

			GpuTexture *albedoTextureData = this->getTextureData(textureData.albedoTexture);
			if(albedoTextureData != nullptr )
			{
				albedoLoaded = 1;
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, albedoTextureData->id);
			}

			GpuTexture *normalMapTextureData = this->getTextureData(textureData.normalMapTexture);
			if(normalMapTextureData != nullptr && normalMapTextureData->id != defaultTexture.id)
			{
				normalLoaded = 1;
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, normalMapTextureData->id);
			}
		
			//todo refactor default texture, just keep it totally separate and treat -1 as default texture
			GpuTexture *rmaTextureData = this->getTextureData(textureData.RMA_Texture);
			if(rmaTextureData != nullptr && rmaTextureData->id != defaultTexture.id)
			{
				rmaLoaded = 1;
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, rmaTextureData->id);
			}



	
			if (normalLoaded && lightShader.normalMap)
			{
				if (indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_normalMap)
				{
					indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_normalMap;
					changed = 1;
				}
			}
			else
			{
				if (indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_noMap)
				{
					indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_noMap;
					changed = 1;
				}
			}
	
			if(rmaLoaded)
			{

				if (indices[lightShader.materialSubroutineLocation] != lightShader.materialSubroutine_functions[textureData.RMA_loadedTextures])
				{
					indices[lightShader.materialSubroutineLocation] = lightShader.materialSubroutine_functions[textureData.RMA_loadedTextures];
					changed = 1;
				}
			
			}else
			{
				if(indices[lightShader.materialSubroutineLocation] != lightShader.materialSubroutine_functions[0])
				{
					indices[lightShader.materialSubroutineLocation] = lightShader.materialSubroutine_functions[0];
					changed = 1;
				}

			}

			
			if(albedoLoaded != 0)
			{
				if (indices[lightShader.getAlbedoSubroutineLocation] != lightShader.albedoSubroutine_sampled)
				{
					indices[lightShader.getAlbedoSubroutineLocation] = lightShader.albedoSubroutine_sampled;
					changed = 1;
				}
			}
			else
			if (indices[lightShader.getAlbedoSubroutineLocation] != lightShader.albedoSubroutine_notSampled)
			{
				indices[lightShader.getAlbedoSubroutineLocation] = lightShader.albedoSubroutine_notSampled;
				changed = 1;
			}


			if (changed)
			{
				glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, n, indices);
			}
			changed = 0;
	
			{
				glBindVertexArray(i.vertexArray);

				if (i.indexBuffer)
				{
					glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
				}
				else
				{
					glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
				}
			}
	
		}

		glBindVertexArray(0);

		delete[] indices;
	
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void Renderer3D::renderObjectNormals(Object o, glm::vec3 position, glm::vec3 rotation, 
		glm::vec3 scale, float normalSize, glm::vec3 normalColor)
	{
		auto obj = getObjectData(o);

		if(!obj)
		{
			return;
		}

		for(int i=0; i<obj->models.size(); i++)
		{
			renderSubObjectNormals(o, i, position, rotation, scale, normalSize, normalColor);
		}
		
	}

	void Renderer3D::renderSubObjectNormals(Object o, int index, glm::vec3 position, glm::vec3 rotation,
		glm::vec3 scale, float normalSize, glm::vec3 normalColor)
	{
			
		showNormalsProgram.shader.bind();
		
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = gl3d::getTransformMatrix(position, rotation, scale);
		
		auto viewTransformMat = viewMat * transformMat;
		
		glUniformMatrix4fv(showNormalsProgram.modelTransformLocation,
			1, GL_FALSE, &viewTransformMat[0][0]);
		
		glUniformMatrix4fv(showNormalsProgram.projectionLocation,
			1, GL_FALSE, &projMat[0][0]);
		
		glUniform1f(showNormalsProgram.sizeLocation, normalSize);

		glUniform3fv(showNormalsProgram.colorLocation, 1, &(normalColor[0]));

		auto modelIndex = this->getObjectIndex(o);

		auto obj = getObjectData(o);
		if(obj == nullptr)
		{
			return;
		}

		{
			if(index >= obj->models.size())
			{
				return;
			}

			auto &i = obj->models[index];
			
			glBindVertexArray(i.vertexArray);

			if (i.indexBuffer)
			{
				glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
			}
			else
			{
				glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
			}
			glBindVertexArray(0);
		}

	}

	void Renderer3D::renderSubObjectBorder(Object o, int index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, float borderSize, glm::vec3 borderColor)
	{
		//auto modelIndex = this->getObjectIndex(o);
		//
		//auto obj = getObjectData(o);
		//if (obj == nullptr)
		//{
		//	return;
		//}
		//
		//if (index >= obj->models.size())
		//{
		//	return;
		//}
		//
		//	
		//glEnable(GL_STENCIL_TEST);
		//glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		//glStencilFunc(GL_ALWAYS, 1, 0xFF);
		//glStencilMask(0xFF);	
		//
		//auto projMat = renderer.camera.getProjectionMatrix();
		//auto viewMat = renderer.camera.getWorldToViewMatrix();
		//auto transformMat = models[0].getTransformMatrix();
		//
		//auto viewProjMat = projMat * viewMat * transformMat;
		//
		////todo implement a light weight shader here
		////lightShader.bind(viewProjMat, transformMat,
		////	lightCubeModel.position, renderer.camera.position, gamaCorection,
		////	models[itemCurrent].models[subItemCurent].material, renderer.pointLights);
		//
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//models[itemCurrent].models[subItemCurent].draw();
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//
		//glDisable(GL_STENCIL_TEST);
		//
		//glEnable(GL_STENCIL_TEST);
		//glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		//glDepthFunc(GL_ALWAYS);
		//glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		//glStencilMask(0x00);
		//
		//auto &m = models[itemCurrent].models[subItemCurent];
		//projMat = renderer.camera.getProjectionMatrix();
		//viewMat = renderer.camera.getWorldToViewMatrix();
		//
		//auto rotation = models[itemCurrent].rotation;
		//auto scale = models[itemCurrent].scale;
		//scale *= 1.05;
		//auto position = models[itemCurrent].position;
		//
		//
		//auto s = glm::scale(scale);
		//auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
		//	glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
		//	glm::rotate(rotation.z, glm::vec3(0, 0, 1));
		//auto t = glm::translate(position);
		//
		//transformMat = t * r * s;
		//
		//viewProjMat = projMat * viewMat * transformMat;
		//
		//shader.bind();
		//glUniformMatrix4fv(location, 1, GL_FALSE, &viewProjMat[0][0]);
		//
		//glBindBuffer(GL_ARRAY_BUFFER, m.vertexBuffer);
		//
		//glEnableVertexAttribArray(0);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		//glVertexAttrib3f(1, 98 / 255.f, 24 / 255.f, 201 / 255.f);
		//
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.indexBuffer);
		//glDrawElements(GL_TRIANGLES, m.primitiveCount, GL_UNSIGNED_INT, 0);
		//
		//glDisable(GL_STENCIL_TEST);
		//glDepthFunc(GL_LESS);


	}

	int Renderer3D::getMaterialIndex(Material m)
	{
		int id = m.id_;
		auto found = std::find(materialIndexes.begin(), materialIndexes.end(), id);
		if (found == materialIndexes.end())
		{
			gl3dAssertComment(found == materialIndexes.end(), "invalid material");
			return -1;
		}
		id = found - materialIndexes.begin();

		return id;
	}

	int Renderer3D::getObjectIndex(Object o)
	{
		int id = o.id_;
		auto found = std::find(graphicModelsIndexes.begin(), graphicModelsIndexes.end(), id);
		if (found == graphicModelsIndexes.end())
		{
			gl3dAssertComment(found == graphicModelsIndexes.end(), "invalid object");
			return -1;
		}
		id = found - graphicModelsIndexes.begin();
	
		return id;
	}

	int Renderer3D::getTextureIndex(Texture t)
	{
		int id = t.id_;
		if (id == 0) { return -1; }//todo add this optimization to other gets

		auto found = std::find(loadedTexturesIndexes.begin(), loadedTexturesIndexes.end(), id);
		if (found == loadedTexturesIndexes.end())
		{
			gl3dAssertComment(found == loadedTexturesIndexes.end(), "invalid texture");
			return -1;
		}
		id = found - loadedTexturesIndexes.begin();

		return id;
	}

	void Renderer3D::render()
	{
		//we draw a rect several times so we keep this vao binded
		glBindVertexArray(lightShader.quadDrawer.quadVAO);
		
	#pragma region ssao
		glViewport(0, 0, w / 2, h / 2);

		glUseProgram(ssao.shader.id);

		glBindBuffer(GL_UNIFORM_BUFFER, ssao.ssaoUniformBlockBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SSAO::SsaoShaderUniformBlockData),
			&ssao.ssaoShaderUniformBlockData);

		glUniformMatrix4fv(ssao.u_projection, 1, GL_FALSE,
			&(camera.getProjectionMatrix())[0][0] );

		glUniformMatrix4fv(ssao.u_view, 1, GL_FALSE,
			&(camera.getWorldToViewMatrix())[0][0]);

		glUniform3fv(ssao.u_samples, 64, &(ssao.ssaoKernel[0][0]));

		glBindFramebuffer(GL_FRAMEBUFFER, ssao.ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.positionViewSpace]);
		glUniform1i(ssao.u_gPosition, 0);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);
		glUniform1i(ssao.u_gNormal, 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ssao.noiseTexture);
		glUniform1i(ssao.u_texNoise, 2);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
		glViewport(0, 0, w, h);
	#pragma endregion

	#pragma region ssao "blur" (more like average blur)
		glViewport(0, 0, w/4, h/4);

		glBindFramebuffer(GL_FRAMEBUFFER, ssao.blurBuffer);
		ssao.blurShader.bind();
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssao.ssaoColorBuffer);
		glUniform1i(ssao.u_ssaoInput, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glViewport(0, 0, w, h);
	#pragma endregion

	#pragma region render into the post processing fbo

		glBindFramebuffer(GL_FRAMEBUFFER, postProcess.fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(lightShader.lightingPassShader.id);

		glUniform1i(lightShader.light_u_positions, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.position]);

		glUniform1i(lightShader.light_u_normals, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);

		glUniform1i(lightShader.light_u_albedo, 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo]);

		glUniform1i(lightShader.light_u_materials, 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material]);

		glUniform1i(lightShader.light_u_ssao, 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, ssao.blurColorBuffer);
		//glBindTexture(GL_TEXTURE_2D, ssao.ssaoColorBuffer);

		glUniform1i(lightShader.light_u_skyboxFiltered, 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.preFilteredMap);

		glUniform1i(lightShader.light_u_skyboxIradiance, 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.convolutedTexture);

		glUniform1i(lightShader.light_u_brdfTexture, 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, lightShader.brdfTexture.id);

		glUniform3f(lightShader.light_u_eyePosition, camera.position.x, camera.position.y, camera.position.z);

		glUniformMatrix4fv(lightShader.light_u_view, 1, GL_FALSE, &(camera.getWorldToViewMatrix()[0][0]) );

		if (pointLights.size())
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightShader.pointLightsBlockBuffer);
		
			glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(internal::GpuPointLight)
				, &pointLights[0], GL_STREAM_DRAW);
		
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightShader.pointLightsBlockBuffer);
		
		}

		glUniform1i(lightShader.light_u_pointLightCount, pointLights.size());

		glUniform1i(lightShader.u_useSSAO, lightShader.useSSAO);

		//update the uniform block with data for the light shader
		glBindBuffer(GL_UNIFORM_BUFFER, lightShader.lightPassShaderData.lightPassDataBlockBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightShader::LightPassData),
			&lightShader.lightPassUniformBlockCpuData);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	#pragma endregion

	#pragma region bloom blur
		
		if(lightShader.bloom)
		{
			
			bool horizontal = 1; bool firstTime = 1;
			postProcess.gausianBLurShader.bind();
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(postProcess.u_toBlurcolorInput, 0);
			glViewport(0, 0, w/2, h/2);
			for (int i = 0; i < lightShader.bloomBlurPasses*2; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[horizontal]);
				glClear(GL_COLOR_BUFFER_BIT);
				glUniform1i(postProcess.u_horizontal, horizontal);

				glBindTexture(GL_TEXTURE_2D,
					firstTime ? postProcess.colorBuffers[1] : postProcess.bluredColorBuffer[!horizontal]);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				horizontal = !horizontal;
				firstTime = false;

			}
			glViewport(0, 0, w, h);

		}

	#pragma endregion

	#pragma region do the post process stuff and draw to screen

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(postProcess.postProcessShader.id);

		//color data
		glUniform1i(postProcess.u_colorTexture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[0]);

		//bloom data
		glUniform1i(postProcess.u_bloomTexture, 1);
		glActiveTexture(GL_TEXTURE1);
		if(lightShader.bloom)
		{
			glBindTexture(GL_TEXTURE_2D, postProcess.bluredColorBuffer[1]); 
		}else
		{
			//if the bloom is not enabeled just pass the colorBuffer[1]
			//i.e. the values that passed the bloom tresshold
			//glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[1]);
			//todo test on other drivers not bind a texture at all
			glBindTexture(GL_TEXTURE_2D, 0);

		}

		//bloom settings
		glUniform1f(postProcess.u_bloomIntensity, postProcess.bloomIntensty);

		

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	#pragma endregion


	#pragma region copy depth buffer for later forward rendering
		glBindVertexArray(0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		glBlitFramebuffer(
		  0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	#pragma endregion


	}

	void Renderer3D::updateWindowMetrics(int x, int y)
	{

		if(w == x && h == y)
		{
			return;
		}
		
		w = x; h = y;

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.position]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.positionViewSpace]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindRenderbuffer(GL_RENDERBUFFER, gBuffer.depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, x, y);
		
		//todo bindless stuff
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ssao
		glBindTexture(GL_TEXTURE_2D, ssao.ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/2, h/2, 0, GL_RED, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, ssao.blurColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/4, h/4, 0, GL_RED, GL_FLOAT, NULL);

		glBindFramebuffer(GL_FRAMEBUFFER, ssao.ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, ssao.blurBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//bloom
		for (int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
		}

		for(int i=0;i<2;i++)
		{
			glBindTexture(GL_TEXTURE_2D, postProcess.bluredColorBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w/2, h/2, 0, GL_RGBA, GL_FLOAT, NULL);

			glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[i]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		}

		glBindFramebuffer(GL_FRAMEBUFFER, postProcess.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


	}

	void Renderer3D::renderSkyBox()
	{
		//todo move into render

		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		viewMat = glm::mat4(glm::mat3(viewMat));

		auto viewProjMat = projMat * viewMat;

		internal.skyBoxLoaderAndDrawer.draw(viewProjMat, skyBox);
	}

	SkyBox Renderer3D::loadSkyBox(const char *names[6])
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.loadTexture(names, skyBox);
		return skyBox;
	}

	SkyBox Renderer3D::loadSkyBox(const char *name, int format)
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.loadTexture(name, skyBox, format);
		return skyBox;
	}

	SkyBox Renderer3D::loadHDRSkyBox(const char *name)
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.loadHDRtexture(name, skyBox);
		return skyBox;
	}

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer3D::SSAO::create(int w, int h)
	{
		std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
		std::uniform_real_distribution<float> randomFloatsSmaller(0.1f, 0.9f); //avoid ssao artefacts
		std::default_random_engine generator;

		ssaoKernel.reserve(64);

		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)	// z component is always positive
			);
			sample = glm::normalize(sample);

			float scale = (float)i / 64.0;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);

		}
		//std::shuffle(ssaoKernel.begin(), ssaoKernel.end(), generator);


		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/2, h/2, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);


		shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/ssao/ssao.frag");


		u_projection = getUniform(shader.id, "u_projection");
		u_view = getUniform(shader.id, "u_view");
		u_gPosition = getUniform(shader.id, "u_gPosition");
		u_gNormal = getUniform(shader.id, "u_gNormal");
		u_texNoise = getUniform(shader.id, "u_texNoise");
		u_samples = getUniform(shader.id, "samples[0]");
		

		glGenBuffers(1, &ssaoUniformBlockBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, ssaoUniformBlockBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SsaoShaderUniformBlockData),
			&ssaoShaderUniformBlockData, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, ssaoUniformBlockBuffer);
		
		u_SSAODATA = glGetUniformBlockIndex(shader.id, "u_SSAODATA");
		glUniformBlockBinding(shader.id, u_SSAODATA, 2);

		//blur
		blurShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/ssao/blur.frag");
		
		glGenFramebuffers(1, &blurBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer);
		glGenTextures(1, &blurColorBuffer);
		glBindTexture(GL_TEXTURE_2D, blurColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/4, h/4, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorBuffer, 0);
		u_ssaoInput = getUniform(blurShader.id, "u_ssaoInput");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer3D::PostProcess::create(int w, int h)
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		//one for colors the other for things to be bloomed
		glGenTextures(2, colorBuffers);
		for (int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach texture to framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
		}

		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		
		postProcessShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/postProcess/postProcess.frag");
		u_colorTexture = getUniform(postProcessShader.id, "u_colorTexture");
		u_bloomTexture = getUniform(postProcessShader.id, "u_bloomTexture");
		u_bloomIntensity = getUniform(postProcessShader.id, "u_bloomIntensity");

		gausianBLurShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/postProcess/gausianBlur.frag");
		u_toBlurcolorInput = getUniform(gausianBLurShader.id, "u_toBlurcolorInput");
		u_horizontal = getUniform(gausianBLurShader.id, "u_horizontal");


		glGenFramebuffers(2, blurFbo);
		glGenTextures(2, bluredColorBuffer);

		for(int i=0;i <2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[i]);

			glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w/2, h/2, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bluredColorBuffer[i], 0);
		}
		

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

};
#pragma endregion


