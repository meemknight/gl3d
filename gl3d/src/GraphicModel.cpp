#include "GraphicModel.h"
#include "Core.h"
#include <OBJ_Loader.h>
#include <stb_image.h>
#include "Texture.h"

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

	void GraphicModel::loadFromModelMeshIndex(const LoadedModelData &model, int index)
	{
		auto &mesh = model.loader.LoadedMeshes[index];
		loadFromComputedData(mesh.Vertices.size() * 8 * 4,
			 (float *)&mesh.Vertices[0],
			mesh.Indices.size() * 4, &mesh.Indices[0]);


		auto &mat = model.loader.LoadedMeshes[index].MeshMaterial;

		material.kd = glm::vec4(glm::vec3(mat.Kd), 1);
		//material.ks = glm::vec4(glm::vec3(mat.Ks), mat.Ns);
		//material.ka = glm::vec4(glm::vec3(mat.Ka), 0);
		material.metallic = mat.metallic;
		material.roughness = mat.roughness;
		//material.ao = mat.ao;

		material.setDefaultMaterial();

		albedoTexture.clear();
		normalMapTexture.clear();

		if (!mat.map_Kd.empty())
		{
			albedoTexture.loadTextureFromFile(std::string(model.path + mat.map_Kd).c_str());
		}

		if (!mat.map_Kn.empty())
		{
			normalMapTexture.loadTextureFromFile(std::string(model.path + mat.map_Kn).c_str());
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

};
