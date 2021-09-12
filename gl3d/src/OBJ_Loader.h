// OBJ_Loader.h - A Single Header OBJ Model Loader
//todo add license 

#pragma once

#include "Core.h"
#include "Animations.h"

// Iostream - STD I/O Library
#include <iostream>

// Vector - STD Vector/Array Library
#include <vector>

// String - STD String Library
#include <string>

// fStream - STD File I/O Library
#include <fstream>

// Math.h - STD math Library
#include <math.h>

// Print progress to console while loading (large models)
//#define OBJL_CONSOLE_OUTPUT

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "tiny_gltf.h"

// Namespace: OBJL
//
// Description: The namespace that holds eveyrthing that
//	is needed and used for the OBJ Model Loader
namespace objl
{
	// Structure: Vector2
	//
	// Description: A 2D Vector that Holds Positional Data
	struct Vector2
	{
		// Default Constructor
		Vector2()
		{
			X = 0.0f;
			Y = 0.0f;
		}
		// Variable Set Constructor
		Vector2(float X_, float Y_)
		{
			X = X_;
			Y = Y_;
		}
		// Bool Equals Operator Overload
		bool operator==(const Vector2& other) const
		{
			return (this->X == other.X && this->Y == other.Y);
		}
		// Bool Not Equals Operator Overload
		bool operator!=(const Vector2& other) const
		{
			return !(this->X == other.X && this->Y == other.Y);
		}
		// Addition Operator Overload
		Vector2 operator+(const Vector2& right) const
		{
			return Vector2(this->X + right.X, this->Y + right.Y);
		}
		// Subtraction Operator Overload
		Vector2 operator-(const Vector2& right) const
		{
			return Vector2(this->X - right.X, this->Y - right.Y);
		}
		// Float Multiplication Operator Overload
		Vector2 operator*(const float& other) const
		{
			return Vector2(this->X *other, this->Y * other);
		}

		// Positional Variables
		float X;
		float Y;
	};

	// Structure: Vector3
	//
	// Description: A 3D Vector that Holds Positional Data
	struct Vector3
	{
		// Default Constructor
		Vector3()
		{
			X = 0.0f;
			Y = 0.0f;
			Z = 0.0f;
		}
		// Variable Set Constructor
		Vector3(float X_, float Y_, float Z_)
		{
			X = X_;
			Y = Y_;
			Z = Z_;
		}
		// Bool Equals Operator Overload
		bool operator==(const Vector3& other) const
		{
			return (this->X == other.X && this->Y == other.Y && this->Z == other.Z);
		}
		// Bool Not Equals Operator Overload
		bool operator!=(const Vector3& other) const
		{
			return !(this->X == other.X && this->Y == other.Y && this->Z == other.Z);
		}
		// Addition Operator Overload
		Vector3 operator+(const Vector3& right) const
		{
			return Vector3(this->X + right.X, this->Y + right.Y, this->Z + right.Z);
		}
		// Subtraction Operator Overload
		Vector3 operator-(const Vector3& right) const
		{
			return Vector3(this->X - right.X, this->Y - right.Y, this->Z - right.Z);
		}
		// Float Multiplication Operator Overload
		Vector3 operator*(const float& other) const
		{
			return Vector3(this->X * other, this->Y * other, this->Z * other);
		}
		// Float Division Operator Overload
		Vector3 operator/(const float& other) const
		{
			return Vector3(this->X / other, this->Y / other, this->Z / other);
		}

		glm::vec3 operator= (glm::vec3 &other)
		{
			other.x = X;
			other.y = Y;
			other.z = Z;
			return other;
		}

		operator glm::vec3() const { return glm::vec3(X,Y,Z); }

		// Positional Variables
		float X;
		float Y;
		float Z;
	};

	

	// Structure: Vertex
	//
	// Description: Model Vertex object that holds
	//	a Position, Normal, and Texture Coordinate
	struct Vertex
	{
		// Position Vector
		Vector3 Position;

		// Normal Vector
		Vector3 Normal;

		// Texture Coordinate Vector
		Vector2 TextureCoordinate;
	};

	struct VertexAnimations
	{
		// Position Vector
		Vector3 Position;

		// Normal Vector
		Vector3 Normal;

		// Texture Coordinate Vector
		Vector2 TextureCoordinate;

		glm::ivec4 jointsId;
		glm::vec4 weights;
	};

	struct LoadedTexture
	{
		std::vector<unsigned char > data;
		int w = 0;
		int h = 0;
		int components = 0;

	};

	struct Material
	{

		// Material Name
		std::string name = "";
		// Ambient Color
		Vector3 Ka;
		// Diffuse Color
		Vector3 Kd = Vector3{ 1,1,1 };
		// Specular Color
		//Vector3 Ks;
		// Specular Exponent
		//float Ns;
		// Optical Density
		//float Ni;
		// Dissolve
		//float d;
		// Illumination
		//int illum;
		// metallic
		float metallic = 0;
		// roughness
		float roughness = 0.5;
		//ambient factor for pbr
		float ao = 1.f;
		// Ambient Texture Map
		std::string map_Ka;
		// Diffuse Texture Map
		std::string map_Kd;
		LoadedTexture loadedDiffuse;
		// Specular Texture Map
		//std::string map_Ks;
		// Specularity Map
		//std::string map_Ns;
		// Alpha Texture Map
		//std::string map_d; //todo implement
		// Bump Map
		//std::string map_bump;
		// Normal Map
		std::string map_Kn;
		LoadedTexture loadedNormal;

		//Roughness Map
		std::string map_Pr;
		//AO map
		//std::string map_Ao;
		//matallic map
		std::string map_Pm;
		//ORM map
		std::string map_ORM;
		LoadedTexture loadedORM;

		//RMA map
		std::string map_RMA;
		//Emissive map
		std::string map_emissive;
		LoadedTexture loadedEmissive;

	};

