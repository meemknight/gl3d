////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on 2021-02-14
////////////////////////////////////////////////

#include "gl3d.h"

////////////////////////////////////////////////
//Core.cpp
////////////////////////////////////////////////
#pragma region Core

#include <stdio.h>
#include <Windows.h>
#include <signal.h>

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


};

#pragma endregion


////////////////////////////////////////////////
//Texture.cpp
////////////////////////////////////////////////
#pragma region Texture

#include <stb_image.h>
#include <iostream>
#include <glm\vec3.hpp>


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
		shader.loadShaderProgramFromFile("shaders/normals.vert", "shaders/normals.frag");
		shader.bind();

		normalShaderLocation = getUniform(shader.id, "u_transform");
		normalShaderNormalTransformLocation = getUniform(shader.id, "u_modelTransform");
		normalShaderLightposLocation = getUniform(shader.id, "u_lightPosition");
		textureSamplerLocation = getUniform(shader.id, "u_albedoSampler");
		normalMapSamplerLocation = getUniform(shader.id, "u_normalSampler");
		eyePositionLocation = getUniform(shader.id, "u_eyePosition");
		skyBoxSamplerLocation = getUniform(shader.id, "u_skybox");
		gamaLocation = getUniform(shader.id, "u_gama");
		RMASamplerLocation = getUniform(shader.id, "u_RMASampler");
		pointLightCountLocation = getUniform(shader.id, "u_pointLightCount");
		materialIndexLocation = getUniform(shader.id, "u_materialIndex");
		//pointLightBufferLocation = getUniform(shader.id, "u_pointLights");
		
		//todo geb buffer for each material
		materialBlockLocation = getStorageBlockIndex(shader.id, "u_material");
		glShaderStorageBlockBinding(shader.id, materialBlockLocation, 0);
		
		glGenBuffers(1, &materialBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);



		pointLightsBlockLocation = getStorageBlockIndex(shader.id, "u_pointLights");
		glShaderStorageBlockBinding(shader.id, pointLightsBlockLocation, 1);

		glGenBuffers(1, &pointLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsBlockBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightsBlockBuffer);

	}

	void LightShader::bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition, float gama,
		const internal::GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights)
	{
		shader.bind();
		
		this->setData(viewProjMat, transformMat, lightPosition, eyePosition, gama, 
			material, pointLights);

	}

	void LightShader::setData(const glm::mat4 &viewProjMat, 
		const glm::mat4 &transformMat, const glm::vec3 &lightPosition, const glm::vec3 &eyePosition,
		float gama, const internal::GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights)
	{
		glUniformMatrix4fv(normalShaderLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniformMatrix4fv(normalShaderNormalTransformLocation, 1, GL_FALSE, &transformMat[0][0]);
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

	void LightShader::setMaterial(const internal::GpuMaterial &material)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(material)
			, &material, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, materialBlockBuffer);
		glUniform1i(materialIndexLocation, 0);
	}



	void LightShader::getSubroutines()
	{
		normalSubroutineLocation = getUniformSubroutine(shader.id, GL_FRAGMENT_SHADER,
			"getNormalMapFunc");

		normalSubroutine_noMap = getUniformSubroutineIndex(shader.id, GL_FRAGMENT_SHADER,
			"noNormalMapped");

		normalSubroutine_normalMap = getUniformSubroutineIndex(shader.id, GL_FRAGMENT_SHADER,
				"normalMapped");


		materialSubroutineLocation = getUniformSubroutine(shader.id, GL_FRAGMENT_SHADER,
			"u_getMaterialMapped");

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
			materialSubroutine_functions[i] = getUniformSubroutineIndex(shader.id, GL_FRAGMENT_SHADER,
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


#include <OBJ_Loader.h>
#include <stb_image.h>


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

						finalData[((j * w) + i) * 4 + 3] = 255; //used only for imgui, remove later
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

	void SkyBox::createGpuData()
	{
		shader.loadShaderProgramFromFile("shaders/skyBox.vert", "shaders/skyBox.frag");

		samplerUniformLocation = getUniform(shader.id, "u_skybox");
		modelViewUniformLocation = getUniform(shader.id, "u_viewProjection");
		gamaUniformLocation = getUniform(shader.id, "u_gama");


		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		
		glBindVertexArray(0);
	}

	//todo add srgb
	void SkyBox::loadTexture(const char *names[6])
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

		int w, h, nrChannels;
		unsigned char *data;
		for (unsigned int i = 0; i <6; i++)
		{
			stbi_set_flip_vertically_on_load(false);
			data = stbi_load(names[i], &w, &h, &nrChannels, 3);

			if (data)
			{

				glTexImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
							0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);

				//gausianBlurRGB(data, w, h, 10);

				//glTexImage2D(
				//			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				//			1, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				//);


				stbi_image_free(data);
			}
			else
			{
				std::cout << "err loading " << names[i] << "\n";
			}

			
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	}

	//todo add srgb
	void SkyBox::loadTexture(const char *name, int format)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

		int width, height, nrChannels;
		unsigned char *data;


		stbi_set_flip_vertically_on_load(false);
		data = stbi_load(name, &width, &height, &nrChannels, 3);

		//right
		//left
		//top
		//bottom
		//front
		//back

		auto getPixel = [&](int x, int y, unsigned char *data)
		{
			return data + 3 * (x + y * width);
		};

		glm::ivec2 paddings[6];
		glm::ivec2 immageRatio = {};

		if(format == 0)
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

		}else if (format == 1)
		{
			immageRatio = { 3, 4 };
			glm::ivec2 paddingscopy[6] =
			{
				{ (width / 3) * 2, (height / 4) * 1, },
				{ (width / 3) * 0, (height / 4) * 1, },
				{ (width / 3) * 1, (height / 4) * 0, },
				{ (width / 3) * 1, (height / 4) * 2, },
				{ (width / 3) * 1, (height / 4) * 3, },
				{ (width / 3) * 1, (height / 4) * 1, },
			};

			memcpy(paddings, paddingscopy, sizeof(paddings));
			
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

				for (int j = 0; j < height / immageRatio.y; j++)
					for (int i = 0; i < width / immageRatio.x; i++)
					{
						extractedData[index] = *getPixel(i + paddingX, j + paddingY, data);
						extractedData[index + 1] = *(getPixel(i + paddingX, j + paddingY, data)+1);
						extractedData[index + 2] = *(getPixel(i + paddingX, j + paddingY, data)+2);
						//extractedData[index] = 100;
						//extractedData[index + 1] = 100;
						//extractedData[index + 2] = 100;
						index += 3;
					}

					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						0, GL_RGB, width/ immageRatio.x, height/ immageRatio.y, 0,
						GL_RGB, GL_UNSIGNED_BYTE, extractedData
					);



				delete[] extractedData;
			}

			stbi_image_free(data);

		}else
		{
			std::cout << "err loading " << name << "\n";
		}

		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	}

	void SkyBox::clearGpuData()
	{
	}

	void SkyBox::draw(const glm::mat4 &viewProjMat, float gama)
	{
		glBindVertexArray(vertexArray);

		bindCubeMap();

		shader.bind();

		glUniformMatrix4fv(modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniform1i(samplerUniformLocation, 0);
		glUniform1f(gamaUniformLocation, gama);

		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLES, 0, 6*6);
		glDepthFunc(GL_LESS);

		glBindVertexArray(0);
	}

	void SkyBox::bindCubeMap()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	}

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

		//todo clear material buffer
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

		albedoTexture.clear();
		normalMapTexture.clear();
		RMA_Texture.clear();

		vertexBuffer = 0;
		indexBuffer = 0;
		primitiveCount = 0;
		vertexArray = 0;
	}

};

