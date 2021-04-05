#include "gl3d.h"

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
		skyBox.createGpuData();

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.position], 0);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal], 0);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo], 0);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material], 0);

		unsigned int attachments[decltype(gBuffer)::bufferCount] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
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

				gm.material = loadedMaterials[model.loader.LoadedMeshes[index].materialIndex];
				
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
		auto transformMat = gl3d::getTransformMatrix(position, rotation, scale);
	
		auto modelViewProjMat = projMat * viewMat * transformMat;
		//auto modelView = viewMat * transformMat;
	
		lightShader.geometryPassShader.bind();
	

		lightShader.getSubroutines();


		glUniformMatrix4fv(lightShader.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);
		glUniformMatrix4fv(lightShader.u_modelTransform, 1, GL_FALSE, &transformMat[0][0]);
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


			//glActiveTexture(GL_TEXTURE2);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture); //note(vlod): this can be bound onlt once (refactor)

	
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
				glBindVertexArray(0);
			}
	
		}
	
		delete[] indices;
	
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
		//we draw a rect several times
		glBindVertexArray(lightShader.quadVAO);
		glUseProgram(ssao.shader.id);

		glUniformMatrix4fv(ssao.u_projection, 1, GL_FALSE,
			&(camera.getProjectionMatrix())[0][0] );

		glUniformMatrix4fv(ssao.u_view, 1, GL_FALSE,
			&(camera.getWorldToViewMatrix())[0][0]);


		glUniform3fv(ssao.u_samples, 64, &(ssao.ssaoKernel[0][0]));

		glBindFramebuffer(GL_FRAMEBUFFER, ssao.ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.position]);
		glUniform1i(ssao.u_gPosition, 0);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);
		glUniform1i(ssao.u_gNormal, 1);


		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ssao.noiseTexture);
		glUniform1i(ssao.u_texNoise, 2);


		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
		////

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


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
		glBindTexture(GL_TEXTURE_2D, ssao.ssaoColorBuffer);


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


		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		glBlitFramebuffer(
		  0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void Renderer3D::updateWindowMetrics(int x, int y)
	{
		w = x; h = y;

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.position]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.normal]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, x, y, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.albedo]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, gBuffer.buffers[gBuffer.material]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindRenderbuffer(GL_RENDERBUFFER, gBuffer.depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, x, y);


	}

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer3D::SSAO::create(int w, int h)
	{
		std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f); 
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);


		shader.loadShaderProgramFromFile("shaders/ssao/ssao.vert", "shaders/ssao/ssao.frag");


		u_projection = getUniform(shader.id, "u_projection");
		u_view = getUniform(shader.id, "u_view");
		u_gPosition = getUniform(shader.id, "u_gPosition");
		u_gNormal = getUniform(shader.id, "u_gNormal");
		u_texNoise = getUniform(shader.id, "u_texNoise");
		u_samples = getUniform(shader.id, "samples[0]");
		

	}

};