#include "GraphicModel.h"
#include "Core.h"
#include "OBJ_Loader.h"
#include <stb_image.h>
#include "Texture.h"
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

		//todo if the object doesn't have texture data we should not render any material to it or just refuze to load it
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
		normalSkyBox.shader.loadShaderProgramFromFile("shaders/skyBox.vert", "shaders/skyBox.frag");
		normalSkyBox.samplerUniformLocation = getUniform(normalSkyBox.shader.id, "u_skybox");
		normalSkyBox.modelViewUniformLocation = getUniform(normalSkyBox.shader.id, "u_viewProjection");

		hdrtoCubeMap.shader.loadShaderProgramFromFile("shaders/hdrToCubeMap.vert", "shaders/hdrToCubeMap.frag");
		hdrtoCubeMap.u_equirectangularMap = getUniform(hdrtoCubeMap.shader.id, "u_equirectangularMap");
		hdrtoCubeMap.modelViewUniformLocation = getUniform(hdrtoCubeMap.shader.id, "u_viewProjection");

		convolute.shader.loadShaderProgramFromFile("shaders/hdrToCubeMap.vert", "shaders/convolute.frag");
		convolute.u_environmentMap = getUniform(convolute.shader.id, "u_environmentMap");
		convolute.modelViewUniformLocation = getUniform(convolute.shader.id, "u_viewProjection");

		preFilterSpecular.shader.loadShaderProgramFromFile("shaders/hdrToCubeMap.vert", "shaders/skyBox/preFilterSpecular.frag");
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
			}


		}

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		createConvolutedAndPrefilteredTextureData(skyBox);
	}

	void SkyBoxLoaderAndDrawer::loadTexture(const char *name, SkyBox &skyBox, int format)
	{
		int width, height, nrChannels;
		unsigned char *data;


		stbi_set_flip_vertically_on_load(false);
		data = stbi_load(name, &width, &height, &nrChannels, 3);

		if (!data) { return; }

		glGenTextures(1, &skyBox.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture);

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

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	//todo disable this after generated
		//the specular prefilter map
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	
		createConvolutedAndPrefilteredTextureData(skyBox);

	}

	void SkyBoxLoaderAndDrawer::loadHDRtexture(const char *name, SkyBox &skyBox)
	{
		int width, height, nrChannels;
		float *data;

		stbi_set_flip_vertically_on_load(true);
		data = stbi_loadf(name, &width, &height, &nrChannels, 0);
		if (!data) { return; }

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

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	//todo disable this after generated
		//the specular prefilter map

		//
		glDeleteTextures(1, &hdrTexture);

		createConvolutedAndPrefilteredTextureData(skyBox);
	}

	void SkyBoxLoaderAndDrawer::createConvolutedAndPrefilteredTextureData(SkyBox &skyBox)
	{

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