#pragma endregion


////////////////////////////////////////////////
//gl3d.cpp
////////////////////////////////////////////////
#pragma region gl3d


#include <algorithm>
#include <stb_image.h>

namespace gl3d
{
	void renderLightModel(GraphicModel &model, Camera camera, glm::vec3 lightPos, LightShader lightShader, Texture texture, Texture normalTexture, GLuint skyBoxTexture, 
		float gama, const internal::GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights)
	{

		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = model.getTransformMatrix();

		auto viewProjMat = projMat * viewMat * transformMat;

		lightShader.bind(viewProjMat, transformMat, lightPos, camera.position, gama, material,
			pointLights);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.id);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture.id);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

		model.draw();


	}


	void renderLightModel(MultipleGraphicModels &model, Camera camera, glm::vec3 lightPos, 
		LightShader lightShader, GLuint skyBoxTexture, float gama, std::vector<internal::GpuPointLight> &pointLights)
	{
		if(model.models.empty())
		{
			return;
		}

		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = model.getTransformMatrix();

		auto modelViewProjMat = projMat * viewMat * transformMat;
		//auto modelView = viewMat * transformMat;

		lightShader.shader.bind();

		lightShader.getSubroutines();
		lightShader.setData(modelViewProjMat, transformMat, lightPos, camera.position, gama, internal::GpuMaterial(),
			pointLights);

		GLsizei n;
		//glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &n);
		glGetProgramStageiv(lightShader.shader.id,
		GL_FRAGMENT_SHADER,
		GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
		&n);

		GLuint *indices = new GLuint[n]{ 0 };
		bool changed = 1;

		for(auto &i : model.models)
		{
			lightShader.setMaterial(i.material);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, i.albedoTexture.id);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, i.normalMapTexture.id);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, i.RMA_Texture.id);


			if (i.normalMapTexture.id && lightShader.normalMap)
			{
				if(indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_normalMap)
				{
					changed = 1;
				}
				indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_normalMap;
			}else
			{
				if (indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_normalMap)
				{
					changed = 1;
				}
				indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_noMap;
			}

			if(indices[lightShader.materialSubroutineLocation] != lightShader.materialSubroutine_functions[i.RMA_loadedTextures])
			{ 
				changed = 1;
			}

			indices[lightShader.materialSubroutineLocation] = lightShader.materialSubroutine_functions[i.RMA_loadedTextures];

			if(changed)
			{
				glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, n, indices);
			}
			changed = 0;

			i.draw();


		}

		delete[] indices;

		
	}

	void Renderer3D::init()
	{
		lightShader.create();


	}

	Material Renderer3D::createMaterial(glm::vec3 kd, float roughness, float metallic, float ao)
	{
		int id = 0;

		auto materialIndexesCopy = materialIndexes;
		std::sort(materialIndexesCopy.begin(), materialIndexesCopy.end());
		
		if(materialIndexesCopy.empty())
		{
			id = 1;
		}else
		{
			id = 1;

			for (int i = 0; i< materialIndexesCopy.size(); i++)
			{
				if(materialIndexesCopy[i] != id)
				{
					break;
				}else
				{
					id++;
				}
			}

		}

		internal::GpuMaterial gpuMaterial;
		gpuMaterial.kd = kd;
		gpuMaterial.roughness = roughness;
		gpuMaterial.metallic = metallic;
		gpuMaterial.ao = ao;

		materialIndexes.push_back(id);
		materials.push_back(gpuMaterial);

		Material m;
		m._id = id;
		return m;

	}

	void Renderer3D::deleteMaterial(Material m)
	{
		auto pos = std::find(materialIndexes.begin(), materialIndexes.end(), m._id);

		if (pos == materialIndexes.end())
		{
			gl3dAssertComment(pos == materialIndexes.end(), "invalid delete material");
			return;
		}

		int index = pos - materialIndexes.begin();

		materialIndexes.erase(pos);
		materials.erase(materials.begin() + index);
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

		int id = 0;

		auto objectIndexesCopy = graphicModelsIndexes;
		std::sort(objectIndexesCopy.begin(), objectIndexesCopy.end());

		if (objectIndexesCopy.empty())
		{
			id = 1;
		}
		else
		{
			id = 1;

			for (int i = 0; i < objectIndexesCopy.size(); i++)
			{
				if (objectIndexesCopy[i] != id)
				{
					break;
				}
				else
				{
					id++;
				}
			}

		}

		GpuMultipleGraphicModel returnModel;


		{

			int s = model.loader.LoadedMeshes.size();
			returnModel.models.reserve(s);

			for (int i = 0; i < s; i++)
			{
				GpuGraphicModel gm;
				int index = i;
				internal::GpuMaterial material;

				{
					auto &mesh = model.loader.LoadedMeshes[index];
					gm.loadFromComputedData(mesh.Vertices.size() * 8 * 4,
						 (float *)&mesh.Vertices[0],
						mesh.Indices.size() * 4, &mesh.Indices[0]);


					auto &mat = model.loader.LoadedMeshes[index].MeshMaterial;
					gm.material = this->createMaterial(mat.Kd, mat.roughness,
					material.metallic /*,mat.ao todo*/ );


					gm.albedoTexture.clear();
					gm.normalMapTexture.clear();
					gm.RMA_Texture.clear();

					if (!mat.map_Kd.empty())
					{
						gm.albedoTexture.loadTextureFromFile(std::string(model.path + mat.map_Kd).c_str());
					}

					if (!mat.map_Kn.empty())
					{
						gm.normalMapTexture.loadTextureFromFile(std::string(model.path + mat.map_Kn).c_str(),
							TextureLoadQuality::linearMipmap);
					}

					gm.RMA_loadedTextures = 0;

					auto rmaQuality = TextureLoadQuality::linearMipmap;

					if (!mat.map_RMA.empty()) //todo not tested
					{
						gm.RMA_Texture.loadTextureFromFile(mat.map_RMA.c_str(),
						rmaQuality);

						if (gm.RMA_Texture.id)
						{
							gm.RMA_loadedTextures = 7; //all textures loaded
						}

					}

					if (!mat.map_ORM.empty() && gm.RMA_loadedTextures == 0)
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

								gm.RMA_Texture.loadTextureFromMemory(data, w, h, 4, rmaQuality);

								gm.RMA_loadedTextures = 7; //all textures loaded

								stbi_image_free(data);
							}
						}


					}

					//RMA trexture
					if (gm.RMA_loadedTextures == 0)
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
						if (data1 && data2 && data3) { gm.RMA_loadedTextures = 7; }
						else
						if (data2 && data3) { gm.RMA_loadedTextures = 6; }
						else
						if (data1 && data3) { gm.RMA_loadedTextures = 5; }
						else
						if (data1 && data2) { gm.RMA_loadedTextures = 4; }
						else
						if (data3) { gm.RMA_loadedTextures = 3; }
						else
						if (data2) { gm.RMA_loadedTextures = 2; }
						else
						if (data1) { gm.RMA_loadedTextures = 1; }
						else {gm.RMA_loadedTextures = 0;
												};
						if (gm.RMA_loadedTextures)
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

							gm.RMA_Texture.loadTextureFromMemory(finalData, w, h, 4,
								rmaQuality);

							stbi_image_free(data1);
							stbi_image_free(data2);
							stbi_image_free(data3);
							delete[] finalData;

						}

					}

				
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
		o._id = id;
		return o;

	}

	void Renderer3D::deleteObject(Object o)
	{
		auto pos = std::find(graphicModelsIndexes.begin(), graphicModelsIndexes.end(), o._id);

		if (pos == graphicModelsIndexes.end())
		{
			gl3dAssertComment(pos == graphicModelsIndexes.end(), "invalid delete object");
			return;
		}

		int index = pos - graphicModelsIndexes.begin();

		graphicModelsIndexes.erase(pos);

		graphicModels[index].clear();
		graphicModels.erase(graphicModels.begin() + index);
	}

	void Renderer3D::renderObject(Object o, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		
		auto found = std::find(graphicModelsIndexes.begin(), graphicModelsIndexes.end(), o._id);
		if(found == graphicModelsIndexes.end())
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
		auto transformMat = gl3d::getTransformMatrix(position, rotation, scale);;
	
		auto modelViewProjMat = projMat * viewMat * transformMat;
		//auto modelView = viewMat * transformMat;
	
		lightShader.shader.bind();
	
		lightShader.getSubroutines();
		lightShader.setData(modelViewProjMat, transformMat, {}, camera.position, 2.2, internal::GpuMaterial(),
			pointLights);

		//material buffer
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightShader.materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(internal::GpuMaterial) * materials.size()
			, &materials[0], GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightShader.materialBlockBuffer);


		GLsizei n;
		//glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &n);
		glGetProgramStageiv(lightShader.shader.id,
		GL_FRAGMENT_SHADER,
		GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
		&n);
	
		GLuint *indices = new GLuint[n]{ 0 };
		bool changed = 1;
		
	
		for (auto &i : model.models)
		{
			//lightShader.setMaterial(i.material);
			{
				int id = i.material._id;
				auto found = std::find(materialIndexes.begin(), materialIndexes.end(), id);
				if (found == materialIndexes.end())
				{
					gl3dAssertComment(found == materialIndexes.end(), "invalid material during render object");
					continue;
				}
				id = found - materialIndexes.begin();

				glUniform1i(lightShader.materialIndexLocation, id);
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, i.albedoTexture.id);
	
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, i.normalMapTexture.id);
	
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
	
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, i.RMA_Texture.id);
	
	
			if (i.normalMapTexture.id && lightShader.normalMap)
			{
				if (indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_normalMap)
				{
					changed = 1;
				}
				indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_normalMap;
			}
			else
			{
				if (indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_normalMap)
				{
					changed = 1;
				}
				indices[lightShader.normalSubroutineLocation] = lightShader.normalSubroutine_noMap;
			}
	
			if (indices[lightShader.materialSubroutineLocation] != lightShader.materialSubroutine_functions[i.RMA_loadedTextures])
			{
				changed = 1;
			}
	
			indices[lightShader.materialSubroutineLocation] = lightShader.materialSubroutine_functions[i.RMA_loadedTextures];
	
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
				glBindVertexArray(0);
			}
	
		}
	
		delete[] indices;
	
	
	}

};
#pragma endregion


