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
		internal.w = x; internal.h = y;
		internal.adaptiveW = x;
		internal.adaptiveH = y;

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		internal.lightShader.create();
		vao.createVAOs();
		internal.skyBoxLoaderAndDrawer.createGpuData();


		internal.showNormalsProgram.shader.loadShaderProgramFromFile("shaders/showNormals.vert",
		"shaders/showNormals.geom", "shaders/showNormals.frag");


		internal.showNormalsProgram.modelTransformLocation = glGetUniformLocation(internal.showNormalsProgram.shader.id, "u_modelTransform");
		internal.showNormalsProgram.projectionLocation = glGetUniformLocation(internal.showNormalsProgram.shader.id, "u_projection");
		internal.showNormalsProgram.sizeLocation = glGetUniformLocation(internal.showNormalsProgram.shader.id, "u_size");
		internal.showNormalsProgram.colorLocation = glGetUniformLocation(internal.showNormalsProgram.shader.id, "u_color");
		
		//unsigned char textureData[] =
		//{
		//	20, 20, 20, 255,
		//	212, 0, 219, 255,
		//	212, 0, 219, 255,
		//	20, 20, 20, 255,
		//};
		//defaultTexture.loadTextureFromMemory(textureData, 2, 2, 4, TextureLoadQuality::leastPossible);

		internal.gBuffer.create(x, y);
		internal.ssao.create(x, y);
		postProcess.create(x, y);
		directionalShadows.create();
		spotShadows.create();
		pointShadows.create();
		renderDepthMap.create();
		antiAlias.create(x, y);
		adaptiveResolution.create(x, y);

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
	
	Material Renderer3D::createMaterial(glm::vec3 kd,
		float roughness, float metallic, float ao, std::string name,
		gl3d::Texture albedoTexture, gl3d::Texture normalTexture, gl3d::Texture roughnessTexture, gl3d::Texture metallicTexture,
		gl3d::Texture occlusionTexture, gl3d::Texture emmisiveTexture)
	{

		int id = internal::generateNewIndex(internal.materialIndexes);

		MaterialValues gpuMaterial;
		gpuMaterial.kd = glm::vec4(kd, 0);
		gpuMaterial.roughness = roughness;
		gpuMaterial.metallic = metallic;
		gpuMaterial.ao = ao;

		TextureDataForMaterial textureData{};

		textureData.albedoTexture = albedoTexture;
		textureData.normalMapTexture = normalTexture;
		textureData.emissiveTexture = emmisiveTexture;


		textureData.pbrTexture = createPBRTexture(roughnessTexture, metallicTexture, occlusionTexture);


		internal.materialIndexes.push_back(id);
		internal.materials.push_back(gpuMaterial);
		internal.materialNames.push_back(name);
		internal.materialTexturesData.push_back(textureData);

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

	static int max(int x, int y, int z)
	{
		return std::max(std::max(x, y), z);
	}

	//this is the function that loads a material from parsed data
	gl3d::Material createMaterialFromLoadedData(gl3d::Renderer3D &renderer, objl::Material &mat, const std::string &path)
	{
		auto m = renderer.createMaterial(mat.Kd, mat.roughness,
			mat.metallic, mat.ao, mat.name);

		stbi_set_flip_vertically_on_load(true);

		//todo i moved the code from here
		{
			//load textures for materials
			TextureDataForMaterial textureData;


			if (!mat.loadedDiffuse.data.empty())
			{
				textureData.albedoTexture = renderer.loadTextureFromMemory(mat.loadedDiffuse);

			}
			else
			if (!mat.map_Kd.empty())
			{
				textureData.albedoTexture = renderer.loadTexture(std::string(path + mat.map_Kd));
			}

			if (!mat.loadedNormal.data.empty())
			{
				textureData.normalMapTexture = renderer.loadTextureFromMemory(mat.loadedNormal);

			}
			else
			if (!mat.map_Kn.empty())
			{
				textureData.normalMapTexture = renderer.loadTexture(std::string(path + mat.map_Kn));
			}

			if (!mat.loadedEmissive.data.empty())
			{
				textureData.emissiveTexture = renderer.loadTextureFromMemory(mat.loadedEmissive);
				auto ind = renderer.internal.getMaterialIndex(m);
				renderer.internal.materials[ind].emmisive = 1.f;
			}
			else
			if (!mat.map_emissive.empty())
			{
				textureData.emissiveTexture = renderer.loadTexture(std::string(path + mat.map_emissive));
				auto ind = renderer.internal.getMaterialIndex(m);
				renderer.internal.materials[ind].emmisive = 1.f;
			}

			textureData.pbrTexture.RMA_loadedTextures = 0;

			auto rmaQuality = TextureLoadQuality::linearMipmap;

			//todo not working in gltf formats
			if (!mat.map_RMA.empty())
			{
			
				textureData.pbrTexture.texture = renderer.loadTexture(mat.map_RMA.c_str());
				if (textureData.pbrTexture.texture.id_ != 0)
				{
					textureData.pbrTexture.RMA_loadedTextures = 7; //all textures loaded
				}

			}

			if (!mat.loadedORM.data.empty())
			{
				auto &t = mat.loadedORM;

				//convert from ORM ro RMA
				for (int j = 0; j < t.h; j++)
					for (int i = 0; i < t.w; i++)
					{
						unsigned char R = t.data[(i + j * t.w) * 4 + 1];
						unsigned char M = t.data[(i + j * t.w) * 4 + 2];
						unsigned char A = t.data[(i + j * t.w) * 4 + 0];

						t.data[(i + j * t.w) * 3 + 0] = R;
						t.data[(i + j * t.w) * 3 + 1] = M;
						t.data[(i + j * t.w) * 3 + 2] = A;
					}

				GpuTexture tex;
				tex.loadTextureFromMemory(t.data.data(), t.w, t.h, 3, rmaQuality);
				textureData.pbrTexture.texture = renderer.createIntenralTexture(tex, 0);
				textureData.pbrTexture.RMA_loadedTextures = 7; //all textures loaded

			}
			else
				if (!mat.map_ORM.empty() && textureData.pbrTexture.RMA_loadedTextures == 0)
				{
					stbi_set_flip_vertically_on_load(true);

					int w = 0, h = 0;
					unsigned char *data = 0;

					{
						data = stbi_load(std::string(path + mat.map_ORM).c_str(),
							&w, &h, 0, 4);
						if (!data)
						{
							std::cout << "err loading " << std::string(path + mat.map_ORM) << "\n";
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
							textureData.pbrTexture.texture = renderer.createIntenralTexture(t, 0);

							textureData.pbrTexture.RMA_loadedTextures = 7; //all textures loaded

							stbi_image_free(data);
						}
					}

				}

			//RMA trexture
			if (textureData.pbrTexture.RMA_loadedTextures == 0)
			{
				constexpr int MERGE_TEXTURES_ON_GPU = 1;

				if constexpr (MERGE_TEXTURES_ON_GPU)
				{

					GpuTexture roughness{};

					if (!mat.map_Pr.empty())
					{
						roughness.loadTextureFromFile(std::string(path + mat.map_Pr).c_str(), dontSet, 1);
						glBindTexture(GL_TEXTURE_2D, roughness.id);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}

					GpuTexture metallic{};
					if (!mat.map_Pm.empty())
					{
						metallic.loadTextureFromFile(std::string(path + mat.map_Pm).c_str(), dontSet, 1);
						glBindTexture(GL_TEXTURE_2D, metallic.id);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}

					GpuTexture ambientOcclusion{};
					if (!mat.map_Ka.empty())
					{
						ambientOcclusion.loadTextureFromFile(std::string(path + mat.map_Ka).c_str(), dontSet, 1);
						glBindTexture(GL_TEXTURE_2D, ambientOcclusion.id);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}

					auto t = renderer.internal.pBRtextureMaker.createRMAtexture(
						roughness, metallic, ambientOcclusion, renderer.internal.lightShader.quadDrawer.quadVAO,
						textureData.pbrTexture.RMA_loadedTextures);

					if (textureData.pbrTexture.RMA_loadedTextures != 0)
					{
						textureData.pbrTexture.texture = renderer.createIntenralTexture(t, 0);
					}
					else
					{
						textureData.pbrTexture = {};
					}

					roughness.clear();
					metallic.clear();
					ambientOcclusion.clear();

				}
				else
				{
					stbi_set_flip_vertically_on_load(true);

					int w1 = 0, h1 = 0;
					unsigned char *data1 = 0;
					unsigned char *data2 = 0;
					unsigned char *data3 = 0;

					if (!mat.map_Pr.empty())
					{
						data1 = stbi_load(std::string(path + mat.map_Pr).c_str(),
							&w1, &h1, 0, 1);
						if (!data1) { std::cout << "err loading " << std::string(path + mat.map_Pr) << "\n"; }
					}

					int w2 = 0, h2 = 0;
					if (!mat.map_Pm.empty())
					{
						data2 = stbi_load(std::string(path + mat.map_Pm).c_str(),
							&w2, &h2, 0, 1);
						if (!data2) { std::cout << "err loading " << std::string(path + mat.map_Pm) << "\n"; }
					}


					int w3 = 0, h3 = 0;
					if (!mat.map_Ka.empty())
					{
						data3 = stbi_load(std::string(path + mat.map_Ka).c_str(),
							&w3, &h3, 0, 1);
						if (!data3) { std::cout << "err loading " << std::string(path + mat.map_Ka) << "\n"; }
					}

					int w = max(w1, w2, w3);
					int h = max(h1, h2, h3);

					//calculate which function to use
					if (data1 && data2 && data3) { textureData.pbrTexture.RMA_loadedTextures = 7; }
					else
					if (data2 && data3) { textureData.pbrTexture.RMA_loadedTextures = 6; }
					else
					if (data1 && data3) { textureData.pbrTexture.RMA_loadedTextures = 5; }
					else
					if (data1 && data2) { textureData.pbrTexture.RMA_loadedTextures = 4; }
					else
					if (data3) { textureData.pbrTexture.RMA_loadedTextures = 3; }
					else
					if (data2) { textureData.pbrTexture.RMA_loadedTextures = 2; }
					else
					if (data1) { textureData.pbrTexture.RMA_loadedTextures = 1; }
					else { textureData.pbrTexture.RMA_loadedTextures = 0; }

					if (textureData.pbrTexture.RMA_loadedTextures)
					{

						unsigned char *finalData = new unsigned char[w * h * 4];

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
						textureData.pbrTexture.texture = renderer.createIntenralTexture(t, 0);

						stbi_image_free(data1);
						stbi_image_free(data2);
						stbi_image_free(data3);
						delete[] finalData;

					}
				}

				/*

				*/

			}

			renderer.setMaterialTextures(m, textureData);

		}

		return m;
	}

	std::vector<Material> Renderer3D::loadMaterial(std::string file)
	{

		objl::Loader loader;
		if (!loader.LoadMaterials(file)) 
		{
			std::cout << "err loading: " << file << "\n";
			return {};
		}

		std::vector<Material> ret;
		ret.reserve(loader.LoadedMaterials.size());

		std::string path = file;
		while (!path.empty() &&
			*(path.end() - 1) != '\\' &&
			*(path.end() - 1) != '/'
			)
		{
			path.pop_back();
		}

		for (auto &m : loader.LoadedMaterials)
		{

			auto material = createMaterialFromLoadedData(*this, m, path);
			ret.push_back(material);

		}

		return std::move(ret);
	}

	bool Renderer3D::deleteMaterial(Material m)
	{
		auto pos = std::find(internal.materialIndexes.begin(), internal.materialIndexes.end(), m.id_);

		if (pos == internal.materialIndexes.end())
		{
			gl3dAssertComment(pos != internal.materialIndexes.end(), "invalid delete material");
			return 0;
		}

		int index = pos - internal.materialIndexes.begin();

		internal.materialIndexes.erase(pos);
		internal.materials.erase(internal.materials.begin() + index);
		internal.materialNames.erase(internal.materialNames.begin() + index);
		internal.materialTexturesData.erase(internal.materialTexturesData.begin() + index);
		m.id_ = 0;
		return 1;
	}

	bool Renderer3D::copyMaterialData(Material dest, Material source)
	{
		int destId = internal.getMaterialIndex(dest);
		int sourceId = internal.getMaterialIndex(source);

		if(destId == -1 || sourceId == -1)
		{
			gl3dAssertComment(destId != -1, "invaled dest material index");
			gl3dAssertComment(sourceId != -1, "invaled source material index");

			return 0;
		}

		internal.materials[destId] = internal.materials[sourceId];
		internal.materialNames[destId] = internal.materialNames[sourceId];
		internal.materialTexturesData[destId] = internal.materialTexturesData[destId];

		return 1;
	}

	MaterialValues Renderer3D::getMaterialValues(Material m)
	{
		int id = internal.getMaterialIndex(m);

		if(id == -1)
		{
			return {};
		}
		
		return internal.materials[id];
	}

	void Renderer3D::setMaterialValues(Material m, MaterialValues values)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return;
		}

		internal.materials[id] = values;

	}

	TextureDataForMaterial Renderer3D::getMaterialTextures(Material m)
	{
		int id = internal.getMaterialIndex(m);
		if (id == -1)
		{return {};}

		return internal.materialTexturesData[id];
	}

	void Renderer3D::setMaterialTextures(Material m, TextureDataForMaterial textures)
	{
		int id = internal.getMaterialIndex(m);
		if (id == -1)
		{return;}
		
		internal.materialTexturesData[id] = textures;
	}

	std::string Renderer3D::getMaterialName(Material m)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return "";
		}

		return internal.materialNames[id];

	}

	void Renderer3D::setMaterialName(Material m, const std::string& name)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{return;}

		internal.materialNames[id] = name;
	}

	bool Renderer3D::isMaterial(Material& m)
	{
		int id = internal.getMaterialIndex(m);

		if (id == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	//bool Renderer3D::setMaterialData(Material m, const MaterialValues &data, std::string *s)
	//{
	//	int id = internal.getMaterialIndex(m);
	//
	//	if (id == -1)
	//	{
	//		return 0;
	//	}
	//
	//	internal.materials[id] = data;
	//	
	//	if (s)
	//	{
	//		internal.materialNames[id] = *s;
	//	}
	//
	//	return 1;
	//}



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

	Texture Renderer3D::loadTextureFromMemory(objl::LoadedTexture &t)
	{
		if (t.data.empty())
		{
			return Texture{ 0 };
		}

	
		GpuTexture tex;
		internal::GpuTextureWithFlags text;
		int alphaExists = 1; tex.loadTextureFromMemory((void *)t.data.data(), t.w, t.h, t.components); //todo refactor and add check alpha

		text.texture = tex;
		text.flags = alphaExists;

		//if texture is not loaded, return an invalid texture
		if (tex.id == 0)
		{
			return Texture{ 0 };
		}

		int id = internal::generateNewIndex(internal.loadedTexturesIndexes);

		internal.loadedTexturesIndexes.push_back(id);
		internal.loadedTextures.push_back(text);
		internal.loadedTexturesNames.push_back("");

		return Texture{ id };
	}

	GLuint Renderer3D::getTextureOpenglId(Texture& t)
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

	bool Renderer3D::isTexture(Texture& t)
	{
		int i = internal.getTextureIndex(t);

		if (i > -1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void Renderer3D::deleteTexture(Texture& t)
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

	GpuTexture *Renderer3D::getTextureData(Texture& t)
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

	//this takes an id and adds the texture to the internal system
	Texture Renderer3D::createIntenralTexture(GLuint id_, int alphaData)
	{
		if (!id_)
		{
			return {};
		}

		GpuTexture t;
		t.id = id_;
		return createIntenralTexture(t, alphaData);

	}

	PBRTexture Renderer3D::createPBRTexture(Texture& roughness, Texture& metallic,
		Texture& ambientOcclusion)
	{

		PBRTexture ret = {};

		auto t = internal.pBRtextureMaker.createRMAtexture(
			{getTextureOpenglId(roughness)},
			{ getTextureOpenglId(metallic) },
			{ getTextureOpenglId(ambientOcclusion) }, internal.lightShader.quadDrawer.quadVAO, ret.RMA_loadedTextures);

		ret.texture = this->createIntenralTexture(t, 0);

		return ret;
	}

	void Renderer3D::deletePBRTexture(PBRTexture& t)
	{
		deleteTexture(t.texture);
		t.RMA_loadedTextures = 0;
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
	
		ModelData returnModel; //todo move stuff into a function
		{

			int s = model.loader.LoadedMeshes.size();
			returnModel.models.reserve(s);

			#pragma region materials
			returnModel.createdMaterials.reserve(model.loader.LoadedMaterials.size());
			for(int i=0;i<model.loader.LoadedMaterials.size(); i++)
			{
				auto &mat = model.loader.LoadedMaterials[i];
				
				auto m = createMaterialFromLoadedData(*this, mat, model.path);

				returnModel.createdMaterials.push_back(m);
			}
			#pragma endregion

			#pragma region aninmations
			if (!model.loader.animations.empty())
			{
				returnModel.animations = model.loader.animations;
			}

			if (!model.loader.joints.empty())
			{
				returnModel.joints = model.loader.joints;
			}
			#pragma endregion


			for (int i = 0; i < s; i++)
			{
				GraphicModel gm;
				

				int index = i;
				//TextureDataForModel textureData = {};

				auto &mesh = model.loader.LoadedMeshes[index];
				
				if (!mesh.Vertices.empty()) //this has data without animation data
				{
					if (mesh.Indices.empty())
					{
						gm.loadFromComputedData(mesh.Vertices.size() * sizeof(mesh.Vertices[0]),
							(float *)&mesh.Vertices[0],
							0, nullptr);
					}
					else
					{
						gm.loadFromComputedData(mesh.Vertices.size() * sizeof(mesh.Vertices[0]),
							(float *)&mesh.Vertices[0],
							mesh.Indices.size() * 4, &mesh.Indices[0]);
					}
				}
				else //if(mesh.VerticesAnimations.size()) //todo empty model ?
				{
					if (mesh.Indices.empty())
					{
						gm.loadFromComputedData(mesh.VerticesAnimations.size() * sizeof(mesh.VerticesAnimations[0]),
							(float *)&mesh.VerticesAnimations[0],
							0, nullptr, false, true);
					}
					else
					{
						gm.loadFromComputedData(mesh.VerticesAnimations.size() * sizeof(mesh.VerticesAnimations[0]),
							(float *)&mesh.VerticesAnimations[0],
							mesh.Indices.size() * 4, &mesh.Indices[0], false, true);
					}
				}
				
				gm.hasBones = mesh.hasBones;

				if(model.loader.LoadedMeshes[index].materialIndex > -1)
				{
					gm.material = returnModel.createdMaterials[model.loader.LoadedMeshes[index].materialIndex];
				}else
				{
					//if no material loaded for this object create a new default one
					gm.material = createMaterial(glm::vec3{ 0.8 }, 0.5, 0, 1.f, "default material");
				}
				
				gm.ownMaterial = true;

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

	bool Renderer3D::isModel(Model& m)
	{
		auto pos = internal.getModelIndex(m);
		if (pos < 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}

	void Renderer3D::deleteModel(Model &m) //todo delete created materials
	{
		auto pos = internal.getModelIndex(m);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete model");
			return;
		}

		internal.graphicModelsIndexes.erase(internal.graphicModelsIndexes.begin() + pos);
		internal.graphicModels[pos].clear(*this);
		internal.graphicModels.erase(internal.graphicModels.begin() + pos);

		m.id_ = 0;
	}

	void Renderer3D::clearModelData(Model& m)
	{
		auto pos = internal.getModelIndex(m);
		if (pos < 0)
		{
			return;
		}

		internal.graphicModels[pos].clear(*this);
	}

	int Renderer3D::getModelMeshesCount(Model& m)
	{
		auto pos = internal.getModelIndex(m);
		if (pos < 0)
		{
			return 0;
		}
		else
		{
			return internal.graphicModels[pos].models.size();
		}
	}

	std::string Renderer3D::getModelMeshesName(Model& m, int index)
	{
		auto pos = internal.getModelIndex(m);
		if (pos >= 0)
		{
			if(index < internal.graphicModels[pos].models.size())
			{
				return internal.graphicModels[pos].models[pos].name;
			}
			else
			{
				return "";
			}
		}
		else
		{
			return "";
		}
	}

	std::vector<char*>* Renderer3D::getModelMeshesNames(Model& m)
	{
		auto i = internal.getModelIndex(m);
		if (i < 0) { return nullptr; } //warn or sthing

		return &internal.graphicModels[i].subModelsNames;
	}


#pragma region point light

	PointLight Renderer3D::createPointLight(glm::vec3 position, glm::vec3 color,
		float dist, float attenuation)
	{
		int id = internal::generateNewIndex(internal.pointLightIndexes);
		internal::GpuPointLight light;
		light.position = position;
		light.color = color;
		light.dist = glm::max(0.f, dist);
		light.attenuation = glm::max(0.f, attenuation);

		internal.pointLightIndexes.push_back(id);
		internal.pointLights.push_back(light);

		internal.perFrameFlags.shouldUpdatePointShadows = true;

		return { id };
	}

	void Renderer3D::detletePointLight(PointLight& l)
	{
		auto pos = internal.getPointLightIndex(l);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete point light");
			return;
		}

		//if (internal.spotLights[pos].castShadows)
		//{
		//	internal.perFrameFlags.shouldUpdateSpotShadows = true;
		//}

		internal.pointLightIndexes.erase(internal.pointLightIndexes.begin() + pos);
		internal.pointLights.erase(internal.pointLights.begin() + pos);

		l.id_ = 0;

		internal.perFrameFlags.shouldUpdatePointShadows = true;
	}

	glm::vec3 Renderer3D::getPointLightPosition(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].position;
	}

	void Renderer3D::setPointLightPosition(PointLight& l, glm::vec3 position)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		if (internal.pointLights[i].position != position)
		{
			internal.pointLights[i].position = position;
			internal.pointLights[i].changedThisFrame = true;
		}
	
	}

	bool Renderer3D::isPointLight(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	glm::vec3 Renderer3D::getPointLightColor(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].color;
	}

	void Renderer3D::setPointLightColor(PointLight& l, glm::vec3 color)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.pointLights[i].color = color;
	}

	float Renderer3D::getPointLightDistance(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing

		return internal.pointLights[i].dist;
	}

	void Renderer3D::setPointLightDistance(PointLight& l, float distance)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		distance = glm::max(0.f, distance);

		if (internal.pointLights[i].dist != distance)
		{
			internal.pointLights[i].dist = distance;
			internal.pointLights[i].changedThisFrame = true;
		}
	}

	float Renderer3D::getPointLightAttenuation(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].attenuation;
	}

	void Renderer3D::setPointLightAttenuation(PointLight& l, float attenuation)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.pointLights[i].attenuation = glm::max(attenuation, 0.f);
	}

	bool Renderer3D::getPointLightShadows(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].castShadows;
	}

	void Renderer3D::setPointLightShadows(PointLight& l, bool castShadows)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		
		if (internal.pointLights[i].castShadows != castShadows)
		{
			internal.pointLights[i].castShadows = castShadows;
			internal.pointLights[i].changedThisFrame = true;
			internal.perFrameFlags.shouldUpdatePointShadows = true;
		}
	}

	float Renderer3D::getPointLightHardness(PointLight& l)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.pointLights[i].hardness;
	}

	void Renderer3D::setPointLightHardness(PointLight& l, float hardness)
	{
		auto i = internal.getPointLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.pointLights[i].hardness = glm::max(hardness, 0.001f);
	}

	int Renderer3D::getPointLightShadowSize()
	{
		return pointShadows.shadowSize;
	}

	void Renderer3D::setPointLightShadowSize(int size)
	{
		size = std::min(std::max(256, size), 2048);

		if (size != pointShadows.shadowSize)
		{
			pointShadows.shadowSize = size;
			internal.perFrameFlags.shouldUpdatePointShadows = true;
		}
	}

