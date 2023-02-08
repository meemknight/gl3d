#include "GraphicModel.h"
#include "Core.h"
#include "OBJ_Loader.h"
#include "Texture.h"
#include <algorithm>
#include "gl3d.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/euler_angles.hpp"

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


	void LoadedModelData::load(const char *file, ErrorReporter &errorReporter, FileOpener &fileOpener, float scale)
	{
		stbi_set_flip_vertically_on_load(true); //gltf and obj files hold textures flipped diferently...
		bool shouldFlipUVs = 0;

		loader.LoadFile(file, errorReporter, fileOpener, &shouldFlipUVs);

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
			if (shouldFlipUVs)
			{
				for (auto &j : i.Vertices)
				{
					j.Position.X *= scale;
					j.Position.Y *= scale;
					j.Position.Z *= scale;
					j.TextureCoordinate.Y = 1.f - j.TextureCoordinate.Y;
				}

				for (auto &j : i.VerticesAnimations)
				{
					j.Position.X *= scale;
					j.Position.Y *= scale;
					j.Position.Z *= scale;
					j.TextureCoordinate.Y = 1.f - j.TextureCoordinate.Y;
				}
			}
			else
			{
				for (auto &j : i.Vertices)
				{
					j.Position.X *= scale;
					j.Position.Y *= scale;
					j.Position.Z *= scale;
				}

				for (auto &j : i.VerticesAnimations)
				{
					j.Position.X *= scale;
					j.Position.Y *= scale;
					j.Position.Z *= scale;
				}
			}
		}

		for (auto &i : loader.joints)
		{
			//i.scale *= scale;
			//i.trans *= scale;
		}

		for (auto &i : loader.animations)
		{
			for (auto &k1 : i.keyFramesTrans)
			{
				for (auto &k2 : k1)
				{
					//k2.translation *= scale;
				}
			}

			for (auto &k1 : i.keyFramesScale)
			{
				for (auto &k2 : k1)
				{
				//	k2.scale *= scale;
				}
			}
		}

		//errorReporter.currentErrorCallback(std::string("Loaded: ") + std::to_string(loader.LoadedMeshes.size()) + " meshes");
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
		//auto r = glm::rotate(rotation.x, glm::vec3(1, 0, 0)) *
		//	glm::rotate(rotation.y, glm::vec3(0, 1, 0)) *
		//	glm::rotate(rotation.z, glm::vec3(0, 0, 1));

		auto r = glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);

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

	//https://stackoverflow.com/questions/1996957/conversion-euler-to-matrix-and-matrix-to-euler
	void getRotation(const glm::mat4 &mat, float& Yaw, float& Pitch, float& Roll)
	{
		Pitch = asinf(-mat[2][1]);                  // Pitch
		if (abs(cosf(Pitch)) > 0.0001)                 // Not at poles
		{
			Yaw = atan2f(mat[2][0], mat[2][2]);     // Yaw
			Roll = atan2f(mat[0][1], mat[1][1]);     // Roll
		}
		else
		{
			Yaw = 0.0f;								// Yaw
			Roll = atan2f(-mat[1][0], mat[0][0]);    // Roll
		}
	}

	void Transform::setFromMatrix(const glm::mat4& mat)
	{
		glm::quat rot = {};
		glm::vec3 notUsed = {};
		glm::vec4 notUsed2 = {};

		glm::decompose(mat, scale, rot, position, notUsed, notUsed2);
		//rot = glm::conjugate(rot);
		//scale = glm::normalize(scale);
		//rotation = glm::eulerAngles(rot);
		//rotation.z *= -1;
		
		//if (abs(rotation.x) < 0.01)
		//{
		//	rotation.x = 0;
		//}
		//if (abs(rotation.y) < 0.01)
		//{
		//	rotation.y = 0;
		//}
		//if (abs(rotation.z) < 0.01)
		//{
		//	rotation.z = 0;
		//}
		
		//std::swap(rotation.x, rotation.z);
		getRotation(mat, rotation.y, rotation.x, rotation.z);
		//rotation.y = -rotation.y;
		//rotation.x = -rotation.x;
		//rotation = -rotation;
	}

	void ModelData::clear(Renderer3D& renderer)
	{
		for (auto &i : createdMaterials)
		{
			renderer.deleteMaterial(i);
		}

		internalClear();
	}

	void ModelData::internalClear()
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

		createdMaterials.clear();

		models.clear();
	}

	

	void GraphicModel::loadFromComputedData(size_t vertexSize, const float *vertices, size_t indexSize, 
		const unsigned int *indexes, bool noTexture, bool hasAnimationData,
		std::vector<Animation> animation, std::vector<Joint> joints)
	{
		/*
			position					vec3
			normals						vec3
			(optional) texcoords		vec2
			(optional) joints id		ivec4
			(optional) joints weights	vec4
		*/

		//todo check has texture data.

		#pragma region validate

		if (!vertexSize) { return; }


		gl3dAssertComment(indexSize % 3 == 0, "Index count must be multiple of 3");
		if (indexSize % 3 != 0)return;

		if (hasAnimationData)
		{
			if (noTexture)
			{
				gl3dAssertComment(vertexSize % (sizeof(float) * 14) == 0,
					"VertexSize count must be multiple of 14 * sizeof(float)\nwhen not using texture data\nand using animation data.");
				if (vertexSize % (sizeof(float) * 14) != 0)return;
			}
			else
			{
				gl3dAssertComment(vertexSize % (sizeof(float) * 16) == 0,
					"VertexSize count must be multiple of 16 * sizeof(float)\nwhen using texture data\nand using animation data.");
				if (vertexSize % (sizeof(float) * 16) != 0)return;
			}
		}
		else
		{
			if (noTexture)
			{
				gl3dAssertComment(vertexSize % (sizeof(float) * 6) == 0,
					"VertexSize count must be multiple of 6 * sizeof(float)\nwhen not using texture data\nand no animation data.");
				if (vertexSize % (sizeof(float) * 6) != 0)return;
			}
			else
			{
				gl3dAssertComment(vertexSize % (sizeof(float) * 8) == 0,
					"VertexSize count must be multiple of 8 * sizeof(float)\nwhen using texture data\nand no animation data.");
				if (vertexSize % (sizeof(float) * 8) != 0)return;
			}
		}

		
		#pragma endregion

		#pragma region determine object boundaries
		
		if (noTexture)
		{
			float x = vertices[0];
			float y = vertices[1];
			float z = vertices[2];

			minBoundary = glm::vec3(x, y, z);
			maxBoundary = glm::vec3(x, y, z);

			for (int i = 0; i < vertexSize / (sizeof(float)*6); i++)
			{
				float x = vertices[i*6+0];
				float y = vertices[i*6+1];
				float z = vertices[i*6+2];
			
				if (x < minBoundary.x) { minBoundary.x = x; }
				if (y < minBoundary.y) { minBoundary.y = y; }
				if (z < minBoundary.z) { minBoundary.z = z; }
			
				if (x > maxBoundary.x) { maxBoundary.x = x; }
				if (y > maxBoundary.y) { maxBoundary.y = y; }
				if (z > maxBoundary.z) { maxBoundary.z = z; }
			}
		}
		else
		{
			float x = vertices[0];
			float y = vertices[1];
			float z = vertices[2];

			minBoundary = glm::vec3(x, y, z);
			maxBoundary = glm::vec3(x, y, z);

			for (int i = 0; i < vertexSize / (sizeof(float) * 8); i++)
			{
				float x = vertices[i * 8 + 0];
				float y = vertices[i * 8 + 1];
				float z = vertices[i * 8 + 2];

				if (x < minBoundary.x) { minBoundary.x = x; }
				if (y < minBoundary.y) { minBoundary.y = y; }
				if (z < minBoundary.z) { minBoundary.z = z; }

				if (x > maxBoundary.x) { maxBoundary.x = x; }
				if (y > maxBoundary.y) { maxBoundary.y = y; }
				if (z > maxBoundary.z) { maxBoundary.z = z; }
			}
		}

		#pragma endregion

		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexSize, vertices, GL_STATIC_DRAW);


		if (hasAnimationData)
		{
			if (noTexture)
			{
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(3 * sizeof(float)));
				//skip texture
				glEnableVertexAttribArray(3);
				glVertexAttribIPointer(3, 4, GL_INT, 14 * sizeof(int), (void *)(6 * sizeof(int)));
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(10 * sizeof(float)));
			}
			else
			{
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(3 * sizeof(float)));
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(6 * sizeof(float)));
				glEnableVertexAttribArray(3);
				glVertexAttribIPointer(3, 4, GL_INT, 16 * sizeof(int), (void *)(8 * sizeof(int)));
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(12 * sizeof(float)));

			}
		}
		else
		{
			//double check this
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

	//	does not clear owned material
	void GraphicModel::clear()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);

		glDeleteVertexArrays(1, &vertexArray);

		*this = GraphicModel{};
	}

	void SkyBoxLoaderAndDrawer::createGpuData(ErrorReporter &errorReporter, FileOpener &fileOpener, GLuint frameBuffer)
	{
		normalSkyBox.shader.loadShaderProgramFromFile("shaders/skyBox/skyBox.vert", "shaders/skyBox/skyBox.frag", errorReporter, fileOpener);
		normalSkyBox.samplerUniformLocation = getUniform(normalSkyBox.shader.id, "u_skybox", errorReporter);
		normalSkyBox.modelViewUniformLocation = getUniform(normalSkyBox.shader.id, "u_viewProjection", errorReporter);
		normalSkyBox.u_ambient = getUniform(normalSkyBox.shader.id, "u_ambient", errorReporter);
		normalSkyBox.u_skyBoxPresent = getUniform(normalSkyBox.shader.id, "u_skyBoxPresent", errorReporter);
		
		hdrtoCubeMap.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/hdrToCubeMap.frag", errorReporter, fileOpener);
		hdrtoCubeMap.u_equirectangularMap = getUniform(hdrtoCubeMap.shader.id, "u_equirectangularMap", errorReporter);
		hdrtoCubeMap.modelViewUniformLocation = getUniform(hdrtoCubeMap.shader.id, "u_viewProjection", errorReporter);

		convolute.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/convolute.frag", errorReporter, fileOpener);
		convolute.u_environmentMap = getUniform(convolute.shader.id, "u_environmentMap", errorReporter);
		convolute.modelViewUniformLocation = getUniform(convolute.shader.id, "u_viewProjection", errorReporter);
		convolute.u_sampleQuality = getUniform(convolute.shader.id, "u_sampleQuality", errorReporter);

		preFilterSpecular.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert", "shaders/skyBox/preFilterSpecular.frag", errorReporter, fileOpener);
		preFilterSpecular.modelViewUniformLocation = getUniform(preFilterSpecular.shader.id, "u_viewProjection", errorReporter);
		preFilterSpecular.u_environmentMap = getUniform(preFilterSpecular.shader.id, "u_environmentMap", errorReporter);
		preFilterSpecular.u_roughness = getUniform(preFilterSpecular.shader.id, "u_roughness", errorReporter);
		preFilterSpecular.u_sampleCount = getUniform(preFilterSpecular.shader.id, "u_sampleCount", errorReporter);

		atmosphericScatteringShader.shader.loadShaderProgramFromFile("shaders/skyBox/hdrToCubeMap.vert",
			"shaders/skyBox/atmosphericScattering.frag", errorReporter, fileOpener);
		atmosphericScatteringShader.u_lightPos = getUniform(atmosphericScatteringShader.shader.id, "u_lightPos", errorReporter);
		atmosphericScatteringShader.u_g = getUniform(atmosphericScatteringShader.shader.id, "u_g", errorReporter);
		atmosphericScatteringShader.u_color1 = getUniform(atmosphericScatteringShader.shader.id, "u_color1", errorReporter);
		atmosphericScatteringShader.u_color2 = getUniform(atmosphericScatteringShader.shader.id, "u_color2", errorReporter);
		atmosphericScatteringShader.modelViewUniformLocation 
			= getUniform(atmosphericScatteringShader.shader.id, "u_viewProjection", errorReporter);


		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);


		glBindVertexArray(0);

		glGenFramebuffers(1, &captureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO); //also allocate gpu resources
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	}

	void SkyBoxLoaderAndDrawer::loadTexture(const char *names[6], SkyBox &skyBox, ErrorReporter &errorReporter, FileOpener &fileOpener,
		GLuint frameBuffer)
	{
		skyBox = {};

		glGenTextures(1, &skyBox.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		for (unsigned int i = 0; i < 6; i++)
		{
			int w=0, h=0, nrChannels=0;
			unsigned char *data=0;

			stbi_set_flip_vertically_on_load(false);


			bool couldNotOpen = 0;
			auto content = fileOpener.binary(names[i], couldNotOpen);
			if (couldNotOpen)
			{
				errorReporter.callErrorCallback(std::string("Could not open file: ") + names[i]);
				glDeleteTextures(1, &skyBox.texture);
				return;
			}

			data = stbi_load_from_memory((unsigned char*)content.data(), content.size(), &w, &h, &nrChannels, 3);

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
				errorReporter.callErrorCallback(std::string("err loading ") + names[i]);
				glDeleteTextures(1, &skyBox.texture);
				return;
			}


		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		createConvolutedAndPrefilteredTextureData(skyBox, frameBuffer);
	}

	void SkyBoxLoaderAndDrawer::loadTexture(const char *name, SkyBox &skyBox, ErrorReporter &errorReporter, FileOpener &fileOpener,
		GLuint frameBuffer, int format)
	{
		skyBox = {};

		int width=0, height=0, nrChannels=0;
		unsigned char *data=0;

		stbi_set_flip_vertically_on_load(false);

		bool couldNotOpen = 0;
		auto content = fileOpener.binary(name, couldNotOpen);
		if (couldNotOpen)
		{
			errorReporter.callErrorCallback(std::string("Could not open file: ") + name);
			glDeleteTextures(1, &skyBox.texture);
			return;
		}

		data = stbi_load_from_memory((unsigned char *)content.data(), content.size(), &width, &height, &nrChannels, 3);

		if (!data) { errorReporter.callErrorCallback(std::string("err loading ") + name); return; }

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
			errorReporter.callErrorCallback(std::string("err loading ") + name);
		}


		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	
		createConvolutedAndPrefilteredTextureData(skyBox, frameBuffer);

	}

	void SkyBoxLoaderAndDrawer::loadHDRtexture(const char *name, ErrorReporter &errorReporter,
		FileOpener &fileOpener, SkyBox &skyBox, GLuint frameBuffer)
	{
		skyBox = {};

		int width=0, height=0, nrChannels=0;
		float *data=0;

		stbi_set_flip_vertically_on_load(true);
		
		bool couldNotOpen = 0;
		auto content = fileOpener.binary(name, couldNotOpen);
		if (couldNotOpen)
		{
			errorReporter.callErrorCallback(std::string("Could not open file: ") + name);
			glDeleteTextures(1, &skyBox.texture);
			return;
		}

		data = stbi_loadf_from_memory((unsigned char *)content.data(), content.size(), &width, &height, &nrChannels, 3);

		if (!data) { errorReporter.callErrorCallback(std::string("err loading ") + name); return; }


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
				glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

			}

			glDeleteFramebuffers(1, &captureFBO);

		}

		glDeleteTextures(1, &hdrTexture);

		createConvolutedAndPrefilteredTextureData(skyBox, frameBuffer);
	}

	void SkyBoxLoaderAndDrawer::atmosphericScattering(glm::vec3 sun, glm::vec3 color1, glm::vec3 color2, float g,
		SkyBox& skyBox, GLuint frameBuffer)
	{
		skyBox = {};
		constexpr int skyBoxSize = 128;

		//render into the cubemap
		{
			GLuint captureFBO; 
			glGenFramebuffers(1, &captureFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

			glGenTextures(1, &skyBox.texture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
			for (unsigned int i = 0; i < 6; ++i)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R11F_G11F_B10F,
					skyBoxSize, skyBoxSize, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//rendering
			{

				atmosphericScatteringShader.shader.bind();
				glUniform3fv(atmosphericScatteringShader.u_lightPos, 1, &sun[0]);
				glUniform1f(atmosphericScatteringShader.u_g, g);
				glUniform3fv(atmosphericScatteringShader.u_color1, 1, &color1[0]);
				glUniform3fv(atmosphericScatteringShader.u_color2, 1, &color2[0]);

				glViewport(0, 0, skyBoxSize, skyBoxSize);

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
				glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

			}

			glDeleteFramebuffers(1, &captureFBO);

		}

		createConvolutedAndPrefilteredTextureData(skyBox, 0.02, 64u);

	}

	void SkyBoxLoaderAndDrawer::createConvolutedAndPrefilteredTextureData(SkyBox &skyBox
		, GLuint frameBuffer, float sampleQuality, unsigned int specularSamples)
	{

		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		GLint viewPort[4] = {};
		glGetIntegerv(GL_VIEWPORT, viewPort);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	#pragma region convoluted texturelayout(binding = 4) uniform sampler2D u_roughness;


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
		glUniform1f(convolute.u_sampleQuality, sampleQuality);
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

			glDrawArrays(GL_TRIANGLES, 0, 6 * 6); // renders a 1x1 cube, todo refactor to draw only a face lol

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
			glUniform1ui(preFilterSpecular.u_sampleCount, specularSamples);

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

		glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

		//texture = convolutedTexture; //visualize convolutex texture
		//texture = preFilteredMap;

		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//todo delete mipmaps
		//GLuint newTexture = 0;
		//glGenTextures(1, &newTexture);
		//int w, h;
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		//glCopyTexImage2D(GL_TEXTURE_CUBE_MAP, 0, GL_SRGB, 0, 0, w, h, 0);

	}

	//deprecated
	void SkyBoxLoaderAndDrawer::draw(const glm::mat4 &viewProjMat, SkyBox &skyBox, float exposure,
		glm::vec3 ambient)
	{
		glBindVertexArray(vertexArray);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

		normalSkyBox.shader.bind();

		glUniformMatrix4fv(normalSkyBox.modelViewUniformLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniform1i(normalSkyBox.samplerUniformLocation, 0);
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
		glUniform3f(normalSkyBox.u_ambient, ambient.r, ambient.g, ambient.b);


		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(0);
	}

	void SkyBoxLoaderAndDrawer::clear()
	{
		normalSkyBox.shader.clear();
		hdrtoCubeMap.shader.clear();
		convolute.shader.clear();
		preFilterSpecular.shader.clear();
		atmosphericScatteringShader.shader.clear();

		glDeleteVertexArrays(1, &vertexArray);
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteFramebuffers(1, &captureFBO);

	}


	void SkyBox::clearTextures()
	{
		glDeleteTextures(3, (GLuint*)this);
		texture = 0;
		convolutedTexture = 0;
		preFilteredMap = 0;
	}

	//does not clear materials owned
	void CpuEntity::clear()
	{
		deleteGpuData();

		for (auto &n : subModelsNames)
		{
			delete[] n;
		}

		for (auto &m : models)
		{
			m.clear();
		}

		*this = CpuEntity();
	}

	void CpuEntity::allocateGpuData()
	{
		glGenBuffers(1, &appliedSkinningMatricesBuffer);
	}

	void CpuEntity::deleteGpuData()
	{
		glDeleteBuffers(1, &appliedSkinningMatricesBuffer);
	}

};
