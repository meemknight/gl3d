#include "gl3d.h"

#include <algorithm>
#include <stb_image.h>
#include <random>
#include <string>

#ifdef _MSC_VER
//#pragma warning( disable : 4244 4305 4267 4996 4018)
#pragma warning( disable : 26812)
#endif

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
		
		//unsigned char textureData[] =
		//{
		//	20, 20, 20, 255,
		//	212, 0, 219, 255,
		//	212, 0, 219, 255,
		//	20, 20, 20, 255,
		//};
		//defaultTexture.loadTextureFromMemory(textureData, 2, 2, 4, TextureLoadQuality::leastPossible);


		//create gBuffer
		glGenFramebuffers(1, &gBuffer.gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);

		glGenTextures(gBuffer.bufferCount, gBuffer.buffers);
		

		//todo refactor
		//todo glGetInternalFormativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_FORMAT, 1, &preferred_format).
		//https://www.khronos.org/opengl/wiki/Common_Mistakes#Extensions_and_OpenGL_Versions

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo], 0);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
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

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.emissive]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.emissive], 0);


		unsigned int attachments[decltype(gBuffer)::bufferCount] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, 
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
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
		directionalShadows.create();
		renderDepthMap.create();

		internal.pBRtextureMaker.init();
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

		int id = internal::generateNewIndex(internal.materialIndexes);

		GpuMaterial gpuMaterial;
		gpuMaterial.kd = glm::vec4(kd, 0);
		gpuMaterial.roughness = roughness;
		gpuMaterial.metallic = metallic;
		gpuMaterial.ao = ao;

		internal.materialIndexes.push_back(id);
		internal.materials.push_back(gpuMaterial);
		internal.materialNames.push_back(name);
		internal.materialTexturesData.push_back({});

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
		auto pos = std::find(internal.materialIndexes.begin(), internal.materialIndexes.end(), m.id_);

		if (pos == internal.materialIndexes.end())
		{
			gl3dAssertComment(pos != internal.materialIndexes.end(), "invalid delete material");
			return;
		}

		int index = pos - internal.materialIndexes.begin();

		internal.materialIndexes.erase(pos);
		internal.materials.erase(internal.materials.begin() + index);
		internal.materialNames.erase(internal.materialNames.begin() + index);
		internal.materialTexturesData.erase(internal.materialTexturesData.begin() + index);
		m.id_ = 0;
	}

	void Renderer3D::copyMaterialData(Material dest, Material source)
	{
		int destId = internal.getMaterialIndex(dest);
		int sourceId = internal.getMaterialIndex(source);

		if(destId == -1 || sourceId == -1)
		{
			gl3dAssertComment(destId != -1, "invaled dest material index");
			gl3dAssertComment(sourceId != -1, "invaled source material index");

			return;
		}

		internal.materials[destId] = internal.materials[sourceId];
		internal.materialNames[destId] = internal.materialNames[sourceId];
		internal.materialTexturesData[destId] = internal.materialTexturesData[destId];

	}

	GpuMaterial *Renderer3D::getMaterialData(Material m)
	{
		int id = internal.getMaterialIndex(m);

		if(id == -1)
		{
			return nullptr;
		}
		
		auto data = &internal.materials[id];

		return data;
	}

	TextureDataForModel *Renderer3D::getMaterialTextures(Material m)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &internal.materialTexturesData[id];

		return data;
	}

	std::string *Renderer3D::getMaterialName(Material m)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &internal.materialNames[id];

		return data;
	}

	bool Renderer3D::getMaterialData(Material m, GpuMaterial *gpuMaterial, std::string *name, TextureDataForModel *textureData)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return false;
		}

		if(gpuMaterial)
		{
			gpuMaterial = &internal.materials[id];
		}

		if(name)
		{
			name = &internal.materialNames[id];
		}

		if(textureData)
		{
			textureData = &internal.materialTexturesData[id];
		}

		return true;
	}

	bool Renderer3D::setMaterialData(Material m, const GpuMaterial &data, std::string *s)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return 0;
		}

		internal.materials[id] = data;
		
		if (s)
		{
			internal.materialNames[id] = *s;
		}

		return 1;
	}

	GpuMultipleGraphicModel *Renderer3D::getModelData(Model o)
	{
		int id = internal.getModelIndex(o);
	
		if (id == -1)
		{
			return nullptr;
		}
	
		auto data = &internal.graphicModels[id];
	
		return data;
	}

	Texture Renderer3D::loadTexture(std::string path)
	{

		if(path == "")
		{
			return Texture{ 0 };
		}

		int pos = 0;
		for (auto &i : internal.loadedTexturesNames)
		{
			if (i == path)
			{
				Texture t;
				t.id_ = internal.loadedTexturesIndexes[pos];
				return t;
			}
			pos++;
		}

		GpuTexture t;
		internal::GpuTextureWithFlags text;
		int alphaExists = t.loadTextureFromFileAndCheckAlpha(path.c_str());

		text.texture = t;
		text.flags = alphaExists;

		//if texture is not loaded, return an invalid texture
		if(t.id == 0)
		{
			return Texture{ 0 };
		}

		int id = internal::generateNewIndex(internal.loadedTexturesIndexes);


		internal.loadedTexturesIndexes.push_back(id);
		internal.loadedTextures.push_back(text);
		internal.loadedTexturesNames.push_back(path);

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
		int index = internal.getTextureIndex(t);

		if(index < 0)
		{
			return;
		}

		auto gpuTexture = internal.loadedTextures[index];

		internal.loadedTexturesIndexes.erase(internal.loadedTexturesIndexes.begin() + index);
		internal.loadedTextures.erase(internal.loadedTextures.begin() + index);
		internal.loadedTexturesNames.erase(internal.loadedTexturesNames.begin() + index);
		
		t.id_ = 0;

	}

	GpuTexture *Renderer3D::getTextureData(Texture t)
	{
		int id = internal.getTextureIndex(t);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &internal.loadedTextures[id];

		return &data->texture;
	}

	Texture Renderer3D::createIntenralTexture(GpuTexture t, int alphaData)
	{
		//if t is null return an empty texture
		if (t.id == 0)
		{
			Texture{ 0 };
		}

		int id = internal::generateNewIndex(internal.loadedTexturesIndexes);

		internal::GpuTextureWithFlags text;
		text.texture = t;
		text.flags= alphaData;

		internal.loadedTexturesIndexes.push_back(id);
		internal.loadedTextures.push_back(text);
		internal.loadedTexturesNames.push_back("");

		return Texture{ id };
	}

	Texture Renderer3D::createIntenralTexture(GLuint id_, int alphaData)
	{
		GpuTexture t;
		t.id = id_;
		return createIntenralTexture(t, alphaData);

	}

	static int max(int x, int y, int z)
	{
		return std::max(std::max(x, y), z);
	}

	Model Renderer3D::loadModel(std::string path, float scale)
	{

		gl3d::LoadedModelData model(path.c_str(), scale);
		if(model.loader.LoadedMeshes.empty())
		{
			std::cout << "err loading " + path + "\n";
			return { 0 };
		
		}

		int id = internal::generateNewIndex(internal.graphicModelsIndexes);
	
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
						textureData->albedoTexture = this->loadTexture(std::string(model.path + mat.map_Kd));
					}

					if (!mat.map_Kn.empty())
					{
						textureData->normalMapTexture = this->loadTexture(std::string(model.path + mat.map_Kn));
						//	TextureLoadQuality::linearMipmap);
					}

					if (!mat.map_emissive.empty())
					{
						textureData->emissiveTexture = this->loadTexture(std::string(model.path + mat.map_emissive));
					}

					textureData->RMA_loadedTextures = 0;

					auto rmaQuality = TextureLoadQuality::linearMipmap;

					if (!mat.map_RMA.empty()) 
					{
						//todo not tested
						//rmaQuality);

						textureData->RMA_Texture = this->loadTexture(mat.map_RMA.c_str());

						if (textureData->RMA_Texture.id_ != 0)
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
								t.loadTextureFromMemory(data, w, h, 4, rmaQuality); //todo 3 channels
								textureData->RMA_Texture = this->createIntenralTexture(t, 0);

								textureData->RMA_loadedTextures = 7; //all textures loaded

								stbi_image_free(data);
							}
						}


					}

					//RMA trexture
					if (textureData->RMA_loadedTextures == 0)
					{
						constexpr int MERGE_TEXTURES_ON_GPU = 1;

						if constexpr (MERGE_TEXTURES_ON_GPU)
						{

							GpuTexture roughness;
							int emptyData[1] = {};
							int roughnessLoaded = 0, metallicLoaded = 0, ambientLoaded = 0;

							if (!mat.map_Pr.empty())
							{
								roughness.loadTextureFromFile(std::string(model.path + mat.map_Pr).c_str(), dontSet, 1);
								glBindTexture(GL_TEXTURE_2D, roughness.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								roughnessLoaded = 1;
							}
							else
							{
								roughness.loadTextureFromMemory(emptyData, 1, 1, 1, dontSet);
								glBindTexture(GL_TEXTURE_2D, roughness.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							}

							GpuTexture metallic;
							if (!mat.map_Pm.empty())
							{
								metallic.loadTextureFromFile(std::string(model.path + mat.map_Pm).c_str(), dontSet, 1);
								glBindTexture(GL_TEXTURE_2D, metallic.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								metallicLoaded = 1;
							}
							else
							{
								metallic.loadTextureFromMemory(emptyData, 1, 1, 1, dontSet);
								glBindTexture(GL_TEXTURE_2D, metallic.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							}

							GpuTexture ambientOcclusion;
							if (!mat.map_Ka.empty())
							{
								ambientOcclusion.loadTextureFromFile(std::string(model.path + mat.map_Ka).c_str(), dontSet, 1);
								glBindTexture(GL_TEXTURE_2D, ambientOcclusion.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								ambientLoaded = 1;
							}
							else
							{
								ambientOcclusion.loadTextureFromMemory(emptyData, 1, 1, 1, dontSet);
								glBindTexture(GL_TEXTURE_2D, ambientOcclusion.id);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							}

							//calculate which function to use
							if (roughnessLoaded && metallicLoaded && ambientLoaded) { textureData->RMA_loadedTextures = 7; }
							else
							if (metallicLoaded && ambientLoaded) { textureData->RMA_loadedTextures = 6; }
							else
							if (roughnessLoaded && ambientLoaded) { textureData->RMA_loadedTextures = 5; }
							else
							if (roughnessLoaded && metallicLoaded) { textureData->RMA_loadedTextures = 4; }
							else
							if (ambientLoaded) { textureData->RMA_loadedTextures = 3; }
							else
							if (metallicLoaded) { textureData->RMA_loadedTextures = 2; }
							else
							if (roughnessLoaded) { textureData->RMA_loadedTextures = 1; }
							else { textureData->RMA_loadedTextures = 0; }

							auto t = internal.pBRtextureMaker.createRMAtexture(1024, 1024,
								roughness, metallic, ambientOcclusion, lightShader.quadDrawer.quadVAO);

							textureData->RMA_Texture = this->createIntenralTexture(t, 0);

							roughness.clear();
							metallic.clear();
							ambientOcclusion.clear();
						}
						else 
						{
							stbi_set_flip_vertically_on_load(true);

							int w1 = 0, h1 = 0;
							unsigned char* data1 = 0;
							unsigned char* data2 = 0;
							unsigned char* data3 = 0;

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

								unsigned char* finalData = new unsigned char[w * h * 4];

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
								textureData->RMA_Texture = this->createIntenralTexture(t, 0);

								stbi_image_free(data1);
								stbi_image_free(data2);
								stbi_image_free(data3);
								delete[] finalData;

							}
						}
					

						/*
						
						*/

					}





					
				}

				loadedMaterials.push_back(m);
			}


			for (int i = 0; i < s; i++)
			{
				GpuGraphicModel gm;
				int index = i;
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

		
		internal.graphicModelsIndexes.push_back(id);
		internal.graphicModels.push_back(returnModel);


		Model o;
		o.id_ = id;
		return o;

	}

	void Renderer3D::deleteModel(Model o)
	{
		auto pos = internal.getModelIndex(o);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete model");
			return;
		}

		internal.graphicModelsIndexes.erase(internal.graphicModelsIndexes.begin() + pos);
		internal.graphicModels[pos].clear();
		internal.graphicModels.erase(internal.graphicModels.begin() + pos);

	}

	Entity Renderer3D::createEntity(Model m, Transform transform)
	{
		int id = internal::generateNewIndex(internal.entitiesIndexes);

		CpuEntity entity;
		entity.model = m;
		entity.transform = transform;

		internal.entitiesIndexes.push_back(id);
		internal.cpuEntities.push_back(entity);

		Entity e;
		e.id_ = id;
		return e;
	}

	CpuEntity* Renderer3D::getEntityData(Entity e)
	{
		auto i = internal.getEntityIndex(e);

		if (i < 0) { return nullptr; }

		return &internal.cpuEntities[i];

	}

	void Renderer3D::deleteEntity(Entity e)
	{
		auto pos = internal.getEntityIndex(e);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete entity");
			return;
		}

		internal.entitiesIndexes.erase(internal.entitiesIndexes.begin() + pos);
		internal.cpuEntities.erase(internal.cpuEntities.begin() + pos);
		
	}

	//todo look into  glProgramUniform
	//in order to send less stuff tu uniforms

	//todo look into
	//ATI/AMD created GL_ATI_meminfo. This extension is very easy to use. 
	//You basically need to call glGetIntegerv with the appropriate token values.
	//https://www.khronos.org/registry/OpenGL/extensions/ATI/ATI_meminfo.txt
	//http://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt

	//deprecated
	void Renderer3D::renderModel(Model o, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);

		auto found = std::find(internal.graphicModelsIndexes.begin(), internal.graphicModelsIndexes.end(), o.id_);
		if (found == internal.graphicModelsIndexes.end())
		{
			gl3dAssertComment(found != internal.graphicModelsIndexes.end(), "invalid render object");
			return;
		}
		int id = found - internal.graphicModelsIndexes.begin();

		auto &model = internal.graphicModels[id];


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
		glUniform1i(lightShader.u_emissiveTexture, 4);



		//material buffer
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightShader.materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuMaterial) * internal.materials.size()
			, &internal.materials[0], GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightShader.materialBlockBuffer);

		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//glDepthFunc(GL_LESS);
		//for (auto &i : model.models)
		//{
		//	
		//	glBindVertexArray(i.vertexArray);
		//	//glBindBuffer(GL_ARRAY_BUFFER, i.vertexBuffer);
		//
		//	if (i.indexBuffer)
		//	{
		//		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i.indexBuffer);
		//		glDrawElements(GL_TRIANGLES, i.primitiveCount, GL_UNSIGNED_INT, 0);
		//	}
		//	else
		//	{
		//		glDrawArrays(GL_TRIANGLES, 0, i.primitiveCount);
		//	}
		//}
		
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//glDepthFunc(GL_EQUAL);

		GLsizei n;
		glGetProgramStageiv(lightShader.geometryPassShader.id,
		GL_FRAGMENT_SHADER,
		GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
		&n);

		GLuint *indices = new GLuint[n]{ 0 };
		bool changed = 1;	
	
		for (auto &i : model.models)
		{
			
			int materialId = internal.getMaterialIndex(i.material);

			if (materialId == -1)
				{ continue; }

			glUniform1i(lightShader.materialIndexLocation, materialId);
			

			TextureDataForModel textureData = internal.materialTexturesData[materialId];

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
			if(normalMapTextureData != nullptr && normalMapTextureData->id != 0)
			{
				normalLoaded = 1;
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, normalMapTextureData->id);
			}
		
			GpuTexture *rmaTextureData = this->getTextureData(textureData.RMA_Texture);
			if(rmaTextureData != nullptr && rmaTextureData->id != 0)
			{
				rmaLoaded = 1;
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, rmaTextureData->id);
			}

			int emissiveTextureLoaded = 0;
			GpuTexture* emissiveTextureData = this->getTextureData(textureData.emissiveTexture);
			if(emissiveTextureData != nullptr && emissiveTextureData->id != 0)
			{
				emissiveTextureLoaded = 1;
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, emissiveTextureData->id);
			}


			if(emissiveTextureLoaded)
			{
				if (indices[lightShader.getEmmisiveSubroutineLocation] != lightShader.emissiveSubroutine_sampled)
				{
					indices[lightShader.getEmmisiveSubroutineLocation] = lightShader.emissiveSubroutine_sampled;
					changed = 1;
				}
			}
			else
			{
				if (indices[lightShader.getEmmisiveSubroutineLocation] != lightShader.emissiveSubroutine_notSampled)
				{
					indices[lightShader.getEmmisiveSubroutineLocation] = lightShader.emissiveSubroutine_notSampled;
					changed = 1;
				}
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

	void Renderer3D::renderModelNormals(Model o, glm::vec3 position, glm::vec3 rotation,
		glm::vec3 scale, float normalSize, glm::vec3 normalColor)
	{
		auto obj = getModelData(o);

		if(!obj)
		{
			return;
		}

		for(int i=0; i<obj->models.size(); i++)
		{
			renderSubModelNormals(o, i, position, rotation, scale, normalSize, normalColor);
		}
		
	}

	void Renderer3D::renderSubModelNormals(Model o, int index, glm::vec3 position, glm::vec3 rotation,
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

		auto modelIndex = this->internal.getModelIndex(o);

		auto obj = getModelData(o);
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

	void Renderer3D::renderSubModelBorder(Model o, int index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, float borderSize, glm::vec3 borderColor)
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

	int Renderer3D::InternalStruct::getMaterialIndex(Material m)
	{
		int id = m.id_;
		auto found = std::find(materialIndexes.begin(), materialIndexes.end(), id);
		if (found == materialIndexes.end())
		{
			gl3dAssertComment(found != materialIndexes.end(), "invalid material");
			return -1;
		}
		id = found - materialIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getModelIndex(Model o)
	{
		int id = o.id_;
		auto found = std::find(graphicModelsIndexes.begin(), graphicModelsIndexes.end(), id);
		if (found == graphicModelsIndexes.end())
		{
			gl3dAssertComment(found != graphicModelsIndexes.end(), "invalid object");
			return -1;
		}
		id = found - graphicModelsIndexes.begin();
	
		return id;
	}

	int Renderer3D::InternalStruct::getTextureIndex(Texture t)
	{
		int id = t.id_;
		if (id == 0) { return -1; }

		auto found = std::find(loadedTexturesIndexes.begin(), loadedTexturesIndexes.end(), id);
		if (found == loadedTexturesIndexes.end())
		{
			gl3dAssertComment(found != loadedTexturesIndexes.end(), "invalid texture");
			return -1;
		}
		id = found - loadedTexturesIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getEntityIndex(Entity t)
	{
		int id = t.id_;
		if (id == 0) { return -1; }

		auto found = std::find(entitiesIndexes.begin(), entitiesIndexes.end(), id);
		if (found == entitiesIndexes.end())
		{
			gl3dAssertComment(found != entitiesIndexes.end(), "invalid entity");
			return -1;
		}
		id = found - entitiesIndexes.begin();

		return id;
	}

	//todo add to other projects and places
	glm::mat4 lookAtSafe(glm::vec3 const& eye, glm::vec3 const& center, glm::vec3 const& upVec)
	{
		glm::vec3 up = glm::normalize(upVec);

		glm::vec3 f;
		glm::vec3 s;
		glm::vec3 u;

		f = (normalize(center - eye));
		if (f == up || f == -up)
		{
			s = glm::vec3(up.z, up.x, up.y);
			u = (cross(s, f));

		}
		else
		{
			s = (normalize(cross(f, up)));
			u = (cross(s, f));
		}
		
		glm::mat4 Result(1);
		Result[0][0] = s.x;
		Result[1][0] = s.y;
		Result[2][0] = s.z;
		Result[0][1] = u.x;
		Result[1][1] = u.y;
		Result[2][1] = u.z;
		Result[0][2] = -f.x;
		Result[1][2] = -f.y;
		Result[2][2] = -f.z;
		Result[3][0] = -dot(s, eye);
		Result[3][1] = -dot(u, eye);
		Result[3][2] = dot(f, eye);
		return Result;
	}

	void Renderer3D::render()
	{
		glStencilMask(0xFF);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDepthFunc(GL_LESS);


		renderSkyBoxBefore();


		lightShader.prePass.shader.bind();


		#pragma region render shadow maps
		if (directionalLights.size())
		{

			glViewport(0, 0, directionalShadows.shadowSize, directionalShadows.shadowSize);
			glBindFramebuffer(GL_FRAMEBUFFER, directionalShadows.depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			
			glm::vec3 lightDir = directionalLights[0].direction;
			glm::vec3 lightPosition = -lightDir * glm::vec3(11);

			float near_plane = 1.f, far_plane = 26.f;
			glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
			glm::mat4 lightView = lookAtSafe(lightPosition, { 0.f,0.f,0.f }, { 0.f,1.f,0.f });

			glm::mat4 lightSpaceMatrix = lightProjection * lightView;

			directionalLights[0].lightSpaceMatrix = lightSpaceMatrix;

			//render shadow of the models
			for (auto& i : internal.cpuEntities)
			{
				auto id = internal.getModelIndex(i.model.id_);
				if (id < 0)
				{
					continue;
				}
				
				auto& model = internal.graphicModels[id];
				auto transformMat = i.transform.getTransformMatrix();
				auto modelViewProjMat = lightSpaceMatrix * transformMat;

				glUniformMatrix4fv(lightShader.prePass.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);

				for (auto& i : model.models)
				{
					auto m = internal.getMaterialIndex(i.material);

					if (m < 0)
					{
						glUniform1i(lightShader.prePass.u_hasTexture, 0);
					}
					else
					{

						auto t = internal.materialTexturesData[m];
						auto tId = internal.getTextureIndex(t.albedoTexture);

						if (tId < 0)
						{
							glUniform1i(lightShader.prePass.u_hasTexture, 0);
						}
						else
						{
							auto texture = internal.loadedTextures[tId];

							glUniform1i(lightShader.prePass.u_hasTexture, 1);
							glUniform1i(lightShader.prePass.u_albedoSampler, 0);
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, texture.texture.id);
						}
					}

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
			


			glViewport(0, 0, w, h);
		}

		#pragma endregion


		#pragma region stuff to be bound for rendering the pre pass geometry

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);


		#pragma endregion



		#pragma region z pre pass
		for (auto& i : internal.cpuEntities)
		{
			auto id = internal.getModelIndex(i.model.id_);
			if (id < 0)
			{
				continue;
			}

			auto& model = internal.graphicModels[id];
			auto projMat = camera.getProjectionMatrix();
			auto viewMat = camera.getWorldToViewMatrix();
			auto transformMat = i.transform.getTransformMatrix();
			auto modelViewProjMat = projMat * viewMat * transformMat;
			glUniformMatrix4fv(lightShader.prePass.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);

			for (auto &i : model.models)
			{
				auto m = internal.getMaterialIndex(i.material);

				if (m < 0)
				{
					glUniform1i(lightShader.prePass.u_hasTexture, 0);
				}
				else
				{

					auto t = internal.materialTexturesData[m];
					auto tId = internal.getTextureIndex(t.albedoTexture);

					if (tId < 0)
					{
						glUniform1i(lightShader.prePass.u_hasTexture, 0);

					}
					else
					{
						auto texture = internal.loadedTextures[tId];

						glUniform1i(lightShader.prePass.u_hasTexture, 1);
						glUniform1i(lightShader.prePass.u_albedoSampler, 0);
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, texture.texture.id);
					}

				}

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
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		#pragma endregion 




		#pragma region stuff to be bound for rendering the geometry


		lightShader.geometryPassShader.bind();
		lightShader.getSubroutines();

		//glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		//glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(lightShader.textureSamplerLocation, 0);
		glUniform1i(lightShader.normalMapSamplerLocation, 1);
		//glUniform1i(lightShader.skyBoxSamplerLocation, 2);
		glUniform1i(lightShader.RMASamplerLocation, 3);
		glUniform1i(lightShader.u_emissiveTexture, 4);


		//material buffer
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightShader.materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuMaterial) * internal.materials.size()
			, &internal.materials[0], GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightShader.materialBlockBuffer);

		GLsizei n;
		glGetProgramStageiv(lightShader.geometryPassShader.id,
			GL_FRAGMENT_SHADER,
			GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
			&n);

		GLuint* indices = new GLuint[n]{ 0 };


		glDepthFunc(GL_EQUAL);

		#pragma endregion


		//first we render the entities in the gbuffer
		for (auto& i : internal.cpuEntities)
		{
			//renderModel(i.model, i.transform.position, i.transform.rotation, i.transform.scale);

			auto id = internal.getModelIndex(i.model.id_);
			if (id < 0) 
			{ continue; }

			auto& model = internal.graphicModels[id];

			if (model.models.empty())
			{
				continue;
			}

			auto projMat = camera.getProjectionMatrix();
			auto viewMat = camera.getWorldToViewMatrix();
			auto transformMat = i.transform.getTransformMatrix();
			auto modelViewProjMat = projMat * viewMat * transformMat;

			glUniformMatrix4fv(lightShader.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);
			glUniformMatrix4fv(lightShader.u_modelTransform, 1, GL_FALSE, &transformMat[0][0]);
			glUniformMatrix4fv(lightShader.u_motelViewTransform, 1, GL_FALSE, &(viewMat * transformMat)[0][0]);
			
			
			bool changed = 1;

			for (auto& i : model.models)
			{

				int materialId = internal.getMaterialIndex(i.material);

				if (materialId == -1)
				{
					continue;
				}

				glUniform1i(lightShader.materialIndexLocation, materialId);


				TextureDataForModel textureData = internal.materialTexturesData[materialId];

				int rmaLoaded = 0;
				int albedoLoaded = 0;
				int normalLoaded = 0;

				GpuTexture* albedoTextureData = this->getTextureData(textureData.albedoTexture);
				if (albedoTextureData != nullptr)
				{
					albedoLoaded = 1;
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, albedoTextureData->id);
				}

				GpuTexture* normalMapTextureData = this->getTextureData(textureData.normalMapTexture);
				if (normalMapTextureData != nullptr && normalMapTextureData->id != 0)
				{
					normalLoaded = 1;
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, normalMapTextureData->id);
				}

				GpuTexture* rmaTextureData = this->getTextureData(textureData.RMA_Texture);
				if (rmaTextureData != nullptr && rmaTextureData->id != 0)
				{
					rmaLoaded = 1;
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, rmaTextureData->id);
				}

				int emissiveTextureLoaded = 0;
				GpuTexture* emissiveTextureData = this->getTextureData(textureData.emissiveTexture);
				if (emissiveTextureData != nullptr && emissiveTextureData->id != 0)
				{
					emissiveTextureLoaded = 1;
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, emissiveTextureData->id);
				}


				if (emissiveTextureLoaded)
				{
					if (indices[lightShader.getEmmisiveSubroutineLocation] != lightShader.emissiveSubroutine_sampled)
					{
						indices[lightShader.getEmmisiveSubroutineLocation] = lightShader.emissiveSubroutine_sampled;
						changed = 1;
					}
				}
				else
				{
					if (indices[lightShader.getEmmisiveSubroutineLocation] != lightShader.emissiveSubroutine_notSampled)
					{
						indices[lightShader.getEmmisiveSubroutineLocation] = lightShader.emissiveSubroutine_notSampled;
						changed = 1;
					}
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

				if (rmaLoaded)
				{

					if (indices[lightShader.materialSubroutineLocation] != lightShader.materialSubroutine_functions[textureData.RMA_loadedTextures])
					{
						indices[lightShader.materialSubroutineLocation] = lightShader.materialSubroutine_functions[textureData.RMA_loadedTextures];
						changed = 1;
					}

				}
				else
				{
					if (indices[lightShader.materialSubroutineLocation] != lightShader.materialSubroutine_functions[0])
					{
						indices[lightShader.materialSubroutineLocation] = lightShader.materialSubroutine_functions[0];
						changed = 1;
					}

				}


				if (albedoLoaded != 0)
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



		}

		delete[] indices;

		glBindVertexArray(0);

		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		//we draw a rect several times so we keep this vao binded
		glBindVertexArray(lightShader.quadDrawer.quadVAO);
		
		if(lightShader.useSSAO)
		{
		#pragma region ssao
			glViewport(0, 0, w / 2, h / 2);

			glUseProgram(ssao.shader.id);

			glBindBuffer(GL_UNIFORM_BUFFER, ssao.ssaoUniformBlockBuffer);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SSAO::SsaoShaderUniformBlockData),
				&ssao.ssaoShaderUniformBlockData);

			glUniformMatrix4fv(ssao.u_projection, 1, GL_FALSE,
				&(camera.getProjectionMatrix())[0][0]);

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
			glViewport(0, 0, w / 4, h / 4);

			glBindFramebuffer(GL_FRAMEBUFFER, ssao.blurBuffer);
			ssao.blurShader.bind();
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ssao.ssaoColorBuffer);
			glUniform1i(ssao.u_ssaoInput, 0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glViewport(0, 0, w, h);
		#pragma endregion
		}
	

	#pragma region do the lighting pass

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


		glUniform1i(lightShader.light_u_skyboxFiltered, 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.preFilteredMap);

		glUniform1i(lightShader.light_u_skyboxIradiance, 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.convolutedTexture);

		glUniform1i(lightShader.light_u_brdfTexture, 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, lightShader.brdfTexture.id);

		glUniform1i(lightShader.light_u_emmisive, 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.emissive]);

		glUniform1i(lightShader.light_u_directionalShadow, 8);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, directionalShadows.depthMapTexture);


		glUniform3f(lightShader.light_u_eyePosition, camera.position.x, camera.position.y, camera.position.z);

		glUniformMatrix4fv(lightShader.light_u_view, 1, GL_FALSE, &(camera.getWorldToViewMatrix()[0][0]) );

		if (pointLights.size())
		{//todo laziness if lights don't change and stuff
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightShader.pointLightsBlockBuffer);
		
			glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(internal::GpuPointLight)
				, &pointLights[0], GL_STREAM_DRAW);
		
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightShader.pointLightsBlockBuffer);
		
		}
		glUniform1i(lightShader.light_u_pointLightCount, pointLights.size());

		if (directionalLights.size())
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightShader.directionalLightsBlockBuffer);

			glBufferData(GL_SHADER_STORAGE_BUFFER, directionalLights.size() * sizeof(internal::GpuDirectionalLight)
				, &directionalLights[0], GL_STREAM_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightShader.directionalLightsBlockBuffer);

		}
		glUniform1i(lightShader.light_u_directionalLightCount, directionalLights.size());


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

	#pragma region do the post process stuff and draw to the screen

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

			if (lightShader.bloomBlurPasses <= 0)
			{
				glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[1]);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, postProcess.bluredColorBuffer[1]);
			}

			glUniform1f(postProcess.u_bloomIntensity, postProcess.bloomIntensty);

		}else
		{
			glUniform1f(postProcess.u_bloomIntensity, 0);

			//todo uniform block for this and also probably boolean to check if using bloom or not
			glBindTexture(GL_TEXTURE_2D, 0);

		}

		if (lightShader.useSSAO)
		{
			glUniform1i(postProcess.u_useSSAO, 1);
			//todo change ssao_finalColor_exponent
			glUniform1f(postProcess.u_ssaoExponent, ssao_finalColor_exponent);
			
			
			glUniform1i(postProcess.u_ssao, 3);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ssao.blurColorBuffer);
			

		}else
		{
			glUniform1i(postProcess.u_useSSAO, 0);

		}

		glUniform1i(postProcess.u_bloomNotBluredTexture, 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[1]);

		
		glUniform1f(postProcess.u_exposure, this->exposure);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisable(GL_BLEND);


	#pragma endregion

	#pragma region copy depth buffer for later forward rendering
		glBindVertexArray(0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		glBlitFramebuffer(
		  0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);

		
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.positionViewSpace]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.emissive]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

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


		//bloom buffer and color buffer
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

	void Renderer3D::renderADepthMap(GLuint texture)
	{
		glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, renderDepthMap.fbo);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


		renderDepthMap.shader.bind();
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, 1024, 1024);

		glBindVertexArray(lightShader.quadDrawer.quadVAO);
		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(renderDepthMap.u_depth, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glViewport(0, 0, w, h);

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glEnable(GL_DEPTH_TEST);

	}

	void Renderer3D::renderSkyBox()
	{
		//todo move into render
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		viewMat = glm::mat4(glm::mat3(viewMat));

		auto viewProjMat = projMat * viewMat;

		internal.skyBoxLoaderAndDrawer.draw(viewProjMat, skyBox, this->exposure,
			lightShader.lightPassUniformBlockCpuData.ambientLight);
	}

	void Renderer3D::renderSkyBoxBefore()
	{
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		viewMat = glm::mat4(glm::mat3(viewMat));

		auto viewProjMat = projMat * viewMat;

		internal.skyBoxLoaderAndDrawer.drawBefore(viewProjMat, skyBox, this->exposure,
			lightShader.lightPassUniformBlockCpuData.ambientLight);
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

	SkyBox Renderer3D::atmosfericScattering(glm::vec3 sun, float g, float g2)
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.atmosphericScattering(sun, g, g2, skyBox);
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
		u_bloomNotBluredTexture = getUniform(postProcessShader.id, "u_bloomNotBluredTexture");
		u_bloomIntensity = getUniform(postProcessShader.id, "u_bloomIntensity");
		u_exposure = getUniform(postProcessShader.id, "u_exposure");

		u_useSSAO = getUniform(postProcessShader.id, "u_useSSAO");
		u_ssaoExponent = getUniform(postProcessShader.id, "u_ssaoExponent");
		u_ssao = getUniform(postProcessShader.id, "u_ssao");


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

	void Renderer3D::InternalStruct::PBRtextureMaker::init()
	{
		shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/modelLoader/mergePBRmat.frag");
		glGenFramebuffers(1, &fbo);


	}

	GLuint Renderer3D::InternalStruct::PBRtextureMaker::createRMAtexture(int w, int h, GpuTexture roughness, 
		GpuTexture metallic, GpuTexture ambientOcclusion, GLuint quadVAO)
	{

		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
		GLuint texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//todo set the quality of this texture in a function parameter.
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	

		glBindVertexArray(quadVAO);

		shader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, roughness.id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, metallic.id);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ambientOcclusion.id);


		glViewport(0, 0, w, h);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return texture;
	}

	void Renderer3D::DirectionalShadows::create()
	{
		glGenFramebuffers(1, &depthMapFBO);


		glGenTextures(1, &depthMapTexture);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowSize, shadowSize, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		
	}

	void Renderer3D::RenderDepthMap::create()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "renderdepth map frame buffer failed\n";
		}


		shader.loadShaderProgramFromFile
			("shaders/drawQuads.vert", "shaders/drawDepth.frag");
		u_depth = getUniform(shader.id, "u_depth");
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

};