#pragma endregion


#pragma region directional light

	DirectionalLight Renderer3D::createDirectionalLight(glm::vec3 direction,
		glm::vec3 color, float hardness, bool castShadows)
	{
		int id = internal::generateNewIndex(internal.directionalLightIndexes);
		internal::GpuDirectionalLight light;
		light.color = color;
		light.hardness = glm::max(hardness, 0.001f);
		if (castShadows)
		{
			light.castShadowsIndex = 1;
		}
		else
		{
			light.castShadowsIndex = -1;
		}

		if (glm::length(direction) == 0)
		{
			direction = glm::vec3{ 0, -1, 0 };
		}
		else
		{
			direction = glm::normalize(direction);
		}
		light.direction = glm::vec4(direction, 0);

		internal.directionalLightIndexes.push_back(id);
		internal.directionalLights.push_back(light);

		internal.perFrameFlags.shouldUpdateDirectionalShadows = true;

		return { id };
	}

	void Renderer3D::deleteDirectionalLight(DirectionalLight& l)
	{
		auto pos = internal.getDirectionalLightIndex(l);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete directional light");
			return;
		}


		//if (internal.directionalLights[pos].castShadows)
		//{
		//	internal.perFrameFlags.shouldUpdateDirectionalShadows = true;
		//}

		internal.directionalLightIndexes.erase(internal.directionalLightIndexes.begin() + pos);
		internal.directionalLights.erase(internal.directionalLights.begin() + pos);

		l.id_ = 0;

		internal.perFrameFlags.shouldUpdateDirectionalShadows = true;
	}

	bool Renderer3D::isDirectionalLight(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	glm::vec3 Renderer3D::getDirectionalLightDirection(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.directionalLights[i].direction;
	}

	void Renderer3D::setDirectionalLightDirection(DirectionalLight& l, glm::vec3 direction)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		if (glm::length(direction) == 0)
		{
			direction = glm::vec3{ 0, -1, 0 };
		}
		else
		{
			direction = glm::normalize(direction);
		}

		if (glm::vec3(internal.directionalLights[i].direction) != direction)
		{
			internal.directionalLights[i].direction = glm::vec4(direction, 0);
			internal.directionalLights[i].changedThisFrame = true;
		}
	}

	glm::vec3 Renderer3D::getDirectionalLightColor(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.directionalLights[i].color;
	}

	void Renderer3D::setDirectionalLightColor(DirectionalLight& l, glm::vec3 color)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.directionalLights[i].color = color;
	}

	float Renderer3D::getDirectionalLightHardness(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.directionalLights[i].hardness;
	}

	void Renderer3D::setDirectionalLightHardness(DirectionalLight& l, float hardness)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.directionalLights[i].hardness = glm::max(hardness, 0.001f);
	}

	bool Renderer3D::getDirectionalLightShadows(DirectionalLight& l)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		
		return internal.directionalLights[i].castShadows;

	}

	void Renderer3D::setDirectionalLightShadows(DirectionalLight& l, bool castShadows)
	{
		auto i = internal.getDirectionalLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		if (castShadows != internal.directionalLights[i].castShadows)
		{
			internal.directionalLights[i].castShadows = castShadows;
			internal.directionalLights[i].changedThisFrame = true;
			internal.perFrameFlags.shouldUpdateDirectionalShadows = true;
		}
	}

	int Renderer3D::getDirectionalLightShadowSize()
	{
		return directionalShadows.shadowSize;
	}

	void Renderer3D::setDirectionalLightShadowSize(int size)
	{
		size = std::min(std::max(256, size), 2048);

		if (size != directionalShadows.shadowSize)
		{
			directionalShadows.shadowSize = size;
			//internal.perFrameFlags.shouldupdatedi
		}
	}

#pragma endregion


#pragma region spot light

	SpotLight Renderer3D::createSpotLight(glm::vec3 position, float fovRadians, glm::vec3 direction,
		float dist, float attenuation, glm::vec3 color, float hardness, int castShadows)
	{
		int id = internal::generateNewIndex(internal.spotLightIndexes);

		internal::GpuSpotLight light = {};
		light.position = position;
		
		fovRadians = glm::clamp(fovRadians, glm::radians(0.f), glm::radians(160.f));
		fovRadians /= 2.f;
		fovRadians = std::cos(fovRadians);
		light.cosHalfAngle = fovRadians;
		
		if (glm::length(direction) == 0)
		{
			direction = glm::vec3{0, -1, 0};
		}
		else
		{
			direction = glm::normalize(direction);
		}
		light.direction = direction;
		
		light.dist = glm::max(0.f, dist);
		light.attenuation = glm::max(attenuation, 0.f);
		light.color = color;
		light.hardness = glm::max(hardness, 0.001f);
		light.castShadows = castShadows;
		light.changedThisFrame = true;

		if (castShadows)
		{
			internal.perFrameFlags.shouldUpdateSpotShadows = true;
		}

		internal.spotLightIndexes.push_back(id);
		internal.spotLights.push_back(light);

		return { id };
	}

	//todo check
	SpotLight Renderer3D::createSpotLight(glm::vec3 position, float fovRadians, glm::vec2 anglesRadians, float dist, float attenuation, glm::vec3 color, float hardness, int castShadows)
	{
		glm::vec3 direction = fromAnglesToDirection(anglesRadians.x, anglesRadians.y);

		return createSpotLight(position, fovRadians, direction, dist, attenuation,
			color, hardness, castShadows);

	}

	void Renderer3D::deleteSpotLight(SpotLight& l)
	{
		auto pos = internal.getSpotLightIndex(l);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete spot light");
			return;
		}


		if (internal.spotLights[pos].castShadows)
		{ 
			internal.perFrameFlags.shouldUpdateSpotShadows = true;
		}

		internal.spotLightIndexes.erase(internal.spotLightIndexes.begin() + pos);
		internal.spotLights.erase(internal.spotLights.begin() + pos);

		l.id_ = 0;
	}

	glm::vec3 Renderer3D::getSpotLightPosition(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].position;

	}

	void Renderer3D::setSpotLightPosition(SpotLight& l, glm::vec3 position)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return ; } //warn or sthing

		if (internal.spotLights[i].position != position)
		{
			internal.spotLights[i].position = position;
			internal.spotLights[i].changedThisFrame = true;
		}
	}

	bool Renderer3D::isSpotLight(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	glm::vec3 Renderer3D::getSpotLightColor(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].color;
	}

	void Renderer3D::setSpotLightColor(SpotLight& l, glm::vec3 color)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.spotLights[i].color = color;
	}

	float Renderer3D::getSpotLightFov(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing

		float angle =  internal.spotLights[i].cosHalfAngle;
		angle = std::acos(angle);
		angle *= 2;

		return angle;
	}

	void Renderer3D::setSpotLightFov(SpotLight& l, float fovRadians)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		fovRadians = glm::clamp(fovRadians, glm::radians(0.f), glm::radians(160.f)); //todo magic number
		fovRadians /= 2.f;
		fovRadians = std::cos(fovRadians);

		if(internal.spotLights[i].cosHalfAngle != fovRadians)
		{
			internal.spotLights[i].cosHalfAngle = fovRadians;
			internal.spotLights[i].changedThisFrame= true;
		}
	}

	glm::vec3 Renderer3D::getSpotLightDirection(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing

		return internal.spotLights[i].direction;
	}

	void Renderer3D::setSpotLightDirection(SpotLight& l, glm::vec3 direction)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		if (glm::length(direction) == 0)
		{
			direction = glm::vec3{ 0, -1, 0 };
		}
		else
		{
			direction = glm::normalize(direction);
		}

		if (internal.spotLights[i].direction != direction)
		{
			internal.spotLights[i].direction = direction;
			internal.spotLights[i].changedThisFrame = true;
		}
	}

	float Renderer3D::getSpotLightDistance(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing

		return internal.spotLights[i].dist;
	}

	void Renderer3D::setSpotLightDistance(SpotLight& l, float distance)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		distance = glm::max(0.f, distance);

		if (internal.spotLights[i].dist != distance)
		{
			internal.spotLights[i].dist = distance;
			internal.spotLights[i].changedThisFrame = true;
		}
	}

	float Renderer3D::getSpotLightAttenuation(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].attenuation;
	}

	void Renderer3D::setSpotLightAttenuation(SpotLight& l, float attenuation)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.spotLights[i].attenuation = glm::max(attenuation, 0.f);
	}

	float Renderer3D::getSpotLightHardness(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].hardness;
	}

	void Renderer3D::setSpotLightHardness(SpotLight& l, float hardness)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing
		internal.spotLights[i].hardness = glm::max(hardness, 0.001f);
	}

	void Renderer3D::setSpotLightShadows(SpotLight& l, bool castShadows)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return; } //warn or sthing

		if (internal.spotLights[i].castShadows != castShadows)
		{
			
			internal.spotLights[i].castShadows = castShadows;
			internal.spotLights[i].changedThisFrame = true;
			internal.perFrameFlags.shouldUpdateSpotShadows = true;
		}
	}

	bool Renderer3D::getSpotLightShadows(SpotLight& l)
	{
		auto i = internal.getSpotLightIndex(l);
		if (i < 0) { return {}; } //warn or sthing
		return internal.spotLights[i].castShadows;
	}

	int Renderer3D::getSpotLightShadowSize()
	{
		return spotShadows.shadowSize;
	}

	void Renderer3D::setSpotLightShadowSize(int size)
	{
		size = std::min(std::max(256, size), 2048);

		if (spotShadows.shadowSize != size)
		{
			spotShadows.shadowSize = size;
			internal.perFrameFlags.shouldUpdateSpotShadows = true;
		}
	}

