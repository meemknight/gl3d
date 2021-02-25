#include "gl3d.h"

#include <algorithm>
#include <stb_image.h>

namespace gl3d
{
	void renderLightModel(GraphicModel &model, Camera camera, glm::vec3 lightPos, LightShader lightShader, Texture texture, Texture normalTexture, GLuint skyBoxTexture, 
		float gama, const GpuMaterial &material, std::vector<internal::GpuPointLight> &pointLights)
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
		lightShader.setData(modelViewProjMat, transformMat, lightPos, camera.position, gama, GpuMaterial(),
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
				if (indices[lightShader.normalSubroutineLocation] != lightShader.normalSubroutine_noMap)
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
		skyBox.createGpuData();

	}

	Material Renderer3D::createMaterial(glm::vec3 kd, float roughness, float metallic, float ao
	, std::string name)
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

		GpuMaterial gpuMaterial;
		gpuMaterial.kd = glm::vec4(kd, 0);
		gpuMaterial.roughness = roughness;
		gpuMaterial.metallic = metallic;
		gpuMaterial.ao = ao;

		materialIndexes.push_back(id);
		materials.push_back(gpuMaterial);
		materialNames.push_back(name);

		Material m;
		m._id = id;
		return m;

	}

	Material Renderer3D::createMaterial(Material m)
	{
		auto newM = createMaterial();
		copyMaterial(newM, m);

		return newM;
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
		materialNames.erase(materialNames.begin() + index);
	}

	void Renderer3D::copyMaterial(Material dest, Material source)
	{
		int destId = getMaterialIndex(dest);
		int sourceId = getMaterialIndex(source);

		gl3dAssertComment(destId != -1, "invaled dest material index");
		gl3dAssertComment(sourceId != -1, "invaled source material index");

		materials[destId] = materials[sourceId];
		materialNames[destId] = materialNames[sourceId];

	}

	GpuMaterial *Renderer3D::getMaterialData(Material m, std::string *s)
	{
		int id = getMaterialIndex(m);

		if(id == -1)
		{
			return nullptr;
		}
		
		auto data = &materials[id];

		if(s)
		{
			*s = materialNames[id];
		}

		return data;
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


			std::vector<gl3d::Material> loadedMaterials;
			loadedMaterials.reserve(model.loader.LoadedMaterials.size());
			for(int i=0;i<model.loader.LoadedMaterials.size(); i++)
			{
				auto &mat = model.loader.LoadedMaterials[i];
				auto m = this->createMaterial(mat.Kd, mat.roughness,
				mat.metallic /*,mat.ao todo*/);

				loadedMaterials.push_back(m);
			}


			for (int i = 0; i < s; i++)
			{
				GpuGraphicModel gm;
				int index = i;
				GpuMaterial material;

				{
					auto &mesh = model.loader.LoadedMeshes[index];
					gm.loadFromComputedData(mesh.Vertices.size() * 8 * 4,
						 (float *)&mesh.Vertices[0],
						mesh.Indices.size() * 4, &mesh.Indices[0]);

					auto &mat = model.loader.LoadedMeshes[index].MeshMaterial;
					gm.material = loadedMaterials[model.loader.LoadedMeshes[index].materialIndex];

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
	
		//todo refactor or remove light shader

		lightShader.getSubroutines();
		lightShader.setData(modelViewProjMat, transformMat, {}, camera.position, 2.2, GpuMaterial(),
			pointLights);

		//material buffer
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightShader.materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuMaterial) * materials.size()
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
				int id = getMaterialIndex(i.material);

				if (id == -1) 
					{ continue; }

				glUniform1i(lightShader.materialIndexLocation, id);
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, i.albedoTexture.id);
	
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, i.normalMapTexture.id);
	
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.texture); //note(vlod): this can be bound onlt once (refactor)
	
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, i.RMA_Texture.id);
	
	
			if (i.normalMapTexture.id && lightShader.normalMap)
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
	
			if (indices[lightShader.materialSubroutineLocation] != lightShader.materialSubroutine_functions[i.RMA_loadedTextures])
			{
				indices[lightShader.materialSubroutineLocation] = lightShader.materialSubroutine_functions[i.RMA_loadedTextures];
				changed = 1;
			}
			
			if(i.albedoTexture.id != 0)
			{
				if (indices[lightShader.getAlbedoSubroutineLocation] != lightShader.albedoSubroutine_sampled)
				{
					indices[lightShader.getAlbedoSubroutineLocation] = lightShader.albedoSubroutine_sampled;
					changed = 1;
				}else
				if (indices[lightShader.getAlbedoSubroutineLocation] != lightShader.albedoSubroutine_notSampled)
				{
					indices[lightShader.getAlbedoSubroutineLocation] = lightShader.albedoSubroutine_notSampled;
					changed = 1;
				}
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
	
	
	}

	int Renderer3D::getMaterialIndex(Material m)
	{
		int id = m._id;
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
		int id = o._id;
		auto found = std::find(graphicModelsIndexes.begin(), graphicModelsIndexes.end(), id);
		if (found == graphicModelsIndexes.end())
		{
			gl3dAssertComment(found == graphicModelsIndexes.end(), "invalid object");
			return -1;
		}
		id = found - graphicModelsIndexes.begin();
	
		return id;
	}

};