	// Structure: Mesh
	//
	// Description: A Simple Mesh Object that holds
	//	a name, a vertex list, and an index list
	struct Mesh
	{
		// Default Constructor
		Mesh()
		{

		}
		// Variable Set Constructor
		Mesh(std::vector<Vertex>& _Vertices, std::vector<unsigned int>& _Indices)
		{
			Vertices = _Vertices;
			Indices = _Indices;
		}
		// Mesh Name
		std::string MeshName;
		// Vertex List
		std::vector<Vertex> Vertices;
		std::vector<VertexAnimations> VerticesAnimations;

		// Index List
		std::vector<unsigned int> Indices;

		// Material
		//Material MeshMaterial;
		int materialIndex = -1;
	};

	// Namespace: Math
	//
	// Description: The namespace that holds all of the math
	//	functions need for OBJL
	namespace math
	{
		// Vector3 Cross Product
		inline Vector3 CrossV3(const Vector3 a, const Vector3 b)
		{
			return Vector3(a.Y * b.Z - a.Z * b.Y,
				a.Z * b.X - a.X * b.Z,
				a.X * b.Y - a.Y * b.X);
		}

		// Vector3 Magnitude Calculation
		inline float MagnitudeV3(const Vector3 in)
		{
			return (sqrtf(powf(in.X, 2) + powf(in.Y, 2) + powf(in.Z, 2)));
		}

		// Vector3 DotProduct
		inline float DotV3(const Vector3 a, const Vector3 b)
		{
			return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z);
		}

		// Angle between 2 Vector3 Objects
		inline float AngleBetweenV3(const Vector3 a, const Vector3 b)
		{
			float angle = DotV3(a, b);
			angle /= (MagnitudeV3(a) * MagnitudeV3(b));
			return angle = acosf(angle);
		}