#pragma endregion

	Entity Renderer3D::createEntity(Model m, Transform transform,
		bool staticGeometry, bool visible, bool castShadows)
	{
		int id = internal::generateNewIndex(internal.entitiesIndexes);

		CpuEntity entity;
		
		entity.allocateGpuData();
		entity.transform = transform;
		entity.setStatic(staticGeometry);
		entity.setVisible(visible);
		entity.setCastShadows(castShadows);

		internal.entitiesIndexes.push_back(id);
		internal.cpuEntities.push_back(std::move(entity));

		if (staticGeometry && visible && castShadows)
		{
			internal.perFrameFlags.staticGeometryChanged = true;
		}

		Entity e;
		e.id_ = id;
		
		setEntityModel(e, m);
		
		return e;
	}

	Entity Renderer3D::duplicateEntity(Entity& e)
	{
		int oldIndex = internal.getEntityIndex(e);

		if (oldIndex < 0)
		{
			return {};
		}
		
		int id = internal::generateNewIndex(internal.entitiesIndexes);

		CpuEntity entity;

		entity.allocateGpuData();
		entity.transform = internal.cpuEntities[oldIndex].transform;
		entity.flags = internal.cpuEntities[oldIndex].flags;
		entity.subModelsNames.reserve(internal.cpuEntities[oldIndex].subModelsNames.size());

		for (auto i : internal.cpuEntities[oldIndex].subModelsNames)
		{
			int size = strlen(i);
			char* c = new char[size] {};
			strcpy(c, i);
			entity.subModelsNames.push_back(c);
		}

		entity.models.reserve(internal.cpuEntities[oldIndex].models.size());
		for (auto i : internal.cpuEntities[oldIndex].models)
		{
			GraphicModel model = i;

			if (model.ownMaterial)
			{
				model.material = createMaterial();
				this->copyMaterialData(model.material, i.material);
			}

			entity.models.push_back(std::move(model));
		}

		internal.entitiesIndexes.push_back(id);
		internal.cpuEntities.push_back(std::move(entity));

		Entity ret;
		ret.id_ = id;
		return ret;
	}

	void Renderer3D::setEntityModel(Entity& e, Model m)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn

		clearEntityModel(e);

		auto& entity = internal.cpuEntities[i];

		//clear if needed

		int modelindex = internal.getModelIndex(m);
		if (modelindex >= 0)
		{
			int size = internal.graphicModels[modelindex].models.size();
			entity.models.reserve(size);

			entity.animations = internal.graphicModels[modelindex].animations;
			entity.joints = internal.graphicModels[modelindex].joints;

			for (int i = 0; i < size; i++)
			{
				entity.models.push_back(internal.graphicModels[modelindex].models[i]);
				entity.models.back().ownMaterial = false;

				int charSize = strlen(internal.graphicModels[modelindex].subModelsNames[i]);
				char* name = new char[charSize + 1]{};
				strcpy(name, internal.graphicModels[modelindex].subModelsNames[i]);

				entity.subModelsNames.push_back(name);
			}

		}
	}

	void Renderer3D::clearEntityModel(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return ; } //warn

		auto& entity = internal.cpuEntities[i];

		if (entity.isStatic() && entity.isVisible())
		{
			internal.perFrameFlags.staticGeometryChanged = true;
		}

		for (auto& i : entity.subModelsNames)
		{
			delete[] i;
		}

		for (auto& i : entity.models)
		{
			if (i.ownMaterial)
			{
				this->deleteMaterial(i.material);
			}
		}

		entity.models.clear();
		entity.subModelsNames.clear();

	}

	CpuEntity* Renderer3D::getEntityData(Entity &e)
	{
		auto i = internal.getEntityIndex(e);

		if (i < 0) { return nullptr; }

		return &internal.cpuEntities[i];

	}

	Transform Renderer3D::getEntityTransform(Entity &e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return Transform{}; } //warn or sthing
	
		return internal.cpuEntities[i].transform;
	}

	void Renderer3D::setEntityTransform(Entity &e, Transform transform)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing
		
		if (internal.cpuEntities[i].isStatic())
		{
			if (internal.cpuEntities[i].transform != transform)
			{
				internal.perFrameFlags.staticGeometryChanged = true;
			};
		}

		internal.cpuEntities[i].transform = transform;

	}

	bool Renderer3D::isEntityStatic(Entity &e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing

		return internal.cpuEntities[i].isStatic();
	}

	void Renderer3D::setEntityStatic(Entity &e, bool s)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		auto &en = internal.cpuEntities[i];

		if (en.animate()) //entities that animate can't be static
		{
			s = false;
		}
		
		if ((en.isStatic() != s)
			&& en.isVisible()
			&& en.castShadows()
			)
		{
			internal.perFrameFlags.staticGeometryChanged = true;
		}

		en.setStatic(s);
	}

	void Renderer3D::deleteEntity(Entity &e)
	{
		auto pos = internal.getEntityIndex(e);
		if (pos < 0)
		{
			gl3dAssertComment(pos >= 0, "invalid delete entity");
			return;
		}

		clearEntityModel(e);

		if (internal.cpuEntities[pos].isStatic())
		{
			internal.perFrameFlags.staticGeometryChanged = true;
		}

		internal.cpuEntities[pos].deleteGpuData();

		internal.entitiesIndexes.erase(internal.entitiesIndexes.begin() + pos);
		internal.cpuEntities.erase(internal.cpuEntities.begin() + pos);
		

		e.id_ = 0;

	}

	int Renderer3D::getEntityMeshesCount(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing

		return internal.cpuEntities[i].models.size();

	}

	MaterialValues Renderer3D::getEntityMeshMaterialValues(Entity& e, int meshIndex)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return {}; } //warn or sthing
	
		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto mat = internal.cpuEntities[i].models[meshIndex].material;
			MaterialValues data = {};
			bool succeeded = internal.getMaterialData(mat, &data, nullptr, nullptr);

			if (succeeded)
			{
				return data;
			}
			else
			{
				return {}; //warn
			}
		}
		else
		{
			return {}; //warn
		}

	}

	void Renderer3D::setEntityMeshMaterialValues(Entity& e, int meshIndex, MaterialValues mat)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return ; } //warn or sthing

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			MaterialValues data = {};
			std::string name = {};
			TextureDataForMaterial textures;
			bool succeeded = internal.getMaterialData(currentMat, &data, &name, &textures);

			if (succeeded)
			{
				if (internal.cpuEntities[i].models[meshIndex].ownMaterial == 1)
				{
					setMaterialValues(currentMat, mat);
				}else
				if (mat != data)
				{
					Material newMat = this->createMaterial(mat.kd, mat.roughness,
						mat.metallic, mat.ao, name);
					int newMatIndex = internal.getMaterialIndex(newMat); //this should not fail

					internal.materialTexturesData[newMatIndex] = textures;

					internal.cpuEntities[i].models[meshIndex].material = newMat;
					internal.cpuEntities[i].models[meshIndex].ownMaterial = 1;
				}
			}
			else
			{
				return ; //warn
			}
		}
		else
		{
			return ; //warn
		}

	}

	std::string Renderer3D::getEntityMeshMaterialName(Entity& e, int meshIndex)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return {}; } //no valid entity

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			std::string name = {};
			bool succeeded = internal.getMaterialData(currentMat, nullptr, &name, nullptr);
			if (succeeded)
			{
				return name;
			}
			else
			{
				return{};//no valid material
			}
		}
		else
		{
			return {};//wrong index
		}

	}

	void Renderer3D::setEntityMeshMaterialName(Entity& e, int meshIndex, const std::string& name)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			MaterialValues data = {};
			std::string oldName = {};
			TextureDataForMaterial textures;
			bool succeeded = internal.getMaterialData(currentMat, &data, &oldName, &textures);

			if (succeeded)
			{
				if (internal.cpuEntities[i].models[meshIndex].ownMaterial == 1)
				{
					setMaterialName(currentMat, name);
				}
				else
				if (name != oldName) //copy to new material
				{
					Material newMat = this->createMaterial(data.kd, data.roughness,
						data.metallic, data.ao, name);
					int newMatIndex = internal.getMaterialIndex(newMat); //this should not fail
					internal.materialTexturesData[newMatIndex] = textures;

					internal.cpuEntities[i].models[meshIndex].material = newMat;
					internal.cpuEntities[i].models[meshIndex].ownMaterial = 1;
				}
			}
			else
			{
				return; //warn
			}
		}
		else
		{
			return; //warn
		}
	}

	void Renderer3D::setEntityMeshMaterial(Entity& e, int meshIndex, Material mat)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //invalid entity;

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			if (internal.cpuEntities[i].models[meshIndex].ownMaterial)
			{
				deleteMaterial(internal.cpuEntities[i].models[meshIndex].material);
			}

			internal.cpuEntities[i].models[meshIndex].material = mat;
			internal.cpuEntities[i].models[meshIndex].ownMaterial = 0;

			//todo look into textures and see if they have alpha data
			if (
				internal.cpuEntities[i].isStatic()
				&& internal.cpuEntities[i].castShadows()
				&&internal.cpuEntities[i].isVisible()
				)
			{
				internal.perFrameFlags.staticGeometryChanged = true;
			}
		}
	}

	TextureDataForMaterial Renderer3D::getEntityMeshMaterialTextures(Entity& e, int meshIndex)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return {}; } //no valid entity

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			TextureDataForMaterial t= {};
			bool succeeded = internal.getMaterialData(currentMat, nullptr, nullptr, &t);
			if (succeeded)
			{
				return t;
			}
			else
			{
				return{};//invalid material
			}

		}
		else
		{
			return {};//invalid index
		}
	}

	void Renderer3D::setEntityMeshMaterialTextures(Entity& e, int meshIndex, TextureDataForMaterial texture)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if (meshIndex < internal.cpuEntities[i].models.size())
		{
			auto currentMat = internal.cpuEntities[i].models[meshIndex].material;
			MaterialValues data = {};
			std::string oldName = {};
			TextureDataForMaterial oldTextures;
			bool succeeded = internal.getMaterialData(currentMat, &data, &oldName, &oldTextures);

			if (succeeded)
			{
				if (internal.cpuEntities[i].models[meshIndex].ownMaterial == 1)
				{
					setMaterialTextures(currentMat, texture);
				}
				else
				if (texture != oldTextures) //copy to new material
				{
					Material newMat = this->createMaterial(data.kd, data.roughness,
						data.metallic, data.ao, oldName);
					int newMatIndex = internal.getMaterialIndex(newMat); //this should not fail
					internal.materialTexturesData[newMatIndex] = texture; //new textures

					internal.cpuEntities[i].models[meshIndex].material = newMat;
					internal.cpuEntities[i].models[meshIndex].ownMaterial = 1;
				}
			}
			else
			{
				return; //warn
			}
		}
		else
		{
			return; //warn
		}
	}

	bool Renderer3D::isEntity(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) 
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool Renderer3D::isEntityVisible(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing
		return internal.cpuEntities[i].isVisible();
	}

	void Renderer3D::setEntityVisible(Entity& e, bool v)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if (internal.cpuEntities[i].isVisible() != v)
		{
			internal.cpuEntities[i].setVisible(v);
			if (internal.cpuEntities[i].isStatic()
				&& internal.cpuEntities[i].castShadows()
				)
			{
				internal.perFrameFlags.staticGeometryChanged = true;
			}
		}
	}

	void Renderer3D::setEntityCastShadows(Entity& e, bool s)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if(
			internal.cpuEntities[i].isVisible()
			&& internal.cpuEntities[i].isStatic()
			&& (s != internal.cpuEntities[i].castShadows())
			)
		{
			internal.perFrameFlags.staticGeometryChanged = true;
		}

		internal.cpuEntities[i].setCastShadows(s);
	}

	bool Renderer3D::getEntityCastShadows(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing
		return internal.cpuEntities[i].castShadows();
	}

	void Renderer3D::setEntityAnimationIndex(Entity &e, int ind)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if (internal.cpuEntities[i].animationIndex != ind)
		{
			internal.cpuEntities[i].totalTimePassed = 0;
			internal.cpuEntities[i].animationIndex = ind;
		}

	}

	int Renderer3D::getEntityAnimationIndex(Entity &e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing
		return internal.cpuEntities[i].animationIndex;
	}

	void Renderer3D::setEntityAnimationSpeed(Entity &e, float speed)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing

		if (speed < 0) { speed = 0; } //todo negative speed mabe?

		internal.cpuEntities[i].animationSpeed = speed;
	}

	float Renderer3D::getEntityAnimationSpeed(Entity &e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing
		return internal.cpuEntities[i].animationSpeed;
	}


	std::vector<char*> *Renderer3D::getEntityMeshesNames(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return nullptr; } //warn or sthing
		
		return &internal.cpuEntities[i].subModelsNames;
	}

	void Renderer3D::setEntityAnimate(Entity& e, bool animate)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return; } //warn or sthing
		auto& en = internal.cpuEntities[i];
		if (en.canBeAnimated())
		{
			en.setAnimate(animate);

			if (animate && en.isStatic())
			{
				setEntityStatic(e, false);
			}
		}

	}

	bool Renderer3D::getEntityAnimate(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing

		auto& en = internal.cpuEntities[i];

		return en.animate() && en.canBeAnimated();
	}

	bool Renderer3D::entityCanAnimate(Entity& e)
	{
		auto i = internal.getEntityIndex(e);
		if (i < 0) { return 0; } //warn or sthing

		auto& en = internal.cpuEntities[i];

		return en.canBeAnimated();
	}

	void Renderer3D::setExposure(float exposure)
	{
		internal.lightShader.lightPassUniformBlockCpuData.exposure =
			std::max(exposure, 0.001f);
	}

	float Renderer3D::getExposure()
	{
		return internal.lightShader.lightPassUniformBlockCpuData.exposure;
	}

	void Renderer3D::enableNormalMapping(bool normalMapping)
	{
		internal.lightShader.normalMap = normalMapping;
	}

	bool Renderer3D::isNormalMappingEnabeled()
	{
		return internal.lightShader.normalMap;
	}

	void Renderer3D::enableLightSubScattering(bool lightSubScatter)
	{
		internal.lightShader.lightPassUniformBlockCpuData.lightSubScater = lightSubScatter;
	}

	bool Renderer3D::isLightSubScatteringEnabeled()
	{
		return internal.lightShader.lightPassUniformBlockCpuData.lightSubScater;
	}

	void Renderer3D::enableSSAO(bool ssao)
	{
		internal.lightShader.useSSAO = ssao;
	}

	bool &Renderer3D::isSSAOenabeled()
	{
		return internal.lightShader.useSSAO;
	}

	float &Renderer3D::getSSAOBias()
	{
		return internal.ssao.ssaoShaderUniformBlockData.bias;
	}

	void Renderer3D::setSSAOBias(float bias)
	{
		internal.ssao.ssaoShaderUniformBlockData.bias = std::max(bias, 0.f);
	}

	float &Renderer3D::getSSAORadius()
	{
		return internal.ssao.ssaoShaderUniformBlockData.radius;
	}

	void Renderer3D::setSSAORadius(float radius)
	{
		internal.ssao.ssaoShaderUniformBlockData.radius = std::max(radius, 0.01f);
	}

	int &Renderer3D::getSSAOSampleCount()
	{
		return internal.ssao.ssaoShaderUniformBlockData.samplesTestSize;

	}

	void Renderer3D::setSSAOSampleCount(int samples)
	{
		internal.ssao.ssaoShaderUniformBlockData.samplesTestSize = std::min(std::max(samples, 5), 64);
	}

	float &Renderer3D::getSSAOExponent()
	{
		return internal.ssao.ssao_finalColor_exponent;
	}

	void Renderer3D::setSSAOExponent(float exponent)
	{
		internal.ssao.ssao_finalColor_exponent = std::min(std::max(1.f, exponent), 64.f);
	}

	void Renderer3D::enableFXAA(bool fxaa)
	{
		this->antiAlias.usingFXAA = fxaa;
	}

	Renderer3D::FXAAData& Renderer3D::getFxaaSettings()
	{
		return antiAlias.fxaaData;
	}

	bool &Renderer3D::isFXAAenabeled()
	{
		return antiAlias.usingFXAA;
	}

	//todo look into  glProgramUniform
	//in order to send less stuff tu uniforms

	//todo look into
	//ATI/AMD created GL_ATI_meminfo. This extension is very easy to use. 
	//You basically need to call glGetIntegerv with the appropriate token values.
	//https://www.khronos.org/registry/OpenGL/extensions/ATI/ATI_meminfo.txt
	//http://developer.download.nvidia.com/opengl/specs/GL_NVX_gpu_memory_info.txt
	
	void Renderer3D::renderModelNormals(Model o, glm::vec3 position, glm::vec3 rotation,
		glm::vec3 scale, float normalSize, glm::vec3 normalColor)
	{
		auto obj = internal.getModelData(o);

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
			
		internal.showNormalsProgram.shader.bind();
		
		auto projMat = camera.getProjectionMatrix();
		auto viewMat = camera.getWorldToViewMatrix();
		auto transformMat = gl3d::getTransformMatrix(position, rotation, scale);
		
		auto viewTransformMat = viewMat * transformMat;
		
		glUniformMatrix4fv(internal.showNormalsProgram.modelTransformLocation,
			1, GL_FALSE, &viewTransformMat[0][0]);
		
		glUniformMatrix4fv(internal.showNormalsProgram.projectionLocation,
			1, GL_FALSE, &projMat[0][0]);
		
		glUniform1f(internal.showNormalsProgram.sizeLocation, normalSize);

		glUniform3fv(internal.showNormalsProgram.colorLocation, 1, &(normalColor[0]));

		auto modelIndex = this->internal.getModelIndex(o);

		auto obj = internal.getModelData(o);
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
		if (id <= 0) { return -1; }

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
		if (id <= 0) { return -1; }

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
		if (id <= 0) { return -1; }

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
		if (id <= 0) { return -1; }

		auto found = std::find(entitiesIndexes.begin(), entitiesIndexes.end(), id);
		if (found == entitiesIndexes.end())
		{
			gl3dAssertComment(found != entitiesIndexes.end(), "invalid entity");
			return -1;
		}
		id = found - entitiesIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getSpotLightIndex(SpotLight l)
	{
		int id = l.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(spotLightIndexes.begin(), spotLightIndexes.end(), id);
		if (found == spotLightIndexes.end())
		{
			gl3dAssertComment(found != spotLightIndexes.end(), "invalid spot light");
			return -1;
		}
		id = found - spotLightIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getPointLightIndex(PointLight l)
	{
		int id = l.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(pointLightIndexes.begin(), pointLightIndexes.end(), id);
		if (found == pointLightIndexes.end())
		{
			gl3dAssertComment(found != pointLightIndexes.end(), "invalid point light");
			return -1;
		}
		id = found - pointLightIndexes.begin();

		return id;
	}

	int Renderer3D::InternalStruct::getDirectionalLightIndex(DirectionalLight l)
	{
		int id = l.id_;
		if (id <= 0) { return -1; }

		auto found = std::find(directionalLightIndexes.begin(), directionalLightIndexes.end(), id);
		if (found == directionalLightIndexes.end())
		{
			gl3dAssertComment(found != directionalLightIndexes.end(), "invalid directional light");
			return -1;
		}
		id = found - directionalLightIndexes.begin();

		return id;
	}

	bool Renderer3D::InternalStruct::getMaterialData(Material m, MaterialValues* gpuMaterial, std::string* name, TextureDataForMaterial* textureData)
	{
		int id = getMaterialIndex(m);

		if (id == -1)
		{
			return false;
		}

		if (gpuMaterial)
		{
			*gpuMaterial = materials[id];
		}

		if (name)
		{
			*name = materialNames[id];
		}

		if (textureData)
		{
			*textureData = materialTexturesData[id];
		}

		return true;
	}

	ModelData* Renderer3D::InternalStruct::getModelData(Model o)
	{
		int id = getModelIndex(o);

		if (id == -1)
		{
			return nullptr;
		}

		auto data = &graphicModels[id];
		return data;
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

	bool shouldCullObject(glm::vec3 minBoundary, glm::vec3 maxBoundary, glm::mat4& modelViewProjMat)
	{
		glm::vec3 cubePoints[8] = {};

		int c = 0;
		for (int x = 0; x < 2; x++)
			for (int y = 0; y < 2; y++)
				for (int z = 0; z < 2; z++)
				{
					float xVal = x ? minBoundary.x : maxBoundary.x;
					float yVal = y ? minBoundary.y : maxBoundary.y;
					float zVal = z ? minBoundary.z : maxBoundary.z;

					cubePoints[c] = { xVal, yVal, zVal };

					glm::vec4 augmentedPoint = glm::vec4(cubePoints[c], 1.f);

					augmentedPoint = modelViewProjMat * augmentedPoint;

					augmentedPoint.x /= augmentedPoint.w;
					augmentedPoint.y /= augmentedPoint.w;
					augmentedPoint.z /= augmentedPoint.w;
					
					if (augmentedPoint.z > 1.f)
					{
						augmentedPoint.y = -augmentedPoint.y;
					}
					
					cubePoints[c] = glm::vec3(augmentedPoint);

					c++;
				}

		int cull = 1;

		cull = 1;
		for (int p = 0; p < 8; p++)
		{
			if (cubePoints[p].z <= 1.f) { cull = 0; break; }
		}
		if (cull) { return true; }

		cull = 1;
		for (int p = 0; p < 8; p++)
		{
			if (cubePoints[p].y <= 1.f) { cull = 0; break; }
		}
		if (cull) { return true; }

		cull = 1;
		for (int p = 0; p < 8; p++)
		{
			if (cubePoints[p].y >= -1.f) { cull = 0; break; }
		}
		if (cull) { return true; }

		cull = 1;
		for (int p = 0; p < 8; p++)
		{
			if (cubePoints[p].x <= 1.f) { cull = 0; break; }
		}
		if (cull) { return true; }

		cull = 1;
		for (int p = 0; p < 8; p++)
		{
			if (cubePoints[p].x >= -1.f) { cull = 0; break; }
		}
		if (cull) { return true; }

		cull = 1;
		for (int p = 0; p < 8; p++)
		{
			if (cubePoints[p].z >= 0.f) { cull = 0; break; }
		}
		if (cull) { return true; }

		return false;
	}

	void applyPoseToJoints(
		std::vector<glm::mat4> &skinningMatrixes,
		std::vector<glm::mat4> &appliedSkinningMatrixes,
		std::vector<Joint> &joints, int index,
		glm::mat4 parentTransform
	)
	{
		auto currentLocalTransform = skinningMatrixes[index];
		auto worldSpaceTransform = parentTransform * currentLocalTransform;

		auto &j = joints[index];
		for (auto &c : j.children)
		{
			applyPoseToJoints(skinningMatrixes, appliedSkinningMatrixes, joints, c, worldSpaceTransform);
		}

		appliedSkinningMatrixes[index] = worldSpaceTransform * j.inverseBindTransform;
		//appliedSkinningMatrixes[index] = glm::mat4(1.f);

	};


	void Renderer3D::render(float deltaTime)
	{
	
		if (internal.w == 0 || internal.h == 0)
		{
			return;
		}

		camera.aspectRatio = (float)internal.w / internal.h;


		#pragma region adaptive rezolution

			if (adaptiveResolution.timeSample >= adaptiveResolution.timeSamplesCount)
			{
				float ms = 0;
				for (int i = 0; i < adaptiveResolution.timeSamplesCount; i++)
				{
					ms += adaptiveResolution.msSampled[i];
				}
				ms /= adaptiveResolution.timeSamplesCount;
				float seconds = ms * 1000;

				if (seconds < adaptiveResolution.stepUpSecTarget)
				{
					adaptiveResolution.rezRatio += 0.1f;

					if (adaptiveResolution.rezRatio >= 1.f)
					{
						adaptiveResolution.rezRatio = 1.f;
						adaptiveResolution.shouldUseAdaptiveResolution = false;
					}
				}
				else if(seconds > adaptiveResolution.stepDownSecTarget)
				{
					adaptiveResolution.rezRatio -= 0.1f;
					if (adaptiveResolution.rezRatio <= adaptiveResolution.maxScaleDown)
					{
						adaptiveResolution.rezRatio = adaptiveResolution.maxScaleDown;
					}

					adaptiveResolution.shouldUseAdaptiveResolution = true;
				}
				adaptiveResolution.timeSample = 0;
			}
			else
			{
				adaptiveResolution.msSampled[adaptiveResolution.timeSample] = deltaTime;
				adaptiveResolution.timeSample++;
			}

			updateWindowMetrics(internal.w, internal.h);
		

		#pragma endregion


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glStencilMask(0xFF);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDepthFunc(GL_LESS);


		if (antiAlias.usingFXAA || adaptiveResolution.useAdaptiveResolution)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, adaptiveResolution.fbo);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, postProcess.fbo);
		glClear(GL_COLOR_BUFFER_BIT);

		glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);
		internal.renderSkyBoxBefore(camera, skyBox);

		auto worldToViewMatrix		= camera.getWorldToViewMatrix();
		auto projectionMatrix		= camera.getProjectionMatrix();
		auto worldProjectionMatrix	= projectionMatrix * worldToViewMatrix;

		#pragma region check camera changes

			bool cameraChanged = false;
			if (camera != internal.lastFrameCamera)
			{
				cameraChanged = true;
			}
			internal.lastFrameCamera = camera;

		#pragma endregion

		
		#pragma region animations
			for (auto &entity : internal.cpuEntities)
			{
				if (!entity.isVisible())
				{
					continue;
				}

				if (entity.models.empty())
				{
					continue;
				}

				if (entity.canBeAnimated() && entity.animate())
				{

					int index = entity.animationIndex;
					index = std::min(index, (int)entity.animations.size() - 1);
					auto &animation = entity.animations[index];

					std::vector<glm::mat4> skinningMatrixes;
					skinningMatrixes.resize(entity.joints.size(), glm::mat4(1.f));

					entity.totalTimePassed += deltaTime * entity.animationSpeed;
					while (entity.totalTimePassed >= animation.animationDuration)
					{
						entity.totalTimePassed -= animation.animationDuration;
					}

					for (int b = 0; b < entity.joints.size(); b++)
					{

						glm::mat4 rotMat(1.f);
						glm::mat4 transMat(1.f);
						glm::mat4 scaleMat(1.f);

						auto &joint = entity.joints[b];

						if (
							animation.keyFramesRot[b].empty() &&
							animation.keyFramesTrans[b].empty() &&
							animation.keyFramesScale[b].empty()
							)
						{
							skinningMatrixes[b] = joint.localBindTransform; //no animations at all here
						}
						else
						{
							if (!animation.keyFramesRot[b].empty()) //no key frames for this bone...
							{
								for (int frame = animation.keyFramesRot[b].size() - 1; frame >= 0; frame--)
								{
									int frames = animation.keyFramesRot[b].size();
									float time = entity.totalTimePassed;
									auto &currentFrame = animation.keyFramesRot[b][frame];
									if (time >= currentFrame.timeStamp)
									{
										auto first = currentFrame.rotation;

										if (frame == animation.keyFramesRot[b].size() - 1)
										{
											rotMat = glm::toMat4(first);
										}
										else
										{
											auto second = animation.keyFramesRot[b][frame + 1].rotation;
											float secondTime = animation.keyFramesRot[b][frame + 1].timeStamp;
											float interpolation = (time - currentFrame.timeStamp) / (secondTime - currentFrame.timeStamp);
											rotMat = glm::toMat4(glm::slerp(first, second, interpolation));
										}
										break;

									}

								}
								//rotMat = glm::toMat4(i.animation.keyFramesRot[b][0].rotation);
							}
							else
							{
								rotMat = glm::toMat4(joint.rotation);
							}

							auto lerp = [](glm::vec3 a, glm::vec3 b, float x) -> glm::vec3
							{
								return a * (1.f - x) + (b * x);
							};

							if (!animation.keyFramesTrans[b].empty()) //no key frames for this bone...
							{
								for (int frame = animation.keyFramesTrans[b].size() - 1; frame >= 0; frame--)
								{
									int frames = animation.keyFramesTrans[b].size();
									float time = entity.totalTimePassed;
									auto &currentFrame = animation.keyFramesTrans[b][frame];
									if (time >= currentFrame.timeStamp)
									{
										auto first = currentFrame.translation;

										if (frame == animation.keyFramesTrans[b].size() - 1)
										{
											transMat = glm::translate(first);
										}
										else
										{
											auto second = animation.keyFramesTrans[b][frame + 1].translation;
											float secondTime = animation.keyFramesTrans[b][frame + 1].timeStamp;
											float interpolation = (time - currentFrame.timeStamp) / (secondTime - currentFrame.timeStamp);
											transMat = glm::translate(lerp(first, second, interpolation));
										}
										break;
									}
								}
								//transMat = glm::translate(i.animation.keyFramesTrans[b][0].translation);
							}
							else
							{
								transMat = glm::translate(joint.trans);
							}

							if (!animation.keyFramesScale[b].empty()) //no key frames for this bone...
							{
								for (int frame = animation.keyFramesScale[b].size() - 1; frame >= 0; frame--)
								{
									int frames = animation.keyFramesScale[b].size();
									float time = entity.totalTimePassed;
									auto &currentFrame = animation.keyFramesScale[b][frame];
									if (time >= currentFrame.timeStamp)
									{
										auto first = currentFrame.scale;

										if (frame == animation.keyFramesScale[b].size() - 1)
										{
											scaleMat = glm::translate(first);
										}
										else
										{
											auto second = animation.keyFramesScale[b][frame + 1].scale;
											float secondTime = animation.keyFramesScale[b][frame + 1].timeStamp;
											float interpolation = (time - currentFrame.timeStamp) / (secondTime - currentFrame.timeStamp);
											scaleMat = glm::scale(lerp(first, second, interpolation));
										}
										break;
									}
								}
								//scaleMat = glm::scale(i.animation.keyFramesScale[b][0].scale);
							}
							else
							{
								scaleMat = glm::scale(joint.scale);
							}

							skinningMatrixes[b] = transMat * rotMat * scaleMat;
							//skinningMatrixes[b] = i.joints[b].localBindTransform; //no animations

						}

					}

					//skinningMatrixes[24] = skinningMatrixes[24] * glm::rotate(glm::radians(90.f), glm::vec3{ 1,0,0 });
					//std::vector<glm::mat4> appliedSkinningMatrixes;
					std::vector<glm::mat4> appliedSkinningMatrixes;
					appliedSkinningMatrixes.clear();
					appliedSkinningMatrixes.resize(entity.joints.size(), glm::mat4(1.f));

					for (auto r : animation.root)
					{
						applyPoseToJoints(skinningMatrixes, appliedSkinningMatrixes, entity.joints,
							r, glm::mat4(1.f));
					}
					
					#pragma region save animation data to the gpu
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, entity.appliedSkinningMatricesBuffer);
					glBufferData(GL_SHADER_STORAGE_BUFFER, appliedSkinningMatrixes.size() * sizeof(glm::mat4),
						&appliedSkinningMatrixes[0], GL_STREAM_DRAW);
					#pragma endregion

				}

			}
		#pragma endregion


		#pragma region render shadow maps
			//filter true and onlyStatic true renders only static geometry
			auto renderModelsShadows = [&](glm::mat4& lightSpaceMatrix, bool filter =0, bool onlyStatic=0)
			{
				//render shadow of the models
				for (auto& i : internal.cpuEntities)
				{

					if (!i.isVisible() || !i.castShadows())
					{
						continue;
					}

					if (filter)
					{
						if (onlyStatic != i.isStatic())
						{
							continue;
						}
					}

					auto transformMat = i.transform.getTransformMatrix();
					auto modelViewProjMat = lightSpaceMatrix * transformMat;

					glUniformMatrix4fv(internal.lightShader.prePass.u_transform, 1, GL_FALSE,
						&modelViewProjMat[0][0]);

					bool potentialAnimations = false;
					if (!i.canBeAnimated() || !i.animate())
					{
						glUniform1i(internal.lightShader.prePass.u_hasAnimations, false);
					}
					else
					{
						//send animation data
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::JointsTransformBlockBinding,
							i.appliedSkinningMatricesBuffer);
						potentialAnimations = true;
					}

					for (auto& j : i.models)
					{
						if (!(potentialAnimations && j.hasBones))
						{
							if (shouldCullObject(j.minBoundary, j.maxBoundary, modelViewProjMat))
							{
								continue;
							}
						}

						if (potentialAnimations)
						{
							if (j.hasBones)
							{
								glUniform1i(internal.lightShader.prePass.u_hasAnimations, true);
							}
							else
							{
								glUniform1i(internal.lightShader.prePass.u_hasAnimations, false);
							}
						}

						auto m = internal.getMaterialIndex(j.material);

						if (m < 0)
						{
							glUniform1i(internal.lightShader.prePass.u_hasTexture, 0);
						}
						else
						{

							auto t = internal.materialTexturesData[m];
							auto tId = internal.getTextureIndex(t.albedoTexture);

							if (tId < 0)
							{
								glUniform1i(internal.lightShader.prePass.u_hasTexture, 0);
							}
							else
							{
								auto texture = internal.loadedTextures[tId];

								glUniform1i(internal.lightShader.prePass.u_hasTexture, 1);
								glUniform1i(internal.lightShader.prePass.u_albedoSampler, 0);

								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, texture.texture.id);
							}
						}

						glBindVertexArray(j.vertexArray);

						if (j.indexBuffer)
						{
							glDrawElements(GL_TRIANGLES, j.primitiveCount, GL_UNSIGNED_INT, 0);
						}
						else
						{
							glDrawArrays(GL_TRIANGLES, 0, j.primitiveCount);
						}
					}

				}
			};
			
			auto renderModelsPointShadows = [&](int lightIndex, int shadowCastIndex, bool filter = 0, bool onlyStatic = 0)
		{
			glUniform1i(internal.lightShader.pointShadowShader.u_lightIndex, shadowCastIndex);

			glm::mat4 shadowProj = glm::perspective(glm::radians(90.f), 1.f, 0.1f,
				internal.pointLights[lightIndex].dist);
			glm::vec3 lightPos = internal.pointLights[lightIndex].position;

			std::vector<glm::mat4> shadowTransforms;
			shadowTransforms.reserve(6);
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

			glUniformMatrix4fv(internal.lightShader.pointShadowShader.u_shadowMatrices, 6, GL_FALSE,
				&(*shadowTransforms.data())[0][0]);

			glUniform3fv(internal.lightShader.pointShadowShader.u_lightPos, 1,
				&lightPos[0]);

			glUniform1f(internal.lightShader.pointShadowShader.u_farPlane,
				internal.pointLights[lightIndex].dist);

			//render shadow of the models
			for (auto& i : internal.cpuEntities)
			{

				if (!i.isVisible() || !i.castShadows())
				{
					continue;
				}

				if (filter)
				{
					if (onlyStatic != i.isStatic())
					{
						continue;
					}
				}

				auto transformMat = i.transform.getTransformMatrix();

				glUniformMatrix4fv(internal.lightShader.pointShadowShader.u_transform, 1, GL_FALSE,
					&transformMat[0][0]);

				if (!i.canBeAnimated() || !i.animate())
				{
					glUniform1i(internal.lightShader.pointShadowShader.u_hasAnimations, false);
				}
				else
				{
					//send animation data
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::JointsTransformBlockBinding,
						i.appliedSkinningMatricesBuffer);
				}

				for (auto& j : i.models)
				{

					if (i.canBeAnimated() && i.animate())
					{
						if (j.hasBones)
						{
							glUniform1i(internal.lightShader.pointShadowShader.u_hasAnimations, true);
						}
						else
						{
							glUniform1i(internal.lightShader.pointShadowShader.u_hasAnimations, false);
						}
					}

					auto m = internal.getMaterialIndex(j.material);

					if (m < 0)
					{
						glUniform1i(internal.lightShader.pointShadowShader.u_hasTexture, 0);
					}
					else
					{

						auto t = internal.materialTexturesData[m];
						auto tId = internal.getTextureIndex(t.albedoTexture);

						if (tId < 0)
						{
							glUniform1i(internal.lightShader.pointShadowShader.u_hasTexture, 0);
						}
						else
						{
							auto texture = internal.loadedTextures[tId];

							glUniform1i(internal.lightShader.pointShadowShader.u_hasTexture, 1);
							glUniform1i(internal.lightShader.pointShadowShader.u_albedoSampler, 0);

							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, texture.texture.id);
						}
					}

					glBindVertexArray(j.vertexArray);

					if (j.indexBuffer)
					{
						glDrawElements(GL_TRIANGLES, j.primitiveCount, GL_UNSIGNED_INT, 0);
					}
					else
					{
						glDrawArrays(GL_TRIANGLES, 0, j.primitiveCount);
					}
				}

			}
		};

			if (internal.pointLights.size())
		{
			int shouldUpdateAllPointShadows = internal.perFrameFlags.staticGeometryChanged
				|| internal.perFrameFlags.shouldUpdatePointShadows;

			int pointLightsShadowsCount = 0;
			for (auto& i :internal.pointLights)
			{
				if (i.castShadows != 0)
				{
					i.castShadowsIndex = pointLightsShadowsCount;
					pointLightsShadowsCount++;
				}

			}
			
			if (pointLightsShadowsCount)
			{
				if (pointLightsShadowsCount != pointShadows.textureCount
					|| pointShadows.currentShadowSize != pointShadows.shadowSize
					)
				{
					pointShadows.allocateTextures(pointLightsShadowsCount);
					shouldUpdateAllPointShadows = true;
				}

				internal.lightShader.pointShadowShader.shader.bind();
				glViewport(0, 0, pointShadows.shadowSize, pointShadows.shadowSize);


				//static geometry
				glBindFramebuffer(GL_FRAMEBUFFER, pointShadows.staticGeometryFbo);
				for (int lightIndex = 0; lightIndex < internal.pointLights.size(); lightIndex++)
				{
					if (internal.pointLights[lightIndex].castShadows == 0)
					{
						continue;
					}

					if (internal.pointLights[lightIndex].changedThisFrame
						|| shouldUpdateAllPointShadows
						)
					{
						internal.pointLights[lightIndex].changedThisFrame = false;


						for (int i = 0; i < 6; i++)
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
								pointShadows.staticGeometryTextures, 0,
								internal.pointLights[lightIndex].castShadowsIndex * 6 + i);
							glClear(GL_DEPTH_BUFFER_BIT);
						}

						glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							pointShadows.staticGeometryTextures, 0);


						renderModelsPointShadows(lightIndex,
							internal.pointLights[lightIndex].castShadowsIndex, true, true);
					}

				}

				//copy static geometry
				glCopyImageSubData(pointShadows.staticGeometryTextures, GL_TEXTURE_CUBE_MAP_ARRAY, 0,
					0, 0, 0,
					pointShadows.shadowTextures, GL_TEXTURE_CUBE_MAP_ARRAY, 0,
					0, 0, 0,
					pointShadows.shadowSize, pointShadows.shadowSize, pointShadows.textureCount * 6
				);

				//dynamic geometry
				glBindFramebuffer(GL_FRAMEBUFFER, pointShadows.fbo);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointShadows.shadowTextures, 0);
				//glClear(GL_DEPTH_BUFFER_BIT);

				for (int lightIndex = 0; lightIndex < internal.pointLights.size(); lightIndex++)
				{
					if (internal.pointLights[lightIndex].castShadows == 0)
					{
						continue;
					}


					renderModelsPointShadows(lightIndex,
						internal.pointLights[lightIndex].castShadowsIndex, true, false);

				}

			}

			
		}

			internal.lightShader.prePass.shader.bind();

			if (internal.directionalLights.size())
			{

				bool shouldRenderStaticGeometryAllLights = internal.perFrameFlags.staticGeometryChanged
					|| internal.perFrameFlags.shouldUpdateDirectionalShadows
					|| cameraChanged;

				int directionalLightsShadows = 0;
				for (int i=0; i< internal.directionalLights.size(); i++)
				{
					if (internal.directionalLights[i].castShadows != 0) 
					{ 
						internal.directionalLights[i].castShadowsIndex = directionalLightsShadows;
						directionalLightsShadows++; 
					}
				}

				if (directionalLightsShadows != directionalShadows.textureCount
					|| directionalShadows.shadowSize != directionalShadows.currentShadowSize
					)
				{
					directionalShadows.allocateTextures(directionalLightsShadows);
					shouldRenderStaticGeometryAllLights = true;
				}
				

				auto calculateLightProjectionMatrix = [&](glm::vec3 lightDir, glm::mat4 lightView, 
					float nearPlane, float farPlane,
					float zOffset)
				{
					glm::vec3 rVector = {};
					glm::vec3 upVectpr = {};
					generateTangentSpace(lightDir, upVectpr, rVector);

					glm::vec2 nearDimensions{};
					glm::vec2 farDimensions{};
					glm::vec3 centerNear{};
					glm::vec3 centerFar{};

					computeFrustumDimensions(camera.position, camera.viewDirection, camera.fovRadians, camera.aspectRatio,
						nearPlane, farPlane, nearDimensions, farDimensions, centerNear, centerFar);

					glm::vec3 nearTopLeft{};
					glm::vec3 nearTopRight{};
					glm::vec3 nearBottomLeft{};
					glm::vec3 nearBottomRight{};
					glm::vec3 farTopLeft{};
					glm::vec3 farTopRight{};
					glm::vec3 farBottomLeft{};
					glm::vec3 farBottomRight{};

					computeFrustumSplitCorners(camera.viewDirection, nearDimensions, farDimensions, centerNear, centerFar,
						nearTopLeft,
						nearTopRight,
						nearBottomLeft,
						nearBottomRight,
						farTopLeft,
						farTopRight,
						farBottomLeft,
						farBottomRight
					);


					glm::vec3 corners[] =
					{
						nearTopLeft,
						nearTopRight,
						nearBottomLeft,
						nearBottomRight,
						farTopLeft,
						farTopRight,
						farBottomLeft,
						farBottomRight,
					};

					float longestDiagonal = glm::distance(nearTopLeft, farBottomRight);

					glm::vec3 minVal{};
					glm::vec3 maxVal{};

					for (int i = 0; i < 8; i++)
					{
						glm::vec4 corner(corners[i], 1);

						glm::vec4 lightViewCorner = lightView * corner;

						if (i == 0)
						{
							minVal = lightViewCorner;
							maxVal = lightViewCorner;
						}
						else
						{
							if (lightViewCorner.x < minVal.x) { minVal.x = lightViewCorner.x; }
							if (lightViewCorner.y < minVal.y) { minVal.y = lightViewCorner.y; }
							if (lightViewCorner.z < minVal.z) { minVal.z = lightViewCorner.z; }

							if (lightViewCorner.x > maxVal.x) { maxVal.x = lightViewCorner.x; }
							if (lightViewCorner.y > maxVal.y) { maxVal.y = lightViewCorner.y; }
							if (lightViewCorner.z > maxVal.z) { maxVal.z = lightViewCorner.z; }

						}

					}

					//keep them square and the same size:
					//https://www.youtube.com/watch?v=u0pk1LyLKYQ&t=99s&ab_channel=WesleyLaFerriere
					if (1)
					{
						float firstSize = maxVal.x - minVal.x;
						float secondSize = maxVal.y - minVal.y;
						float thirdSize = maxVal.z - minVal.z;

						{
							float ratio = longestDiagonal / firstSize;

							glm::vec2 newVecValues = { minVal.x, maxVal.x };
							float dimension = firstSize;
							float dimensionOver2 = dimension / 2.f;

							newVecValues -= glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);
							newVecValues *= ratio;
							newVecValues += glm::vec2(minVal.x + dimensionOver2, minVal.x + dimensionOver2);

							minVal.x = newVecValues.x;
							maxVal.x = newVecValues.y;
						}

						{
							float ratio = longestDiagonal / secondSize;

							glm::vec2 newVecValues = { minVal.y, maxVal.y };
							float dimension = secondSize;
							float dimensionOver2 = dimension / 2.f;

							newVecValues -= glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);
							newVecValues *= ratio;
							newVecValues += glm::vec2(minVal.y + dimensionOver2, minVal.y + dimensionOver2);

							minVal.y = newVecValues.x;
							maxVal.y = newVecValues.y;
						}

						{//todo this size probably can be far-close
							float ratio = longestDiagonal / thirdSize;

							glm::vec2 newVecValues = { minVal.z, maxVal.z };
							float dimension = thirdSize;
							float dimensionOver2 = dimension / 2.f;

							newVecValues -= glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);
							newVecValues *= ratio;
							newVecValues += glm::vec2(minVal.z + dimensionOver2, minVal.z + dimensionOver2);

							minVal.z = newVecValues.x;
							maxVal.z = newVecValues.y;
						}

					}

					float near_plane = minVal.z - zOffset;
					float far_plane = maxVal.z;


					glm::vec2 ortoMin = { minVal.x, minVal.y };
					glm::vec2 ortoMax = { maxVal.x, maxVal.y };

					//remove shadow flicker
					if (1)
					{
						glm::vec2 shadowMapSize(directionalShadows.shadowSize, directionalShadows.shadowSize);
						glm::vec2 worldUnitsPerTexel = (ortoMax - ortoMin) / shadowMapSize;

						ortoMin /= worldUnitsPerTexel;
						ortoMin = glm::floor(ortoMin);
						ortoMin *= worldUnitsPerTexel;

						ortoMax /= worldUnitsPerTexel;
						ortoMax = glm::floor(ortoMax);
						ortoMax *= worldUnitsPerTexel;

						float zWorldUnitsPerTexel = (far_plane - near_plane) / directionalShadows.shadowSize;

						near_plane /= zWorldUnitsPerTexel;
						far_plane /= zWorldUnitsPerTexel;
						near_plane = glm::floor(near_plane);
						far_plane = glm::floor(far_plane);
						near_plane *= zWorldUnitsPerTexel;
						far_plane *= zWorldUnitsPerTexel;

					}

					glm::mat4 lightProjection = glm::ortho(ortoMin.x, ortoMax.x, ortoMin.y, ortoMax.y, near_plane, far_plane);

					return lightProjection;

				};

				if (directionalLightsShadows)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, directionalShadows.staticGeometryFbo);
					for (int lightIndex = 0; lightIndex < internal.directionalLights.size(); lightIndex++)
					{
						if (internal.directionalLights[lightIndex].castShadows != 0
							&& (internal.directionalLights[lightIndex].changedThisFrame
							|| shouldRenderStaticGeometryAllLights)
							)
						{
							internal.directionalLights[lightIndex].changedThisFrame = false;

							glm::vec3 lightDir = internal.directionalLights[lightIndex].direction;
							//glm::mat4 lightView = lookAtSafe(-lightDir, {}, { 0.f,1.f,0.f });

							glm::mat4 lightView = lookAtSafe(camera.position - (lightDir), camera.position, { 0.f,1.f,0.f });
							//glm::mat4 lightView = lookAtSafe(camera.position, camera.position + lightDir, { 0.f,1.f,0.f });

							//zoffset is used to move the light further


							float zOffsets[] = { 15 / 200.f,0,0 };

							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
								directionalShadows.staticGeometryTexture, 0,
								internal.directionalLights[lightIndex].castShadowsIndex); //last is layer

							glClear(GL_DEPTH_BUFFER_BIT);
							float lastNearPlane = 0.0001;

							for (int i = 0; i < DirectionalShadows::CASCADES; i++)
							{
								glViewport(0, directionalShadows.shadowSize * i,
									directionalShadows.shadowSize, directionalShadows.shadowSize);

								auto projection = calculateLightProjectionMatrix(lightDir, lightView,
									lastNearPlane,
									directionalShadows.frustumSplits[i] * camera.farPlane,
									zOffsets[i] * camera.farPlane);

								//this will add some precision but add artefacts todo?
								//lastNearPlane = zOffsets[i] * camera.farPlane;

								internal.directionalLights[lightIndex].lightSpaceMatrix[i] = projection * lightView;

								renderModelsShadows(internal.directionalLights[lightIndex].lightSpaceMatrix[i],
									true, true);

							}
						}

					}

					//copy static geometry
					glCopyImageSubData(directionalShadows.staticGeometryTexture, GL_TEXTURE_2D_ARRAY, 0,
						0, 0, 0,
						directionalShadows.cascadesTexture, GL_TEXTURE_2D_ARRAY, 0,
						0, 0, 0,
						directionalShadows.shadowSize, directionalShadows.shadowSize * directionalShadows.CASCADES,
						directionalLightsShadows
					);

					glBindFramebuffer(GL_FRAMEBUFFER, directionalShadows.cascadesFbo);
					
					for (int lightIndex = 0; lightIndex < internal.directionalLights.size(); lightIndex++)
					{
						if (internal.directionalLights[lightIndex].castShadows != 0)
						{

							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
								directionalShadows.cascadesTexture, 0, 
								internal.directionalLights[lightIndex].castShadowsIndex); //last is layer
							//glClear(GL_DEPTH_BUFFER_BIT);

							for (int i = 0; i < DirectionalShadows::CASCADES; i++)
							{
								glViewport(0, directionalShadows.shadowSize * i,
									directionalShadows.shadowSize, directionalShadows.shadowSize);

								renderModelsShadows(internal.directionalLights[lightIndex].lightSpaceMatrix[i],
									true, false);

							}

						}


					}
				}
				
			}

			if (internal.spotLights.size())
			{
				bool shouldRenderStaticGeometryAllLights = internal.perFrameFlags.staticGeometryChanged
					|| internal.perFrameFlags.shouldUpdateSpotShadows;

				int spotLightsShadowsCount = 0;
				for (const auto& i : internal.spotLights)
				{
					if (i.castShadows) { spotLightsShadowsCount++; }
				}

				if (spotLightsShadowsCount != spotShadows.textureCount
					|| spotShadows.shadowSize != spotShadows.currentShadowSize
					)
				{
					spotShadows.allocateTextures(spotLightsShadowsCount);
					shouldRenderStaticGeometryAllLights = true; 
				}

				if (spotLightsShadowsCount)
				{
					glViewport(0, 0, spotShadows.shadowSize, spotShadows.shadowSize);

					glBindFramebuffer(GL_FRAMEBUFFER, spotShadows.staticGeometryfbo);

					int shadowCastCount = 0;
					for (int lightIndex = 0; lightIndex < internal.spotLights.size(); lightIndex++)
					{
						if (internal.spotLights[lightIndex].castShadows)
						{
							if (shouldRenderStaticGeometryAllLights || internal.spotLights[lightIndex].changedThisFrame)
							{

								glm::vec3 lightDir = internal.spotLights[lightIndex].direction;
								glm::vec3 lightPos = internal.spotLights[lightIndex].position;
								glm::mat4 lightView = lookAtSafe(lightPos, lightPos + lightDir, { 0.f,1.f,0.f });
								float fov = internal.spotLights[lightIndex].cosHalfAngle;
								fov = std::acos(fov);
								fov *= 2;

								float nearPlane = 0.01f;
								float farPlane = internal.spotLights[lightIndex].dist;

								auto projection = glm::perspective(fov, 1.f, nearPlane, farPlane);
								internal.spotLights[lightIndex].lightSpaceMatrix = projection * lightView;
								internal.spotLights[lightIndex].shadowIndex = shadowCastCount;

								internal.spotLights[lightIndex].nearPlane = nearPlane;
								internal.spotLights[lightIndex].farPlane = farPlane;

								internal.spotLights[lightIndex].changedThisFrame = false;

								glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
									spotShadows.staticGeometryTextures, 0, shadowCastCount);
								glClear(GL_DEPTH_BUFFER_BIT);
								//render only static geometry first
								renderModelsShadows(internal.spotLights[lightIndex].lightSpaceMatrix, true, true);

							}

							shadowCastCount++;
						}

					}

					//copy static geometry
					glCopyImageSubData(spotShadows.staticGeometryTextures, GL_TEXTURE_2D_ARRAY, 0,
						0, 0, 0,
						spotShadows.shadowTextures, GL_TEXTURE_2D_ARRAY, 0,
						0, 0, 0,
						spotShadows.shadowSize, spotShadows.shadowSize, spotLightsShadowsCount
					);

					//render dynamic geometry on top
					glBindFramebuffer(GL_FRAMEBUFFER, spotShadows.fbo);
					shadowCastCount = 0; //todo remove !!! bug here
					for (int lightIndex = 0; lightIndex < internal.spotLights.size(); lightIndex++)
					{
						if (internal.spotLights[lightIndex].castShadows)
						{
							glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
								spotShadows.shadowTextures, 0, shadowCastCount);

							renderModelsShadows(internal.spotLights[lightIndex].lightSpaceMatrix, true, false);
							shadowCastCount++;
						}

					}
				}


			}
		#pragma endregion

		
		glBindFramebuffer(GL_FRAMEBUFFER, internal.gBuffer.gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);


		#pragma region z pre pass and frustum culling
		if (zPrePass || frustumCulling)
		{
			if (zPrePass)
			{
				internal.lightShader.prePass.shader.bind();
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			}

			for (auto& i : internal.cpuEntities)
			{
				if (!i.isVisible())
				{
					continue;
				}

				bool potentialAnimations = 0;
				if (!i.canBeAnimated() || !i.animate())
				{
					glUniform1i(internal.lightShader.prePass.u_hasAnimations, false);
					
				}
				else
				{
					//send animation data
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::JointsTransformBlockBinding,
						i.appliedSkinningMatricesBuffer);
					potentialAnimations = true;
				}

				auto transformMat = i.transform.getTransformMatrix();
				auto modelViewProjMat = worldProjectionMatrix * transformMat;

				if (zPrePass)
				{
					glUniformMatrix4fv(internal.lightShader.prePass.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);
				
				}

				for (auto& j : i.models)
				{
					//frustum culling
					if (frustumCulling && !potentialAnimations && !j.hasBones)
					{

						if (shouldCullObject(j.minBoundary, j.maxBoundary, modelViewProjMat))
						{
							j.culledThisFrame = true;
							continue;
						}

					}

					j.culledThisFrame = false;

					if (zPrePass)
					{
						if (i.canBeAnimated() && i.animate())
						{
							if (j.hasBones)
							{
								glUniform1i(internal.lightShader.prePass.u_hasAnimations, true);
							}
							else
							{
								glUniform1i(internal.lightShader.prePass.u_hasAnimations, false);
							}
						}


						auto m = internal.getMaterialIndex(j.material);

						if (m < 0)
						{
							glUniform1i(internal.lightShader.prePass.u_hasTexture, 0);
						}
						else
						{

							auto t = internal.materialTexturesData[m];
							auto tId = internal.getTextureIndex(t.albedoTexture);

							if (tId < 0)
							{
								glUniform1i(internal.lightShader.prePass.u_hasTexture, 0);

							}
							else
							{
								auto texture = internal.loadedTextures[tId];

								glUniform1i(internal.lightShader.prePass.u_hasTexture, 1);
								glUniform1i(internal.lightShader.prePass.u_albedoSampler, 0);
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, texture.texture.id);
							}

						}

						glBindVertexArray(j.vertexArray);

						if (j.indexBuffer)
						{
							glDrawElements(GL_TRIANGLES, j.primitiveCount, GL_UNSIGNED_INT, 0);
						}
						else
						{
							glDrawArrays(GL_TRIANGLES, 0, j.primitiveCount);
						}
					}
					
				}

			}

			if (zPrePass)
			{
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			}
		}
		#pragma endregion 


		#pragma region stuff to be bound for rendering the geometry


		internal.lightShader.geometryPassShader.bind();
		internal.lightShader.getSubroutines();

		//glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		//glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(internal.lightShader.textureSamplerLocation, 0);
		glUniform1i(internal.lightShader.normalMapSamplerLocation, 1);
		//glUniform1i(lightShader.skyBoxSamplerLocation, 2);
		glUniform1i(internal.lightShader.RMASamplerLocation, 3);
		glUniform1i(internal.lightShader.u_emissiveTexture, 4);


		//material buffer
		if (internal.materials.size())
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, internal.lightShader.materialBlockBuffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MaterialValues) * internal.materials.size()
				, &internal.materials[0], GL_STREAM_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::MaterialBlockBinding,
				internal.lightShader.materialBlockBuffer);
		}

		GLsizei n;
		glGetProgramStageiv(internal.lightShader.geometryPassShader.id,
			GL_FRAGMENT_SHADER,
			GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS,
			&n);

		GLuint* indices = new GLuint[n]{ 0 };

		if (zPrePass)
		{
			glDepthFunc(GL_EQUAL);
		}
		else
		{
			glDepthFunc(GL_LESS);
		}

		#pragma endregion


		#pragma region g buffer render

		//first we render the entities in the gbuffer
		for (auto& entity : internal.cpuEntities)
		{
			if (!entity.isVisible())
			{
				continue;
			}

			if (entity.models.empty())
			{
				continue;
			}

			auto transformMat = entity.transform.getTransformMatrix();
			auto modelViewProjMat = worldProjectionMatrix * transformMat;

			glUniformMatrix4fv(internal.lightShader.u_transform, 1, GL_FALSE, &modelViewProjMat[0][0]);
			glUniformMatrix4fv(internal.lightShader.u_modelTransform, 1, GL_FALSE, &transformMat[0][0]);
			glUniformMatrix4fv(internal.lightShader.u_motelViewTransform, 1, GL_FALSE, &(worldToViewMatrix * transformMat)[0][0]);
			

			#pragma region send animation data
			if (entity.animate())
			{
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::JointsTransformBlockBinding,
					entity.appliedSkinningMatricesBuffer);
			}
			else
			{
				glUniform1i(internal.lightShader.u_hasAnimations, false);
			}
			#pragma endregion


			bool changed = 1;
			for (auto& i : entity.models)
			{

				if (frustumCulling && i.culledThisFrame)
				{
					continue;
				}

				int materialId = internal.getMaterialIndex(i.material);

				if (materialId == -1)
				{
					continue;
				}

				glUniform1i(internal.lightShader.materialIndexLocation, materialId);

				#pragma region animations
				if (entity.animate()) //if animations are off we set the uniform up
				{
					if (i.hasBones) //if the sub mesh has bones
					{
						glUniform1i(internal.lightShader.u_hasAnimations, true);
					}
					else
					{
						glUniform1i(internal.lightShader.u_hasAnimations, false);
					}
				}
				#pragma endregion


				TextureDataForMaterial textureData = internal.materialTexturesData[materialId];

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

				GpuTexture* rmaTextureData = this->getTextureData(textureData.pbrTexture.texture);
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
					if (indices[internal.lightShader.getEmmisiveSubroutineLocation] !=
						internal.lightShader.emissiveSubroutine_sampled)
					{
						indices[internal.lightShader.getEmmisiveSubroutineLocation] = 
							internal.lightShader.emissiveSubroutine_sampled;
						changed = 1;
					}
				}
				else
				{
					if (indices[internal.lightShader.getEmmisiveSubroutineLocation] != 
						internal.lightShader.emissiveSubroutine_notSampled)
					{
						indices[internal.lightShader.getEmmisiveSubroutineLocation] = 
							internal.lightShader.emissiveSubroutine_notSampled;
						changed = 1;
					}
				}

				if (normalLoaded && internal.lightShader.normalMap)
				{
					if (indices[internal.lightShader.normalSubroutineLocation] !=
						internal.lightShader.normalSubroutine_normalMap)
					{
						indices[internal.lightShader.normalSubroutineLocation] = 
							internal.lightShader.normalSubroutine_normalMap;
						changed = 1;
					}
				}
				else
				{
					if (indices[internal.lightShader.normalSubroutineLocation] != 
						internal.lightShader.normalSubroutine_noMap)
					{
						indices[internal.lightShader.normalSubroutineLocation] = 
							internal.lightShader.normalSubroutine_noMap;
						changed = 1;
					}
				}

				if (rmaLoaded)
				{

					if (indices[internal.lightShader.materialSubroutineLocation] != 
						internal.lightShader.materialSubroutine_functions[textureData.pbrTexture.RMA_loadedTextures])
					{
						indices[internal.lightShader.materialSubroutineLocation] = 
							internal.lightShader.materialSubroutine_functions[textureData.pbrTexture.RMA_loadedTextures];
						changed = 1;
					}

				}
				else
				{
					if (indices[internal.lightShader.materialSubroutineLocation] != 
						internal.lightShader.materialSubroutine_functions[0])
					{
						indices[internal.lightShader.materialSubroutineLocation] = 
							internal.lightShader.materialSubroutine_functions[0];
						changed = 1;
					}

				}


				if (albedoLoaded != 0)
				{
					if (indices[internal.lightShader.getAlbedoSubroutineLocation] != 
						internal.lightShader.albedoSubroutine_sampled)
					{
						indices[internal.lightShader.getAlbedoSubroutineLocation] = 
							internal.lightShader.albedoSubroutine_sampled;
						changed = 1;
					}
				}
				else
					if (indices[internal.lightShader.getAlbedoSubroutineLocation] != 
						internal.lightShader.albedoSubroutine_notSampled)
					{
						indices[internal.lightShader.getAlbedoSubroutineLocation] = 
							internal.lightShader.albedoSubroutine_notSampled;
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

		#pragma endregion


		glBindVertexArray(0);
		if (zPrePass)
		{
			glDepthFunc(GL_LESS);
		}


		//we draw a rect several times so we keep this vao binded
		glBindVertexArray(internal.lightShader.quadDrawer.quadVAO);
		
		#pragma region ssao

		if(internal.lightShader.useSSAO)
		{
			glViewport(0, 0, internal.adaptiveW / 2, internal.adaptiveH / 2);

			glUseProgram(internal.ssao.shader.id);

			//todo lazyness
			glBindBuffer(GL_UNIFORM_BUFFER, internal.ssao.ssaoUniformBlockBuffer);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(InternalStruct::SSAO::SsaoShaderUniformBlockData),
				&internal.ssao.ssaoShaderUniformBlockData);

			glUniformMatrix4fv(internal.ssao.u_projection, 1, GL_FALSE,
				&(camera.getProjectionMatrix())[0][0]);

			glUniformMatrix4fv(internal.ssao.u_view, 1, GL_FALSE,
				&(camera.getWorldToViewMatrix())[0][0]);

			glUniform3fv(internal.ssao.u_samples, 64, &(internal.ssao.ssaoKernel[0][0]));

			glBindFramebuffer(GL_FRAMEBUFFER, internal.ssao.ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, internal.gBuffer.buffers[internal.gBuffer.positionViewSpace]);
			glUniform1i(internal.ssao.u_gPosition, 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, internal.gBuffer.buffers[internal.gBuffer.normal]);
			glUniform1i(internal.ssao.u_gNormal, 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, internal.ssao.noiseTexture);
			glUniform1i(internal.ssao.u_texNoise, 2);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		#pragma region ssao "blur" (more like average blur)
			glViewport(0, 0, internal.adaptiveW / 4, internal.adaptiveH / 4);

			glBindFramebuffer(GL_FRAMEBUFFER, internal.ssao.blurBuffer);
			internal.ssao.blurShader.bind();
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, internal.ssao.ssaoColorBuffer);
			glUniform1i(internal.ssao.u_ssaoInput, 0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);
		#pragma endregion
		}
		#pragma endregion


		#pragma region do the lighting pass

		glBindFramebuffer(GL_FRAMEBUFFER, postProcess.fbo);
		//glClear(GL_COLOR_BUFFER_BIT); cleared before

		glUseProgram(internal.lightShader.lightingPassShader.id);

		glUniform1i(internal.lightShader.light_u_positions, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, internal.gBuffer.buffers[internal.gBuffer.position]);

		glUniform1i(internal.lightShader.light_u_normals, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, internal.gBuffer.buffers[internal.gBuffer.normal]);

		glUniform1i(internal.lightShader.light_u_albedo, 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, internal.gBuffer.buffers[internal.gBuffer.albedo]);

		glUniform1i(internal.lightShader.light_u_materials, 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, internal.gBuffer.buffers[internal.gBuffer.material]);


		glUniform1i(internal.lightShader.light_u_skyboxFiltered, 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.preFilteredMap);

		glUniform1i(internal.lightShader.light_u_skyboxIradiance, 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.convolutedTexture);

		glUniform1i(internal.lightShader.light_u_brdfTexture, 6);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, internal.lightShader.brdfTexture.id);

		glUniform1i(internal.lightShader.light_u_emmisive, 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, internal.gBuffer.buffers[internal.gBuffer.emissive]);

		glUniform1i(internal.lightShader.light_u_cascades, 8);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D_ARRAY, directionalShadows.cascadesTexture);

		glUniform1i(internal.lightShader.light_u_spotShadows, 9);
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D_ARRAY, spotShadows.shadowTextures);

		glUniform1i(internal.lightShader.light_u_pointShadows, 10);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, pointShadows.shadowTextures);


		glUniform3f(internal.lightShader.light_u_eyePosition, camera.position.x, camera.position.y, camera.position.z);

		if (internal.pointLights.size())
		{//todo laziness if lights don't change and stuff
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, internal.lightShader.pointLightsBlockBuffer);
		
			glBufferData(GL_SHADER_STORAGE_BUFFER, internal.pointLights.size() * sizeof(internal::GpuPointLight)
				, &internal.pointLights[0], GL_STREAM_DRAW);
		
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::PointLightsBlockBinding,
				internal.lightShader.pointLightsBlockBuffer);
		
		}
		glUniform1i(internal.lightShader.light_u_pointLightCount, internal.pointLights.size());

		if (internal.directionalLights.size())
		{//todo laziness if lights don't change and stuff
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, internal.lightShader.directionalLightsBlockBuffer);

			glBufferData(GL_SHADER_STORAGE_BUFFER, internal.directionalLights.size() * sizeof(internal::GpuDirectionalLight)
				, &internal.directionalLights[0], GL_STREAM_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::DirectionalLightsBlockBinding,
				internal.lightShader.directionalLightsBlockBuffer);

		}
		glUniform1i(internal.lightShader.light_u_directionalLightCount, internal.directionalLights.size());

		if (internal.spotLights.size())
		{//todo laziness if lights don't change and stuff
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, internal.lightShader.spotLightsBlockBuffer);

			glBufferData(GL_SHADER_STORAGE_BUFFER, internal.spotLights.size() * sizeof(internal::GpuSpotLight),
				internal.spotLights.data(), GL_STREAM_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::SpotLightsBlockBinding,
				internal.lightShader.spotLightsBlockBuffer);
		}
		glUniform1i(internal.lightShader.light_u_spotLightCount, internal.spotLights.size());


		//update the uniform block with data for the light shader
		internal.lightShader.lightPassUniformBlockCpuData.ambientLight = glm::vec4(skyBox.color, 0.f);

		if (skyBox.texture != 0
			&& skyBox.convolutedTexture != 0
			&& skyBox.preFilteredMap != 0
			)
		{
			internal.lightShader.lightPassUniformBlockCpuData.skyBoxPresent = true;
		}
		else
		{
			internal.lightShader.lightPassUniformBlockCpuData.skyBoxPresent = false;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, internal.lightShader.lightPassShaderData.lightPassDataBlockBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightShader::LightPassData),
			&internal.lightShader.lightPassUniformBlockCpuData);

		//blend with skybox
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisable(GL_BLEND);

	#pragma endregion


		#pragma region filter bloom data

		if (internal.lightShader.bloom)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, postProcess.filterFbo);

			//blend with emmisive data
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);

			postProcess.filterShader.shader.bind();
			glUniform1f(postProcess.filterShader.u_exposure,
				internal.lightShader.lightPassUniformBlockCpuData.exposure);
			glUniform1f(postProcess.filterShader.u_tresshold,
				internal.lightShader.lightPassUniformBlockCpuData.bloomTresshold);
			glUniform1i(postProcess.filterShader.u_texture, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[0]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glDisable(GL_BLEND);

			//fxaa on bloom data
			//antiAlias.shader.bind();
			//glUniform1i(antiAlias.u_texture, 0);
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[1]);
			//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		#pragma endregion

		#pragma region bloom blur
		bool lastBloomChannel = 0;
		if(internal.lightShader.bloom)
		{
			
			int finalMip = postProcess.currentMips;

			if (postProcess.highQualityDownSample)
			{
				bool horizontal = 0; bool firstTime = 1;
				int mipW = internal.adaptiveW;
				int mipH = internal.adaptiveH;
				lastBloomChannel = !horizontal;

				for (int i = 0; i < postProcess.currentMips + 1; i++)
				{
				#pragma region scale down
					mipW /= 2;
					mipH /= 2;
					glViewport(0, 0, mipW, mipH);

					glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[horizontal]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						postProcess.bluredColorBuffer[horizontal], i);

					postProcess.filterDown.shader.bind();
					glActiveTexture(GL_TEXTURE0);
					glUniform1i(postProcess.filterDown.u_texture, 0);
					glUniform1i(postProcess.filterDown.u_mip, firstTime ? 0 : i - 1);
					glBindTexture(GL_TEXTURE_2D,
						firstTime ? postProcess.colorBuffers[1] : postProcess.bluredColorBuffer[lastBloomChannel]);

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					lastBloomChannel = horizontal;
				#pragma endregion

				#pragma region copy data

					glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[!horizontal]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						postProcess.bluredColorBuffer[!horizontal], i);

					postProcess.addMips.shader.bind();
					glActiveTexture(GL_TEXTURE0);
					glUniform1i(postProcess.addMips.u_texture, 0);
					glUniform1i(postProcess.addMips.u_mip, i);
					glBindTexture(GL_TEXTURE_2D,
						postProcess.bluredColorBuffer[lastBloomChannel]);

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					lastBloomChannel = !horizontal;

				#pragma endregion


				#pragma region blur
					postProcess.gausianBLurShader.bind();
					glActiveTexture(GL_TEXTURE0);
					glUniform1i(postProcess.u_toBlurcolorInput, 0);
					glUniform2f(postProcess.u_texel, 1.f / mipW, 1.f / mipH);
					glUniform1i(postProcess.u_mip, i);
					//horizontal = !horizontal;

					for (int j = 0; j < 2; j++)
					{
						glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[horizontal]);
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
							postProcess.bluredColorBuffer[horizontal], i);
						glClear(GL_COLOR_BUFFER_BIT);
						glUniform1i(postProcess.u_horizontal, horizontal);

						glBindTexture(GL_TEXTURE_2D, postProcess.bluredColorBuffer[lastBloomChannel]);

						glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

						lastBloomChannel = horizontal;
						horizontal = !horizontal;
						firstTime = false;
					}
					horizontal = !horizontal;

				#pragma endregion

				}
			}
			else
			{
				bool horizontal = 0; bool firstTime = 1;
				postProcess.gausianBLurShader.bind();
				glActiveTexture(GL_TEXTURE0);
				glUniform1i(postProcess.u_toBlurcolorInput, 0);
				int mipW = internal.adaptiveW;
				int mipH = internal.adaptiveH;

				for (int i = 0; i < (postProcess.currentMips + 1) * 2; i++)
				{
					if (i % 2 == 0)
					{
						mipW /= 2;
						mipH /= 2;
						glViewport(0, 0, mipW, mipH);
						glUniform2f(postProcess.u_texel, 1.f / mipW, 1.f / mipH);
						horizontal = !horizontal;
					}

					glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[horizontal]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						postProcess.bluredColorBuffer[horizontal], i / 2);
					//glClear(GL_COLOR_BUFFER_BIT);
					glUniform1i(postProcess.u_horizontal, horizontal);
					glUniform1i(postProcess.u_mip, (i - 1) / 2);

					glBindTexture(GL_TEXTURE_2D,
						firstTime ? postProcess.colorBuffers[1] : postProcess.bluredColorBuffer[lastBloomChannel]);

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

					lastBloomChannel = horizontal;
					horizontal = !horizontal;
					firstTime = false;

				}

			}

			if (postProcess.highQualityUpSample)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
				postProcess.addMipsBlur.shader.bind();

				glActiveTexture(GL_TEXTURE0);
				glUniform1i(postProcess.addMipsBlur.u_texture, 0);
				for (; finalMip > 0; finalMip--)
				{
					int mipW = internal.adaptiveW;
					int mipH = internal.adaptiveH;

					for (int i = 0; i < finalMip; i++)
					{
						mipW /= 2;
						mipH /= 2;
					}
					glViewport(0, 0, mipW, mipH);

					glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[!lastBloomChannel]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						postProcess.bluredColorBuffer[!lastBloomChannel], finalMip - 1);

					glUniform1i(postProcess.addMipsBlur.u_mip, finalMip);
					glBindTexture(GL_TEXTURE_2D, postProcess.bluredColorBuffer[lastBloomChannel]);

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					lastBloomChannel = !lastBloomChannel;

				}

				glDisable(GL_BLEND);
				glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);

			}
			else
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
				postProcess.addMips.shader.bind();

				glActiveTexture(GL_TEXTURE0);
				glUniform1i(postProcess.addMips.u_texture, 0);
				for (; finalMip > 0; finalMip--)
				{
					int mipW = internal.adaptiveW;
					int mipH = internal.adaptiveH;

					for (int i = 0; i < finalMip; i++)
					{
						mipW /= 2;
						mipH /= 2;
					}
					glViewport(0, 0, mipW, mipH);

					glBindFramebuffer(GL_FRAMEBUFFER, postProcess.blurFbo[!lastBloomChannel]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						postProcess.bluredColorBuffer[!lastBloomChannel], finalMip - 1);

					glUniform1i(postProcess.addMips.u_mip, finalMip);
					glBindTexture(GL_TEXTURE_2D, postProcess.bluredColorBuffer[lastBloomChannel]);

					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					lastBloomChannel = !lastBloomChannel;
				}

				glDisable(GL_BLEND);

				glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);
			}

				
		}

		#pragma endregion

		#pragma region do the post process stuff and draw to the screen

		if (antiAlias.usingFXAA || adaptiveResolution.useAdaptiveResolution)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, adaptiveResolution.fbo);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		glUseProgram(postProcess.postProcessShader.id);

		//color data
		glUniform1i(postProcess.u_colorTexture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[0]);

		//bloom data
		glUniform1i(postProcess.u_bloomTexture, 1);
		glActiveTexture(GL_TEXTURE1);

		if(internal.lightShader.bloom)
		{

			if (postProcess.currentMips <= 0)//remove?
			{
				glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[1]);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, postProcess.bluredColorBuffer[lastBloomChannel]);
			}

			glUniform1f(postProcess.u_bloomIntensity, postProcess.bloomIntensty);

		}else
		{
			glUniform1f(postProcess.u_bloomIntensity, 0);

			//todo uniform block for this and also probably boolean to check if using bloom or not
			glBindTexture(GL_TEXTURE_2D, 0);

		}

		if (internal.lightShader.useSSAO)
		{
			glUniform1i(postProcess.u_useSSAO, 1);
			//todo change ssao_finalColor_exponent
			glUniform1f(postProcess.u_ssaoExponent, internal.ssao.ssao_finalColor_exponent);
			
			
			glUniform1i(postProcess.u_ssao, 3);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, internal.ssao.blurColorBuffer);
			

		}else
		{
			glUniform1i(postProcess.u_useSSAO, 0);

		}

		glUniform1i(postProcess.u_bloomNotBluredTexture, 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, postProcess.colorBuffers[1]);

		
		glUniform1f(postProcess.u_exposure, internal.lightShader.lightPassUniformBlockCpuData.exposure);

		//blend with skybox
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		//glDisable(GL_BLEND);


	#pragma endregion

	#pragma region draw to screen and fxaa

		glViewport(0, 0, internal.w, internal.h);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (antiAlias.usingFXAA || adaptiveResolution.useAdaptiveResolution)
		{

			if (antiAlias.usingFXAA)
			{
				//if adaptive rez is on mabe resample first and then fxaa
				
				antiAlias.shader.bind();
				glUniform1i(antiAlias.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, adaptiveResolution.texture);

				//send data.
				//todo lazyness
				glBindBuffer(GL_UNIFORM_BUFFER, antiAlias.fxaaDataBuffer);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(antiAlias.fxaaData), &antiAlias.fxaaData);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
			else
			{
				antiAlias.noAAshader.bind();
				glUniform1i(antiAlias.noAAu_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, adaptiveResolution.texture);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

			
		}

	#pragma endregion



	#pragma region copy depth buffer for later forward rendering
		glBindVertexArray(0);

		//glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		//glBlitFramebuffer(
		//  0, 0, adaptiveW, adaptiveH, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		//);

		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	#pragma endregion


		//reset per frame flags
		internal.perFrameFlags = {};

	}

	void Renderer3D::updateWindowMetrics(int x, int y)
	{

		internal.w = x; internal.h = y;

		if (adaptiveResolution.useAdaptiveResolution && 
			adaptiveResolution.shouldUseAdaptiveResolution)
		{
			internal.adaptiveW = internal.w * adaptiveResolution.rezRatio;
			internal.adaptiveH = internal.h * adaptiveResolution.rezRatio;
		}
		else
		{
			internal.adaptiveW = internal.w;
			internal.adaptiveH = internal.h;
		}
		

		//gbuffer
		internal.gBuffer.resize(internal.adaptiveW, internal.adaptiveH);

		//ssao
		internal.ssao.resize(internal.adaptiveW, internal.adaptiveH);
	
		//bloom buffer and color buffer
		postProcess.resize(internal.adaptiveW, internal.adaptiveH);

		adaptiveResolution.resize(internal.adaptiveW, internal.adaptiveH);

	}

	//todo remove
	void Renderer3D::renderADepthMap(GLuint texture)
	{
		glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, renderDepthMap.fbo);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


		renderDepthMap.shader.bind();
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, 1024, 1024);

		glBindVertexArray(internal.lightShader.quadDrawer.quadVAO);
		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(renderDepthMap.u_depth, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glViewport(0, 0, internal.adaptiveW, internal.adaptiveH);

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glEnable(GL_DEPTH_TEST);

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

	void Renderer3D::deleteSkyBoxTextures(SkyBox& skyBox)
	{
		skyBox.clearTextures();
	}

	SkyBox Renderer3D::atmosfericScattering(glm::vec3 sun, glm::vec3 color1, glm::vec3 color2, float g)
	{
		SkyBox skyBox = {};
		internal.skyBoxLoaderAndDrawer.atmosphericScattering(sun, color1, color2, g, skyBox);
		return skyBox;
	}

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer3D::InternalStruct::SSAO::create(int w, int h)
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0][0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
		glBindBufferBase(GL_UNIFORM_BUFFER, internal::SSAODataBlockBinding, ssaoUniformBlockBuffer);
		
		u_SSAODATA = glGetUniformBlockIndex(shader.id, "u_SSAODATA");
		glUniformBlockBinding(shader.id, u_SSAODATA, internal::SSAODataBlockBinding);
		
		//blur
		blurShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/ssao/blur.frag");
		
		glGenFramebuffers(1, &blurBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer);
		glGenTextures(1, &blurColorBuffer);
		glBindTexture(GL_TEXTURE_2D, blurColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorBuffer, 0);
		u_ssaoInput = getUniform(blurShader.id, "u_ssaoInput");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		resize(w, h);
	}

	void Renderer3D::InternalStruct::SSAO::resize(int w, int h)
	{
		if (currentDimensions.x != w || currentDimensions.y != h)
		{

			glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w / 2, h / 2, 0, GL_RED, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, blurColorBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w / 4, h / 4, 0, GL_RED, GL_FLOAT, NULL);
			
			currentDimensions = glm::ivec2(w, h);
		}
	
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
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach texture to framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
		}
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);


		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[0], 0);

		glGenFramebuffers(1, &filterFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, filterFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[1], 0);


		
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
		u_mip = getUniform(gausianBLurShader.id, "u_mip");
		u_texel = getUniform(gausianBLurShader.id, "u_texel");

		filterShader.shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/postProcess/filter.frag");
		filterShader.u_exposure = getUniform(filterShader.shader.id, "u_exposure");
		filterShader.u_texture = getUniform(filterShader.shader.id, "u_texture");
		filterShader.u_tresshold = getUniform(filterShader.shader.id, "u_tresshold");

		addMips.shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/postProcess/addMips.frag");
		addMips.u_mip = getUniform(addMips.shader.id, "u_mip");
		addMips.u_texture= getUniform(addMips.shader.id, "u_texture");

		addMipsBlur.shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/postProcess/addMipsBlur.frag");
		addMipsBlur.u_mip = getUniform(addMipsBlur.shader.id, "u_mip");
		addMipsBlur.u_texture = getUniform(addMipsBlur.shader.id, "u_texture");

		filterDown.shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/postProcess/filterDown.frag");
		filterDown.u_mip = getUniform(filterDown.shader.id, "u_mip");
		filterDown.u_texture = getUniform(filterDown.shader.id, "u_texture");

		glGenFramebuffers(2, blurFbo);
		glGenTextures(2, bluredColorBuffer);

		for(int i=0;i <2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[i]);
			float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

			glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, 1, 1, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bluredColorBuffer[i], 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
		resize(w, h);
	}

	void Renderer3D::PostProcess::resize(int w, int h)
	{
		//16 = max mips
		int mips = 0;
		{
			int mipW = w / 2;
			int mipH = h / 2;
			for (int i = 0; i < 16; i++)
			{
				if (mipW <= 4 || mipH <= 4 || 
					(mipW < 12 && mipH < 12)
					)
				{
					break;
				}

				mipW /= 2;
				mipH /= 2;
				mips = i;
			}
		}

		//mips = 1;

		if (currentDimensions.x != w || currentDimensions.y != h
			|| currentMips != mips) 
		{
			currentMips = mips;

			for (int i = 0; i < 2; i++)
			{
				glBindTexture(GL_TEXTURE_2D, bluredColorBuffer[i]);

				int mipW = w / 2;
				int mipH = h / 2;
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mips);


				for (int m = 0; m <= mips; m++)
				{
					glTexImage2D(GL_TEXTURE_2D, m, GL_R11F_G11F_B10F, mipW, mipH, 0, GL_RGB, GL_FLOAT, NULL);
					
					mipW = mipW /= 2;
					mipH = mipH /= 2;
				}
			}
		}

		if (currentDimensions.x != w || currentDimensions.y != h)
		{
			currentDimensions = glm::ivec2(w, h);

			for (int i = 0; i < 2; i++)
			{
				glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
			}
		
		}


	}

	void Renderer3D::InternalStruct::PBRtextureMaker::init()
	{
		shader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/modelLoader/mergePBRmat.frag");
		glGenFramebuffers(1, &fbo);


	}

	void Renderer3D::InternalStruct::renderSkyBox(Camera& c, SkyBox& s)
	{
		auto projMat = c.getProjectionMatrix();
		auto viewMat = c.getWorldToViewMatrix();
		viewMat = glm::mat4(glm::mat3(viewMat));

		auto viewProjMat = projMat * viewMat;

		skyBoxLoaderAndDrawer.draw(viewProjMat, s,
			lightShader.lightPassUniformBlockCpuData.exposure,
			lightShader.lightPassUniformBlockCpuData.ambientLight);
	}

	void Renderer3D::InternalStruct::renderSkyBoxBefore(Camera& c, SkyBox& s)
	{
		auto projMat = c.getProjectionMatrix();
		auto viewMat = c.getWorldToViewMatrix();
		viewMat = glm::mat4(glm::mat3(viewMat));

		auto viewProjMat = projMat * viewMat;

		skyBoxLoaderAndDrawer.drawBefore(viewProjMat, s,
			lightShader.lightPassUniformBlockCpuData.exposure,
			lightShader.lightPassUniformBlockCpuData.ambientLight);
	}

	//todo use the max w h to create it
	GLuint Renderer3D::InternalStruct::PBRtextureMaker::createRMAtexture(GpuTexture roughness, 
		GpuTexture metallic, GpuTexture ambientOcclusion, GLuint quadVAO, int &RMA_loadedTextures)
	{
		bool roughnessLoaded = (roughness.id != 0);
		bool metallicLoaded = (metallic.id != 0);
		bool ambientLoaded = (ambientOcclusion.id != 0);

		RMA_loadedTextures = 0;
		if (roughnessLoaded && metallicLoaded && ambientLoaded) { RMA_loadedTextures = 7; }
		else
		if (metallicLoaded && ambientLoaded) { RMA_loadedTextures = 6; }
		else
		if (roughnessLoaded && ambientLoaded) { RMA_loadedTextures = 5; }
		else
		if (roughnessLoaded && metallicLoaded) { RMA_loadedTextures = 4; }
		else
		if (ambientLoaded) { RMA_loadedTextures = 3; }
		else
		if (metallicLoaded) { RMA_loadedTextures = 2; }
		else
		if (roughnessLoaded) { RMA_loadedTextures = 1; }
		else { RMA_loadedTextures = 0; }

		if (RMA_loadedTextures == 0) { return 0; }

		//set w and h to the biggest texture size
		int w = 0, h = 0;
		GpuTexture textures[3] = { roughness, metallic, ambientOcclusion};
		for (int i = 0; i < 3; i++)
		{
			if (textures[i].id)
			{
				auto s = textures[i].getTextureSize();
				
				if (s.x > w) { w = s.x; }
				if (s.y > h) { h = s.y; }
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
		GLuint texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

		glBindTexture(GL_TEXTURE_2D, texture);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return texture;
	}

	void Renderer3D::DirectionalShadows::allocateTextures(int count)
	{
		textureCount = count;
		currentShadowSize = shadowSize;

		GLuint textures[2] = { cascadesTexture, staticGeometryTexture };
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		for (int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i]);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, shadowSize, shadowSize * CASCADES,
				textureCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		}
	}

	void Renderer3D::DirectionalShadows::create()
	{
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glGenTextures(1, &cascadesTexture);
		glGenTextures(1, &staticGeometryTexture);

		GLuint textures[2] = { cascadesTexture, staticGeometryTexture };

		for(int i=0; i<2; i++)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i]);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		}

		
		glGenFramebuffers(1, &cascadesFbo);
		glGenFramebuffers(1, &staticGeometryFbo);
		
		GLuint fbos[2] = { cascadesFbo, staticGeometryFbo };

		for (int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void Renderer3D::AdaptiveResolution::create(int w, int h)
	{

		if (useAdaptiveResolution)
		{
			currentDimensions = glm::ivec2(w*rezRatio, h*rezRatio);
		}
		else
		{
			currentDimensions = glm::ivec2(w, h);
		}

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, currentDimensions.x, currentDimensions.y
			, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	}

	void Renderer3D::AdaptiveResolution::resize(int w, int h)
	{
		if (currentDimensions.x != w || currentDimensions.y != h)
		{

			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			currentDimensions = glm::ivec2(w, h);
		}
	}


	void Renderer3D::AntiAlias::create(int w, int h)
	{

		shader.loadShaderProgramFromFile("shaders/drawQuads.vert",
			"shaders/aa/fxaa.frag");
		u_texture = getUniform(shader.id, "u_texture");

		u_FXAAData = glGetUniformBlockIndex(shader.id, "u_FXAAData");
		glGenBuffers(1, &fxaaDataBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, fxaaDataBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(fxaaData), &fxaaData, GL_DYNAMIC_DRAW);

		glUniformBlockBinding(shader.id, u_FXAAData, internal::FXAADataBlockBinding);
		glBindBufferBase(GL_UNIFORM_BUFFER, internal::FXAADataBlockBinding, fxaaDataBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		noAAshader.loadShaderProgramFromFile("shaders/drawQuads.vert",
			"shaders/aa/noaa.frag");
		noAAu_texture = getUniform(noAAshader.id, "u_texture");



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

	void Renderer3D::PointShadows::create()
	{

		glGenTextures(1, &shadowTextures);
		glGenTextures(1, &staticGeometryTextures);

		GLuint textures[2] = { shadowTextures , staticGeometryTextures };

		for (int i = 0; i < 2; i++)
		{

			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, textures[i]);

			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		}

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glGenFramebuffers(1, &staticGeometryFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, staticGeometryFbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer3D::PointShadows::allocateTextures(int count)
	{
		textureCount = count;
		currentShadowSize = shadowSize;

		GLuint textures[2] = { shadowTextures , staticGeometryTextures };

		for (int i = 0; i < 2; i++)
		{

			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, textures[i]);

			glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0,
				GL_DEPTH_COMPONENT32, shadowSize, shadowSize,
				textureCount*6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		}

	}


	void Renderer3D::SpotShadows::create()
	{
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glGenTextures(1, &shadowTextures);
		glGenTextures(1, &staticGeometryTextures);

		glGenFramebuffers(1, &fbo);
		glGenFramebuffers(1, &staticGeometryfbo);

		GLuint textures[2] = { shadowTextures , staticGeometryTextures };

		for (int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i]);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		}


		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, staticGeometryfbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer3D::SpotShadows::allocateTextures(int count)
	{
		textureCount = count;
		currentShadowSize = shadowSize;

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLuint textures[2] = { shadowTextures , staticGeometryTextures };

		for (int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i]);

			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, shadowSize, shadowSize,
				textureCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

		
		}
		

	}

	void Renderer3D::InternalStruct::GBuffer::create(int w, int h)
	{

		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		glGenTextures(bufferCount, buffers);


		//todo refactor
		//todo glGetInternalFormativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_FORMAT, 1, &preferred_format).
		//https://www.khronos.org/opengl/wiki/Common_Mistakes#Extensions_and_OpenGL_Versions

		glBindTexture(GL_TEXTURE_2D, buffers[position]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffers[position], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[normal]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffers[normal], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[albedo]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffers[albedo], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[material]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, buffers[material], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[positionViewSpace]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1, 1, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, buffers[positionViewSpace], 0);

		glBindTexture(GL_TEXTURE_2D, buffers[emissive]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, buffers[emissive], 0);


		unsigned int attachments[bufferCount] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
		glDrawBuffers(bufferCount, attachments);

		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1, 1);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Gbuffer failed\n";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		resize(w, h);
	}

	void Renderer3D::InternalStruct::GBuffer::resize(int w, int h)
	{
		if (currentDimensions.x != w || currentDimensions.y != h)
		{
			glBindTexture(GL_TEXTURE_2D, buffers[position]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[normal]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[albedo]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[material]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[positionViewSpace]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, buffers[emissive]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

			glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
			
			currentDimensions = glm::ivec2(w, h);
		}
	}

};