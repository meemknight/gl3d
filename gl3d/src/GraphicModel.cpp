#include "GraphicModel.h"
#include "Core.h"
#include "OBJ_Loader.h"
#include <stb_image.h>
#include "Texture.h"
#include <algorithm>
#include "gl3d.h"

namespace gl3d 
{

	void DebugGraphicModel::loadFromComputedData(size_t vertexSize, const float * vercies, size_t indexSize,
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

	
	void DebugGraphicModel::clear()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);

		glDeleteVertexArrays(1, &vertexArray);

		vertexBuffer = 0;
		indexBuffer = 0;
		primitiveCount = 0;
		vertexArray = 0;
	}

	void DebugGraphicModel::draw()
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

	glm::mat4 DebugGraphicModel::getTransformMatrix()
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

	glm::mat4 getTransformMatrix(const Transform& t)
	{
		return getTransformMatrix(t.position, t.rotation, t.scale);
	}

	glm::mat4 Transform::getTransformMatrix()
	{
		return gl3d::getTransformMatrix(*this);
	}


	void ModelData::clear(Renderer3D& renderer)
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
		
		for (auto& i : createdMaterials)
		{
			renderer.deleteMaterial(i);
		}
		createdMaterials.clear();

		models.clear();

	}

	

	void GraphicModel::loadFromComputedData(size_t vertexSize, const float *vercies, size_t indexSize, const unsigned int *indexes, bool noTexture)
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

	void GraphicModel::clear()
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
		normalSkyBox.u_exposure = getUniform(normalSkyBox.shader.id, "u_exposure");
		normalSkyBox.u_ambient = getUniform(normalSkyBox.shader.id, "u_ambient");
		normalSkyBox.u_skyBoxPresent = getUniform(normalSkyBox.shader.id, "u_skyBoxPresent");
		
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

		atmosphericScatteringShader.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert",
			"shaders/skyBox/atmosphericScattering.frag");
		//atmosphericScatteringShader.u_lightPos = getUniform(atmosphericScatteringShader.shader.id, "u_lightPos");
		//atmosphericScatteringShader.u_g = getUniform(atmosphericScatteringShader.shader.id, "u_g");
		//atmosphericScatteringShader.u_g2 = getUniform(atmosphericScatteringShader.shader.id, "u_g2");
		atmosphericScatteringShader.modelViewUniformLocation 
			= getUniform(atmosphericScatteringShader.shader.id, "u_viewProjection");


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
			GLuint captureFBO; //todo cache this
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

	void SkyBoxLoaderAndDrawer::atmosphericScattering(glm::vec3 sun, float g, float g2, SkyBox& skyBox)
	{
		skyBox = {};

		//render into the cubemap
		{
			GLuint captureFBO; 
			glGenFramebuffers(1, &captureFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);


			glGenTextures(1, &skyBox.texture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
			for (unsigned int i = 0; i < 6; ++i)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, //todo 16 is probably too much
					512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//rendering
			{

				atmosphericScatteringShader.shader.bind();
				//glUniform3fv(atmosphericScatteringShader.u_lightPos, 1, &sun[0]);
				//glUniform1f(atmosphericScatteringShader.u_g, g);
				//glUniform1f(atmosphericScatteringShader.u_g2, g2);

				glViewport(0, 0, 512, 512);

				glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
				glBindVertexArray(vertexArray);

				for (unsigned int i = 0; i < 6; ++i)
				{
					glm::mat4 viewProjMat = captureProjection * captureViews[i];
					glUniformMatrix4fv(atmosphericScatteringShader.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
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


		createConvolutedAndPrefilteredTextureData(skyBox);

	}

	void SkyBoxLoaderAndDrawer::createConvolutedAndPrefilteredTextureData(SkyBox &skyBox)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		GLint viewPort[4] = {};
		glGetIntegerv(GL_VIEWPORT, viewPort); //todo remove because slow


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

	void SkyBoxLoaderAndDrawer::draw(const glm::mat4 &viewProjMat, SkyBox &skyBox, float exposure,
		glm::vec3 ambient)
	{
		glBindVertexArray(vertexArray);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		normalSkyBox.shader.bind();

		glUniformMatrix4fv(normalSkyBox.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniform1i(normalSkyBox.samplerUniformLocation, 0);
		glUniform1f(normalSkyBox.u_exposure, exposure);
		glUniform3f(normalSkyBox.u_ambient, ambient.r, ambient.g, ambient.b);

		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
		glDepthFunc(GL_LESS);

		glBindVertexArray(0);
	}

	void SkyBoxLoaderAndDrawer::drawBefore(const glm::mat4& viewProjMat, SkyBox& skyBox, float exposure,
		glm::vec3 ambient)
	{
		glBindVertexArray(vertexArray);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		normalSkyBox.shader.bind();

		bool skyBoxPresent = 1;

		if (skyBox.texture == 0 || skyBox.convolutedTexture == 0 || skyBox.preFilteredMap == 0)
		{
			skyBoxPresent = 0;
		}

		glUniformMatrix4fv(normalSkyBox.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniform1i(normalSkyBox.samplerUniformLocation, 0);
		glUniform1i(normalSkyBox.u_skyBoxPresent, skyBoxPresent);
		glUniform1f(normalSkyBox.u_exposure, exposure);
		glUniform3f(normalSkyBox.u_ambient, ambient.r, ambient.g, ambient.b);


		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(0);
	}

	void SkyBox::clearTextures()
	{
		glDeleteTextures(3, (GLuint*)this);
		texture = 0;
		convolutedTexture = 0;
		preFilteredMap = 0;
	}

};