		// Projection Calculation of a onto b
		inline Vector3 ProjV3(const Vector3 a, const Vector3 b)
		{
			Vector3 bn = b / MagnitudeV3(b);
			return bn * DotV3(a, bn);
		}
	}

	// Namespace: Algorithm
	//
	// Description: The namespace that holds all of the
	// Algorithms needed for OBJL
	namespace algorithm
	{
		// Vector3 Multiplication Opertor Overload
		inline Vector3 operator*(const float& left, const Vector3& right)
		{
			return Vector3(right.X * left, right.Y * left, right.Z * left);
		}

		// A test to see if P1 is on the same side as P2 of a line segment ab
		inline bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b)
		{
			Vector3 cp1 = math::CrossV3(b - a, p1 - a);
			Vector3 cp2 = math::CrossV3(b - a, p2 - a);

			if (math::DotV3(cp1, cp2) >= 0)
				return true;
			else
				return false;
		}

		// Generate a cross produect normal for a triangle
		inline Vector3 GenTriNormal(Vector3 t1, Vector3 t2, Vector3 t3)
		{
			Vector3 u = t2 - t1;
			Vector3 v = t3 - t1;

			Vector3 normal = math::CrossV3(u,v);

			return normal;
		}

		// Check to see if a Vector3 Point is within a 3 Vector3 Triangle
		inline bool inTriangle(Vector3 point, Vector3 tri1, Vector3 tri2, Vector3 tri3)
		{
			// Test to see if it is within an infinite prism that the triangle outlines.
			bool within_tri_prisim = SameSide(point, tri1, tri2, tri3) && SameSide(point, tri2, tri1, tri3)
				&& SameSide(point, tri3, tri1, tri2);

			// If it isn't it will never be on the triangle
			if (!within_tri_prisim)
				return false;

			// Calulate Triangle's Normal
			Vector3 n = GenTriNormal(tri1, tri2, tri3);

			// Project the point onto this normal
			Vector3 proj = math::ProjV3(point, n);

			// If the distance from the triangle to the point is 0
			//	it lies on the triangle
			if (math::MagnitudeV3(proj) == 0)
				return true;
			else
				return false;
		}

		// Split a String into a string array at a given token
		inline void split(const std::string &in,
			std::vector<std::string> &out,
			std::string token)
		{
			out.clear();
			out.reserve(12);

			std::string temp;

			for (int i = 0; i < int(in.size()); i++)
			{
				std::string test = in.substr(i, token.size());

				if (test == token)
				{
					if (!temp.empty())
					{
						out.push_back(temp);
						temp.clear();
						i += (int)token.size() - 1;
					}
					else
					{
						out.push_back("");
					}
				}
				else if (i + token.size() >= in.size())
				{
					temp += in.substr(i, token.size());
					out.push_back(temp);
					break;
				}
				else
				{
					temp += in[i];
				}
			}
		}

		inline void split2(const std::string &in,
		std::vector<std::string> &out,
		char token)
		{
			out.clear();
			out.reserve(12);

			std::string temp;
			temp.reserve(25);

			for (int i = 0; i < int(in.size()); i++)
			{

				if (in[i] == token)
				{
					if (!temp.empty())
					{
						out.push_back(temp);
						temp.clear();
					}
					else
					{
					}
				}
				else if (i + 1 >= in.size())
				{
					temp += in[i];
					out.push_back(temp);
					break;
				}
				else
				{
					temp += in[i];
				}
			}
		
		}


		// Get tail of string after first token and possibly following spaces
		inline std::string tail(const std::string &in)
		{
			size_t token_start = in.find_first_not_of(" \t");
			size_t space_start = in.find_first_of(" \t", token_start);
			size_t tail_start = in.find_first_not_of(" \t", space_start);
			size_t tail_end = in.find_last_not_of(" \t");
			if (tail_start != std::string::npos && tail_end != std::string::npos)
			{
				return in.substr(tail_start, tail_end - tail_start + 1);
			}
			else if (tail_start != std::string::npos)
			{
				return in.substr(tail_start);
			}
			return "";
		}

		// Get first token of string
		inline std::string firstToken(const std::string &in)
		{
			if (!in.empty())
			{
				size_t token_start = in.find_first_not_of(" \t");
				size_t token_end = in.find_first_of(" \t", token_start);
				if (token_start != std::string::npos && token_end != std::string::npos)
				{
					return in.substr(token_start, token_end - token_start);
				}
				else if (token_start != std::string::npos)
				{
					return in.substr(token_start);
				}
			}
			return "";
		}

		// Get element at given index position
		template <class T>
		inline const T & getElement(const std::vector<T> &elements, std::string &index)
		{
			int idx = std::stoi(index);
			if (idx < 0)
				idx = int(elements.size()) + idx;
			else
				idx--;
			return elements[idx];
		}
	}

	// Class: Loader
	//
	// Description: The OBJ Model Loader
	class Loader
	{
	public:
		// Default Constructor
		Loader() = default;
		
		// Load a file into the loader
		//
		// If file is loaded return true
		//
		// If the file is unable to be found
		// or unable to be loaded return false
		bool LoadFile(std::string Path)
		{
			// If the file is not an .obj file return false
			if (Path.substr(Path.size() - 4, 4) == ".obj")
			{
				return loadObj(Path);
			}
			else if (Path.substr(Path.size() - 5, 5) == ".gltf") 
			{
				return loadGltf(Path, 0);
			}else if (Path.substr(Path.size() - 4, 4) == ".glb")
			{
				return loadGltf(Path, 1);
			}
			else
			{
				std::cout << "3D model format not supported: " << Path << "\n"; //todo proper log
				return 0;
			}
				

		}


		bool loadGltf(const std::string &Path, bool glb = 0)
		{

			tinygltf::Model model;
			tinygltf::TinyGLTF loader;

			std::string err;
			std::string warn;

		#pragma region load and check

			bool ret;
			if (glb)
			{
				ret = loader.LoadBinaryFromFile(&model, &err, &warn, Path); // for binary glTF(.glb)
			}
			else
			{
				ret = loader.LoadASCIIFromFile(&model, &err, &warn, Path);
			}


			if (!warn.empty())
			{
				printf("Warn: %s\n", warn.c_str());
			}

			if (!err.empty())
			{
				printf("Err: %s\n", err.c_str());
			}

			if (!ret)
			{
				printf("Failed to parse glTF\n");
				return 0;
			}

		#pragma endregion

		#pragma region materials

			int count = 0;
			LoadedMaterials.resize(model.materials.size());
			for (int i = 0; i < model.materials.size(); i++)
			{
				auto &mat = model.materials[i];

				LoadedMaterials[i].name = mat.name;
				
				LoadedMaterials[i].Kd.X = mat.pbrMetallicRoughness.baseColorFactor[0]; 
				LoadedMaterials[i].Kd.Y = mat.pbrMetallicRoughness.baseColorFactor[1];
				LoadedMaterials[i].Kd.Z = mat.pbrMetallicRoughness.baseColorFactor[2];

				//todo tweak default values for gltf to be the same
				LoadedMaterials[i].metallic = mat.pbrMetallicRoughness.metallicFactor;
				LoadedMaterials[i].roughness = mat.pbrMetallicRoughness.roughnessFactor;
				

				auto MimeToExt = [](const std::string &mimeType) -> std::string
				{
					if (mimeType == "image/jpeg")
					{
						return "jpg";
					}
					else if (mimeType == "image/png")
					{
						return "png";
					}
					else if (mimeType == "image/bmp")
					{
						return "bmp";
					}
					else if (mimeType == "image/gif")
					{
						return "gif";
					}

					return "";
				};

				auto setTexture = [&](int index, LoadedTexture *t, bool checkData)->std::string
				{
					if (index != -1)
					{
						if (t) 
						{
							if (checkData) 
							{
								bool isData = false;
								t->data.resize(4 * model.images[index].width * model.images[index].height);
								for (int i = 0; i < model.images[index].width * model.images[index].height; i++)
								{
									auto r = model.images[index].image[i * 4 + 0];
									auto g = model.images[index].image[i * 4 + 1];
									auto b = model.images[index].image[i * 4 + 2];
									auto a = model.images[index].image[i * 4 + 3];

									if (r != 0 || g != 0 || b != 0) { isData = true; }

									t->data[i * 4 + 0] = r;
									t->data[i * 4 + 1] = g;
									t->data[i * 4 + 2] = b;
									t->data[i * 4 + 3] = a;

								}

								if (isData)
								{
									t->w = model.images[index].width;
									t->h = model.images[index].height;
									t->components = model.images[index].component; //todo check component
								}
								else
								{
									t->data.clear();
								}
							}
							else
							{
								t->w = model.images[index].width;
								t->h = model.images[index].height;
								t->components = model.images[index].component; //todo check component
								t->data = model.images[index].image; //
							}

						}

						//if (model.images[index].uri.empty())
						//{
						//	//std::string ret = model.images[index].name;
						//	//ret += "." + MimeToExt(model.images[index].mimeType);
						//	//return ret;
						//	return "";
						//}
						//else 
						//{
						//	//std::string ret = 
						//	//	std::string(model.images[index].uri.begin()+2, model.images[index].uri.end());
						//	//return ret;
						//	return "";
						//}
						return "";

					}
					else 
					{
						return std::string();
					}

				};


				LoadedMaterials[i].map_Kd = setTexture(mat.pbrMetallicRoughness.baseColorTexture.index, 
					&LoadedMaterials[i].loadedDiffuse, false);
				
				LoadedMaterials[i].map_Kn = setTexture(mat.normalTexture.index,
					&LoadedMaterials[i].loadedNormal, false);

				LoadedMaterials[i].map_emissive = setTexture(mat.emissiveTexture.index,
					&LoadedMaterials[i].loadedEmissive, false);

				//LoadedMaterials[i].map_Ka = setTexture(mat.occlusionTexture.index);
				//LoadedMaterials[i].map_Pr = setTexture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
				//LoadedMaterials[i].map_Pm = setTexture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
				LoadedMaterials[i].map_ORM = setTexture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index,
					&LoadedMaterials[i].loadedORM, true);


			}

		#pragma endregion

		#pragma region bones

			joints.reserve(model.nodes.size());
			int indexCount = 0;
			for (auto &b : model.nodes)
			{
				gl3d::Joint joint;

				joint.index = indexCount;
				joint.children = b.children;
				joint.name = b.name;
				
				glm::mat4 rotation(1.f);
				glm::mat4 scale(1.f);
				glm::mat4 translation(1.f);

				//suppose 4 component quaternion
				if (!b.rotation.empty())
				{
					glm::quat rot;
					rot.x = b.rotation[0];
					rot.y = b.rotation[1];
					rot.z = b.rotation[2];
					rot.w = b.rotation[3];

					rotation = glm::toMat4(rot);
				}
				
				if (!b.translation.empty())
				{
					translation = glm::translate(glm::vec3((float)b.translation[0], (float)b.translation[1], (float)b.translation[2]));
				}

				if (!b.scale.empty()) 
				{
					scale = glm::scale(glm::vec3((float)b.scale[0], (float)b.scale[1], (float)b.scale[2]));
				}


				joint.inversBindLocalTransform = translation * rotation * scale;
				joint.inversBindLocalTransform =  glm::inverse(joint.inversBindLocalTransform);

				joints.push_back(std::move(joint));
			}

		#pragma endregion


		
		#pragma region meshes

			if (!model.meshes.empty()) 
			{
				int meshesCount = 0;
				for (int j = 0; j < model.meshes.size(); j++)
				{
					meshesCount += model.meshes[j].primitives.size();
				}

				LoadedMeshes.reserve(model.meshes.size());

				for (int j = 0; j < model.meshes.size(); j++)
				{
					
					for (int i = 0; i < model.meshes[j].primitives.size(); i++)
					{
						Mesh m;
						m.MeshName = model.meshes[j].name;
						m.materialIndex = model.meshes[j].primitives[i].material;

						auto &p = model.meshes[j].primitives[i];

						tinygltf::Accessor &accessor = model.accessors[p.attributes["POSITION"]];
						tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
						tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
						float *positions = (float *)
							(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

						tinygltf::Accessor &accessorNor = model.accessors[p.attributes["NORMAL"]];
						tinygltf::BufferView &bufferViewNor = model.bufferViews[accessorNor.bufferView];
						tinygltf::Buffer &bufferNor = model.buffers[bufferViewNor.buffer];
						float *normals = (float *)
							(&bufferNor.data[bufferViewNor.byteOffset + accessorNor.byteOffset]);

						tinygltf::Accessor &accessorTex = model.accessors[p.attributes["TEXCOORD_0"]];
						tinygltf::BufferView &bufferViewTex = model.bufferViews[accessorTex.bufferView];
						tinygltf::Buffer &bufferTex = model.buffers[bufferViewTex.buffer];
						float *tex = (float *)
							(&bufferTex.data[bufferViewTex.byteOffset + accessorTex.byteOffset]);

						


						//todo look into support models without texcoords

						if(p.attributes.find("JOINTS_0") != p.attributes.end() && 
							p.attributes.find("WEIGHTS_0") != p.attributes.end()
							)
						{
							tinygltf::Accessor &accessorJoints = model.accessors[p.attributes["JOINTS_0"]];
							tinygltf::BufferView &bufferViewJoints = model.bufferViews[accessorJoints.bufferView];
							tinygltf::Buffer &bufferJoints = model.buffers[bufferViewJoints.buffer];
							

							tinygltf::Accessor &accessorWeights = model.accessors[p.attributes["WEIGHTS_0"]];
							tinygltf::BufferView &bufferViewWeights = model.bufferViews[accessorWeights.bufferView];
							tinygltf::Buffer &bufferWeights = model.buffers[bufferViewWeights.buffer];
							float *weights = (float *)
								(&bufferWeights.data[bufferViewWeights.byteOffset + accessorWeights.byteOffset]);


							for (size_t i = 0; i < accessor.count; ++i)
							{
								glm::ivec4 jointsIndex{};
								glm::vec4 weightsVec{};
								int componentCount = accessorJoints.ByteStride(bufferViewJoints);

								switch (accessorJoints.componentType)
								{
									case TINYGLTF_COMPONENT_TYPE_INT:
									{
										componentCount /= sizeof(int);
										int *joint = (int *)
											(&bufferJoints.data[bufferViewJoints.byteOffset + accessorJoints.byteOffset]);
										
										if (componentCount <= 4) 
										{
											for (int j = 0; j < componentCount; j++)
											{
												jointsIndex[j] = joint[i * componentCount + j];
												weightsVec[j] = weights[i * componentCount + j];

											}
										}

										break;
									}
									case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
									{
										componentCount /= sizeof(unsigned int);

										unsigned int *joint = (unsigned int *)
											(&bufferJoints.data[bufferViewJoints.byteOffset + accessorJoints.byteOffset]);

										if (componentCount <= 4)
										{
											for (int j = 0; j < componentCount; j++)
											{
												jointsIndex[j] = joint[i * componentCount + j];
												weightsVec[j] = weights[i * componentCount + j];

											}
										}

										break;
									}
									case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
									{
										componentCount /= sizeof(unsigned short);

										unsigned short *joint = (unsigned short *)
											(&bufferJoints.data[bufferViewJoints.byteOffset + accessorJoints.byteOffset]);

										if (componentCount <= 4)
										{
											for (int j = 0; j < componentCount; j++)
											{
												jointsIndex[j] = joint[i * componentCount + j];
												weightsVec[j] = weights[i * componentCount + j];

											}
										}

										break;
									}
									case TINYGLTF_COMPONENT_TYPE_SHORT:
									{
										componentCount /= sizeof(unsigned short);

										short *joint = (short *)
											(&bufferJoints.data[bufferViewJoints.byteOffset + accessorJoints.byteOffset]);

										if (componentCount <= 4)
										{
											for (int j = 0; j < componentCount; j++)
											{
												jointsIndex[j] = joint[i * componentCount + j];
												weightsVec[j] = weights[i * componentCount + j];

											}
										}

										break;
									}

								};

								gl3dAssertComment(componentCount <= 4, "too many bone influences per vertex. Max: 4.");
								//todo handle this case

								// Positions are Vec3 components, so for each vec3 stride, offset for x, y, and z.
								float x = positions[i * 3 + 0];// x
								float y = positions[i * 3 + 1];// y
								float z = positions[i * 3 + 2];// z

								float nx = normals[i * 3 + 0];// x
								float ny = normals[i * 3 + 1];// y
								float nz = normals[i * 3 + 2];// z

								float s = tex[i * 2 + 0];// s
								float t = tex[i * 2 + 1];// t

								VertexAnimations v;
								v.Position = Vector3(x, y, z);
								v.Normal = Vector3(nx, ny, nz);
								v.TextureCoordinate = Vector2(s, t);
								v.jointsId = jointsIndex;
								v.weights = weightsVec;

								m.VerticesAnimations.push_back(v);
							}
						}
						else
						{
							for (size_t i = 0; i < accessor.count; ++i)
							{
								// Positions are Vec3 components, so for each vec3 stride, offset for x, y, and z.
								float x = positions[i * 3 + 0];// x
								float y = positions[i * 3 + 1];// y
								float z = positions[i * 3 + 2];// z

								float nx = normals[i * 3 + 0];// x
								float ny = normals[i * 3 + 1];// y
								float nz = normals[i * 3 + 2];// z

								float s = tex[i * 2 + 0];// s
								float t = tex[i * 2 + 1];// t

								Vertex v;
								v.Position = Vector3(x, y, z);
								v.Normal = Vector3(nx, ny, nz);
								v.TextureCoordinate = Vector2(s, t);

								m.Vertices.push_back(v);
							}
						}

					
						tinygltf::Accessor &accessorIndices = model.accessors[p.indices];
						tinygltf::BufferView &bufferViewInd = model.bufferViews[accessorIndices.bufferView];
						tinygltf::Buffer &bufferInd = model.buffers[bufferViewInd.buffer];	

						for (int i = 0; i < accessorIndices.count; i++) 
						{
							switch (accessorIndices.componentType)
							{
								case TINYGLTF_COMPONENT_TYPE_INT:
								{
									int *ind = (int *)
										(&bufferInd.data[bufferViewInd.byteOffset + accessorIndices.byteOffset]);

									m.Indices.push_back(ind[i]);
									break;
								}
								case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
								{
									unsigned int *ind = (unsigned int *)
										(&bufferInd.data[bufferViewInd.byteOffset + accessorIndices.byteOffset]);

									m.Indices.push_back(ind[i]);
									break;
								}
								case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
								{
									unsigned short *ind = (unsigned short *)
										(&bufferInd.data[bufferViewInd.byteOffset + accessorIndices.byteOffset]);

									m.Indices.push_back(ind[i]);
									break;
								}
								case TINYGLTF_COMPONENT_TYPE_SHORT:
								{
									short *ind = (short *)
										(&bufferInd.data[bufferViewInd.byteOffset + accessorIndices.byteOffset]);

									m.Indices.push_back(ind[i]);
									break;
								}

							};
								
						}

						LoadedMeshes.push_back(std::move(m)); //todo add move constructor

					}

				}

			}

		#pragma endregion


			return 1;

		}

		bool loadObj(const std::string &Path)
		{

			std::ifstream file(Path);

			if (!file.is_open())
				return false;

			//todo delete materials or make sure you can't load over things
			LoadedMeshes.clear();

			std::vector<Vector3> Positions;
			std::vector<Vector2> TCoords;
			std::vector<Vector3> Normals;

			std::vector<Vertex> Vertices;
			std::vector<unsigned int> Indices;

			std::vector<std::string> MeshMatNames;

			bool listening = false;
			std::string meshname;

			Mesh tempMesh;

		#ifdef OBJL_CONSOLE_OUTPUT
			const unsigned int outputEveryNth = 1000;
			unsigned int outputIndicator = outputEveryNth;
		#endif

			std::string curline;
			while (std::getline(file, curline))
			{
			#ifdef OBJL_CONSOLE_OUTPUT
				if ((outputIndicator = ((outputIndicator + 1) % outputEveryNth)) == 1)
				{
					if (!meshname.empty())
					{
						std::cout
							<< "\r- " << meshname
							<< "\t| vertices > " << Positions.size()
							<< "\t| texcoords > " << TCoords.size()
							<< "\t| normals > " << Normals.size()
							<< "\t| triangles > " << (Vertices.size() / 3)
							<< (!MeshMatNames.empty() ? "\t| material: " + MeshMatNames.back() : "");
					}
				}
			#endif

				// Generate a Mesh Object or Prepare for an object to be created
				if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g" || curline[0] == 'g')
				{
					if (!listening)
					{
						listening = true;

						if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g")
						{
							meshname = algorithm::tail(curline);
						}
						else
						{
							meshname = "unnamed";
						}
					}
					else
					{
						// Generate the mesh to put into the array

						if (!Indices.empty() && !Vertices.empty())
						{
							// Create Mesh
							tempMesh = Mesh(Vertices, Indices);
							tempMesh.MeshName = meshname;

							// Insert Mesh
							LoadedMeshes.push_back(tempMesh);

							// Cleanup
							Vertices.clear();
							Indices.clear();
							meshname.clear();

							meshname = algorithm::tail(curline);
						}
						else
						{
							if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g")
							{
								meshname = algorithm::tail(curline);
							}
							else
							{
								meshname = "unnamed";
							}
						}
					}
				#ifdef OBJL_CONSOLE_OUTPUT
					std::cout << std::endl;
					outputIndicator = 0;
				#endif
				}
				// Generate a Vertex Position
				if (algorithm::firstToken(curline) == "v")
				{
					std::vector<std::string> spos;
					Vector3 vpos;
					algorithm::split2(algorithm::tail(curline), spos, ' ');

					vpos.X = std::stof(spos[0]);
					vpos.Y = std::stof(spos[1]);
					vpos.Z = std::stof(spos[2]);

					Positions.push_back(vpos);
				}
				// Generate a Vertex Texture Coordinate
				if (algorithm::firstToken(curline) == "vt")
				{
					std::vector<std::string> stex;
					Vector2 vtex;
					algorithm::split2(algorithm::tail(curline), stex, ' ');

					vtex.X = std::stof(stex[0]);
					vtex.Y = std::stof(stex[1]);

					TCoords.push_back(vtex);
				}
				// Generate a Vertex Normal;
				if (algorithm::firstToken(curline) == "vn")
				{
					std::vector<std::string> snor;
					Vector3 vnor;
					algorithm::split2(algorithm::tail(curline), snor, ' ');

					vnor.X = std::stof(snor[0]);
					vnor.Y = std::stof(snor[1]);
					vnor.Z = std::stof(snor[2]);

					Normals.push_back(vnor);
				}
				// Generate a Face (vertices & indices)
				if (algorithm::firstToken(curline) == "f")
				{
					// Generate the vertices
					std::vector<Vertex> vVerts;
					GenVerticesFromRawOBJ(vVerts, Positions, TCoords, Normals, curline);

					// Add Vertices
					for (int i = 0; i < int(vVerts.size()); i++)
					{
						Vertices.push_back(vVerts[i]);

					}

					std::vector<unsigned int> iIndices;

					VertexTriangluation(iIndices, vVerts);

					// Add Indices
					for (int i = 0; i < int(iIndices.size()); i++)
					{
						unsigned int indnum = (unsigned int)((Vertices.size()) - vVerts.size()) + iIndices[i];
						Indices.push_back(indnum);

						//indnum = (unsigned int)((LoadedVertices.size()) - vVerts.size()) + iIndices[i];
						//LoadedIndices.push_back(indnum);

					}
				}
				// Get Mesh Material Name
				if (algorithm::firstToken(curline) == "usemtl")
				{
					MeshMatNames.push_back(algorithm::tail(curline));

					// Create new Mesh, if Material changes within a group
					if (!Indices.empty() && !Vertices.empty())
					{
						// Create Mesh
						tempMesh = Mesh(Vertices, Indices);
						tempMesh.MeshName = meshname;
						int i = 2;
						while (1)
						{
							tempMesh.MeshName = meshname + "_" + std::to_string(i);

							for (auto &m : LoadedMeshes)
								if (m.MeshName == tempMesh.MeshName)
									continue;
							break;
						}

						// Insert Mesh
						LoadedMeshes.push_back(tempMesh);

						// Cleanup
						Vertices.clear();
						Indices.clear();
					}

				#ifdef OBJL_CONSOLE_OUTPUT
					outputIndicator = 0;
				#endif
				}
				// Load Materials
				if (algorithm::firstToken(curline) == "mtllib")
				{
					// Generate LoadedMaterial

					// Generate a path to the material file
					std::vector<std::string> temp;
					algorithm::split2(Path, temp, '/');

					std::string pathtomat = "";

					if (temp.size() != 1)
					{
						for (int i = 0; i < temp.size() - 1; i++)
						{
							pathtomat += temp[i] + "/";
						}
					}


					pathtomat += algorithm::tail(curline);

				#ifdef OBJL_CONSOLE_OUTPUT
					std::cout << std::endl << "- find materials in: " << pathtomat << std::endl;
				#endif

					// Load Materials
					LoadMaterials(pathtomat);
				}
			}

		#ifdef OBJL_CONSOLE_OUTPUT
			std::cout << std::endl;
		#endif

			// Deal with last mesh

			if (!Indices.empty() && !Vertices.empty())
			{
				// Create Mesh
				tempMesh = Mesh(Vertices, Indices);
				tempMesh.MeshName = meshname;

				// Insert Mesh
				LoadedMeshes.push_back(tempMesh);
			}

			file.close();

			// Set Materials for each Mesh
			for (int i = 0; i < MeshMatNames.size(); i++)
			{
				std::string matname = MeshMatNames[i];

				// Find corresponding material name in loaded materials
				// when found copy material variables into mesh material
				for (int j = 0; j < LoadedMaterials.size(); j++)
				{
					if (LoadedMaterials[j].name == matname)
					{
						//LoadedMeshes[i].MeshMaterial = LoadedMaterials[j];
						LoadedMeshes[i].materialIndex = j;
						break;
					}
				}
			}

			if (LoadedMeshes.empty())
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		// Loaded Mesh Objects
		std::vector<Mesh> LoadedMeshes;

		std::vector<gl3d::Joint> joints;

		// Loaded Material Objects
		std::vector<Material> LoadedMaterials;

	private:
		// Generate vertices from a list of positions, 
		//	tcoords, normals and a face line
		void GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
			const std::vector<Vector3>& iPositions,
			const std::vector<Vector2>& iTCoords,
			const std::vector<Vector3>& iNormals,
			std::string icurline)
		{
			std::vector<std::string> sface, svert;
			Vertex vVert;
			algorithm::split2(algorithm::tail(icurline), sface, ' ');

			bool noNormal = false;

			// For every given vertex do this
			for (int i = 0; i < int(sface.size()); i++)
			{
				// See What type the vertex is.
				int vtype;

				algorithm::split2(sface[i], svert, '/');

				// Check for just position - v1
				if (svert.size() == 1)
				{
					// Only position
					vtype = 1;
				}

				// Check for position & texture - v1/vt1
				if (svert.size() == 2)
				{
					// Position & Texture
					vtype = 2;
				}

				// Check for Position, Texture and Normal - v1/vt1/vn1
				// or if Position and Normal - v1//vn1
				if (svert.size() == 3)
				{
					if (svert[1] != "")
					{
						// Position, Texture, and Normal
						vtype = 4;
					}
					else
					{
						// Position & Normal
						vtype = 3;
					}
				}

				// Calculate and store the vertex
				switch (vtype)
				{
				case 1: // P
				{
					vVert.Position = algorithm::getElement(iPositions, svert[0]);
					vVert.TextureCoordinate = Vector2(0, 0);
					noNormal = true;
					oVerts.push_back(vVert);
					break;
				}
				case 2: // P/T
				{
					vVert.Position = algorithm::getElement(iPositions, svert[0]);
					vVert.TextureCoordinate = algorithm::getElement(iTCoords, svert[1]);
					noNormal = true;
					oVerts.push_back(vVert);
					break;
				}
				case 3: // P//N
				{
					vVert.Position = algorithm::getElement(iPositions, svert[0]);
					vVert.TextureCoordinate = Vector2(0, 0);
					vVert.Normal = algorithm::getElement(iNormals, svert[2]);
					oVerts.push_back(vVert);
					break;
				}
				case 4: // P/T/N
				{
					vVert.Position = algorithm::getElement(iPositions, svert[0]);
					vVert.TextureCoordinate = algorithm::getElement(iTCoords, svert[1]);
					vVert.Normal = algorithm::getElement(iNormals, svert[2]);
					oVerts.push_back(vVert);
					break;
				}
				default:
				{
					break;
				}
				}
			}

			// take care of missing normals
			// these may not be truly acurate but it is the 
			// best they get for not compiling a mesh with normals	
			if (noNormal)
			{
				Vector3 A = oVerts[0].Position - oVerts[1].Position;
				Vector3 B = oVerts[2].Position - oVerts[1].Position;

				Vector3 normal = math::CrossV3(B, A); //note vlod: changed order
				float size = normal.X * normal.X + normal.Y * normal.Y + normal.Z * normal.Z;
				size = sqrt(size);
				normal.X /= size;
				normal.Y /= size;
				normal.Z /= size;
				
				//note Vlod: normalized

				for (int i = 0; i < int(oVerts.size()); i++)
				{
					oVerts[i].Normal = normal;
				}
			}
		}

		// Triangulate a list of vertices into a face by printing
		//	inducies corresponding with triangles within it
		void VertexTriangluation(std::vector<unsigned int>& oIndices,
			const std::vector<Vertex>& iVerts)
		{
			// If there are 2 or less verts,
			// no triangle can be created,
			// so exit
			if (iVerts.size() < 3)
			{
				return;
			}
			// If it is a triangle no need to calculate it
			if (iVerts.size() == 3)
			{
				oIndices.push_back(0);
				oIndices.push_back(1);
				oIndices.push_back(2);
				return;
			}

			// Create a list of vertices
			std::vector<Vertex> tVerts = iVerts;

			while (true)
			{
				// For every vertex
				for (int i = 0; i < int(tVerts.size()); i++)
				{
					// pPrev = the previous vertex in the list
					Vertex pPrev;
					if (i == 0)
					{
						pPrev = tVerts[tVerts.size() - 1];
					}
					else
					{
						pPrev = tVerts[i - 1];
					}

					// pCur = the current vertex;
					Vertex pCur = tVerts[i];

					// pNext = the next vertex in the list
					Vertex pNext;
					if (i == tVerts.size() - 1)
					{
						pNext = tVerts[0];
					}
					else
					{
						pNext = tVerts[i + 1];
					}

					// Check to see if there are only 3 verts left
					// if so this is the last triangle
					if (tVerts.size() == 3)
					{
						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(tVerts.size()); j++)
						{
							if (iVerts[j].Position == pCur.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pPrev.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pNext.Position)
								oIndices.push_back(j);
						}

						tVerts.clear();
						break;
					}
					if (tVerts.size() == 4)
					{
						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(iVerts.size()); j++)
						{
							if (iVerts[j].Position == pCur.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pPrev.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pNext.Position)
								oIndices.push_back(j);
						}

						Vector3 tempVec;
						for (int j = 0; j < int(tVerts.size()); j++)
						{
							if (tVerts[j].Position != pCur.Position
								&& tVerts[j].Position != pPrev.Position
								&& tVerts[j].Position != pNext.Position)
							{
								tempVec = tVerts[j].Position;
								break;
							}
						}

						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(iVerts.size()); j++)
						{
							if (iVerts[j].Position == pPrev.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pNext.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == tempVec)
								oIndices.push_back(j);
						}

						tVerts.clear();
						break;
					}

					// If Vertex is not an interior vertex
					float angle = math::AngleBetweenV3(pPrev.Position - pCur.Position, pNext.Position - pCur.Position) * (180 / 3.14159265359);
					if (angle <= 0 && angle >= 180)
						continue;

					// If any vertices are within this triangle
					bool inTri = false;
					for (int j = 0; j < int(iVerts.size()); j++)
					{
						if (algorithm::inTriangle(iVerts[j].Position, pPrev.Position, pCur.Position, pNext.Position)
							&& iVerts[j].Position != pPrev.Position
							&& iVerts[j].Position != pCur.Position
							&& iVerts[j].Position != pNext.Position)
						{
							inTri = true;
							break;
						}
					}
					if (inTri)
						continue;

					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(iVerts.size()); j++)
					{
						if (iVerts[j].Position == pCur.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pPrev.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pNext.Position)
							oIndices.push_back(j);
					}

					// Delete pCur from the list
					for (int j = 0; j < int(tVerts.size()); j++)
					{
						if (tVerts[j].Position == pCur.Position)
						{
							tVerts.erase(tVerts.begin() + j);
							break;
						}
					}

					// reset i to the start
					// -1 since loop will add 1 to it
					i = -1;
				}

				// if no triangles were created
				if (oIndices.size() == 0)
					break;

				// if no more vertices
				if (tVerts.size() == 0)
					break;
			}
		}

		// Load Materials from .mtl file
		bool LoadMaterials(std::string path)
		{
			// If the file is not a material file return false
			if (path.substr(path.size() - 4, path.size()) != ".mtl")
				return false;

			std::ifstream file(path);

			// If the file is not found return false
			if (!file.is_open())
			{
				std::cout << "error loading mtl file: " << path << "\n";
				return false;
			}

			Material tempMaterial;

			bool listening = false;

			// Go through each line looking for material variables
			std::string curline;
			while (std::getline(file, curline))
			{
				auto firstToken = algorithm::firstToken(curline);

				// new material and material name
				if (firstToken == "newmtl")
				{
					if (!listening)
					{
						listening = true;

						if (curline.size() > 7)
						{
							tempMaterial.name = algorithm::tail(curline);
						}
						else
						{
							tempMaterial.name = "none";
						}
					}
					else
					{
						// Generate the material

						// Push Back loaded Material
						LoadedMaterials.push_back(tempMaterial);

						// Clear Loaded Material
						tempMaterial = Material();

						if (curline.size() > 7)
						{
							tempMaterial.name = algorithm::tail(curline);
						}
						else
						{
							tempMaterial.name = "none";
						}
					}
				}
				else
				// Ambient Color
				if (firstToken == "Ka")
				{
					std::vector<std::string> temp;
					algorithm::split2(algorithm::tail(curline), temp, ' ');

					if (temp.size() != 3)
						continue;

					tempMaterial.Ka.X = std::stof(temp[0]);
					tempMaterial.Ka.Y = std::stof(temp[1]);
					tempMaterial.Ka.Z = std::stof(temp[2]);
				}
				else
				// Diffuse Color
				if (firstToken == "Kd")
				{
					std::vector<std::string> temp;
					algorithm::split2(algorithm::tail(curline), temp, ' ');

					if (temp.size() != 3)
						continue;

					tempMaterial.Kd.X = std::stof(temp[0]);
					tempMaterial.Kd.Y = std::stof(temp[1]);
					tempMaterial.Kd.Z = std::stof(temp[2]);
				}
				else
				// Specular Color
				//if (algorithm::firstToken(curline) == "Ks")
				//{
				//	std::vector<std::string> temp;
				//	algorithm::split2(algorithm::tail(curline), temp, ' ');
				//
				//	if (temp.size() != 3)
				//		continue;
				//
				//	tempMaterial.Ks.X = std::stof(temp[0]);
				//	tempMaterial.Ks.Y = std::stof(temp[1]);
				//	tempMaterial.Ks.Z = std::stof(temp[2]);
				//}
				//else
				// Specular Exponent
				//if (algorithm::firstToken(curline) == "Ns")
				//{
				//	tempMaterial.Ns = std::stof(algorithm::tail(curline));
				//}
				//else
				// Optical Density
				//if (algorithm::firstToken(curline) == "Ni")
				//{
				//	tempMaterial.Ni = std::stof(algorithm::tail(curline));
				//}
				//else
				//// Dissolve
				//if (algorithm::firstToken(curline) == "d")
				//{
				//	tempMaterial.d = std::stof(algorithm::tail(curline));
				//}
				//else
				//// Illumination
				//if (algorithm::firstToken(curline) == "illum")
				//{
				//	tempMaterial.illum = std::stoi(algorithm::tail(curline));
				//}
				//else
				if (firstToken == "Pm")
				{
					tempMaterial.metallic = std::stoi(algorithm::tail(curline));
				}
				else
				if (firstToken == "Pr")
				{
					tempMaterial.roughness = std::stoi(algorithm::tail(curline));
				}
				else
				if (firstToken == "Ao")
				{
					tempMaterial.ao = std::stoi(algorithm::tail(curline));
				}
				else
				// Ambient Texture Map
				if (firstToken == "map_Ka" ||
					firstToken == "map_Ao")
				{
					tempMaterial.map_Ka = algorithm::tail(curline);
				}
				else
				// Diffuse Texture Map
				if (firstToken == "map_Kd")
				{
					tempMaterial.map_Kd = algorithm::tail(curline);
				}
				else
				// Specular Texture Map
				//if (firstToken == "map_Ks")
				//{
				//	tempMaterial.map_Ks = algorithm::tail(curline);
				//}
				//else
				//// Specular Hightlight Map
				//if (firstToken == "map_Ns")
				//{
				//	tempMaterial.map_Ns = algorithm::tail(curline);
				//}
				//else
				// Alpha Texture Map
				//if (firstToken == "map_d")
				//{
				//	tempMaterial.map_d = algorithm::tail(curline);
				//}
				//else
				// Bump Map
				//if (algorithm::firstToken(curline) == "map_Bump" || algorithm::firstToken(curline) == "map_bump" || algorithm::firstToken(curline) == "bump")
				//{
				//	tempMaterial.map_bump = algorithm::tail(curline);
				//}
				//else
				// Normal Map
				if (	firstToken == "map_Kn"
					||	firstToken == "norm"
					||	firstToken == "Norm"
					)
				{
					tempMaterial.map_Kn = algorithm::tail(curline);
				}
				else
				// Roughness Map
				if (firstToken == "map_Pr")
				{
					tempMaterial.map_Pr = algorithm::tail(curline);
				}
				else
				// Metallic Map
				if (firstToken == "map_Pm")
				{
					tempMaterial.map_Pm = algorithm::tail(curline);
				}
				else
				if (firstToken == "map_ORM")
				{
					tempMaterial.map_ORM = algorithm::tail(curline);
				}
				else
				if (firstToken == "map_RMA")
				{
					tempMaterial.map_ORM = algorithm::tail(curline);
				}
				else
				if (firstToken == "map_emissive" || firstToken == "map_Ke")
				{
					tempMaterial.map_emissive = algorithm::tail(curline);
				}

			}

			// Deal with last material

			// Push Back loaded Material
			LoadedMaterials.push_back(tempMaterial);

			// Test to see if anything was loaded
			// If not return false
			if (LoadedMaterials.empty())
				return false;
			// If so return true
			else
				return true;
		}

	};
}
