////////////////////////////////////////////////
//gl32 --Vlad Luta -- 
//built on 2020-12-18
////////////////////////////////////////////////

#include "gl3d.h"

////////////////////////////////////////////////
//Core.cpp
////////////////////////////////////////////////
#pragma region Core

#include <stdio.h>
#include <Windows.h>
#include <signal.h>

void gl3d::assertFunc(const char *expression,
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

#pragma endregion


////////////////////////////////////////////////
//Texture.cpp
////////////////////////////////////////////////
#pragma region Texture

#include <stb_image.h>
#include <iostream>

namespace gl3d
{

	void Texture::loadTextureFromFile(const char *file)
	{
		int w, h, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load(file, &w, &h, &nrChannels, 4);

		if (!data)
		{
			//todo err messages
			std::cout << "err: " << file << "\n";
		}
		else
		{
			loadTextureFromMemory(data, w, h);
			stbi_image_free(data);
		}


	}

	void Texture::loadTextureFromMemory(void *data, int w, int h)
	{
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8);


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

			message = new char[l];

			glGetShaderInfoLog(shaderId, l, &l, message);

			message[l - 1] = 0;

			std::cout << source << ": " << message << "\n";

			delete[] message;

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

		//todo probably free the created shader


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

	void Shader::deleteShaderProgram()
	{
		glDeleteProgram(id);
		id = 0;
	}

	void Shader::bind()
	{
		glUseProgram(id);
	}

	GLint getUniform(GLuint id, const char *name)
	{
		GLint uniform = glGetUniformLocation(id, name);
		if (uniform == -1)
		{
			std::cout << "uniform error " << name << "\n";
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

	}

	void LightShader::bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition
		)
	{
		shader.bind();
		glUniformMatrix4fv(normalShaderLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniformMatrix4fv(normalShaderNormalTransformLocation, 1, GL_FALSE, &transformMat[0][0]);
		glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(textureSamplerLocation, 0);
		glUniform1i(normalMapSamplerLocation, 1);
		glUniform1i(skyBoxSamplerLocation, 2);
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

		gl3dAssertComment(vertexSize % 3 == 0, "Index count must be multiple of 3");
		if (vertexSize % 3 != 0)return;


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

	void GraphicModel::loadFromModelMeshIndex(const LoadedModelData &model, int index)
	{
		auto &mesh = model.loader.LoadedMeshes[0];
		loadFromComputedData(mesh.Vertices.size() * 8 * 4,
			 (float *)&mesh.Vertices[0],
			mesh.Indices.size() * 4, &mesh.Indices[0]);

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
		auto s = glm::scale(scale);
		auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(rotation.z, glm::vec3(0, 0, 1));
		auto t = glm::translate(position);

		return t * r * s;

	}



	void LoadedModelData::load(const char *file)
	{
		loader.LoadFile(file);
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


		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		
		glBindVertexArray(0);
	}

	void SkyBox::loadTexture(const char *names[6])
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

		int width, height, nrChannels;
		unsigned char *data;
		for (unsigned int i = 0; i <6; i++)
		{
			stbi_set_flip_vertically_on_load(false);
			data = stbi_load(names[i], &width, &height, &nrChannels, 3);

			if (data)
			{
				glTexImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
							0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);

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

	void SkyBox::loadTexture(const char *name)
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

		glm::ivec2 paddings[6] =
		{
			{(width / 4) * 2, (height / 3) * 1, },
			{(width / 4) * 0, (height / 3) * 1, },
			{(width / 4) * 1, (height / 3) * 0, },
			{(width / 4) * 1, (height / 3) * 2, },
			{(width / 4) * 1, (height / 3) * 1, },
			{(width / 4) * 3, (height / 3) * 1, },
		};

		if (data)
		{
			for (unsigned int i = 0; i < 6; i++)
			{
				unsigned char *extractedData = new unsigned char[3 * (width / 4) * (height / 3)];

				int index = 0;

				int paddingX = paddings[i].x;
				int paddingY = paddings[i].y;

				for (int j = 0; j < height / 3; j++)
					for (int i = 0; i < width / 4; i++)
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
								0, GL_RGB, width/4, height/3, 0, GL_RGB, GL_UNSIGNED_BYTE, extractedData
					);



				delete[] extractedData;
			}

			stbi_image_free(data);

		}else
		{
			std::cout << "err loading " << name << "\n";
		}

	
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	}

	void SkyBox::clearGpuData()
	{
	}

	void SkyBox::draw(const glm::mat4 &viewProjMat)
	{
		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(vertexArray);

		bindCubeMap();

		shader.bind();

		glUniformMatrix4fv(modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniform1i(samplerUniformLocation, 0);

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

};

#pragma endregion


////////////////////////////////////////////////
//gl3d.cpp
////////////////////////////////////////////////
#pragma region gl3d

namespace gl3d
{


	void renderLightModel(GraphicModel &model, Camera camera, glm::vec3 lightPos, LightShader lightShader,
		Texture texture, Texture normalTexture, GLuint skyBoxTexture)
	{
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = model.getTransformMatrix();

		auto viewProjMat = projMat * viewMat * transformMat;


		lightShader.bind(viewProjMat, transformMat, lightPos, camera.position);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.id);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture.id);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

		model.draw();

	}

};
#pragma endregion


