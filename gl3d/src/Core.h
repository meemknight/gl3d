#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#define TINYGLTF_NO_INCLUDE_STB_IMAGE

#include <gl/glew.h>
#include <stb_image.h>


#define GL3D_REMOVE_IOSTREAM 0 //you can remove this if neded to. It is just used for the default errorcallback
#define GL3D_REMOVE_FSTREAM 0 //you can remove this if neded to. It is used for the default file callback, supply your own function for file oppening so the library still works :))


#if GL3D_REMOVE_IOSTREAM == 0
#include <iostream> 
#endif

#if GL3D_REMOVE_FSTREAM == 0
#include <fstream> 
#endif



#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>

#undef min
#undef max

#define GL3D_ADD_FLAG(NAME, SETNAME, VALUE)							\
		bool NAME() {return (flags & ((unsigned char)1 << VALUE) );}	\
		void SETNAME(bool s)										\
		{	if (s) { flags = flags | ((unsigned char)1 << VALUE); }	\
			else { flags = flags & ~((unsigned char)1 << VALUE); }	\
		}


namespace gl3d
{
	//todo optimization also hold the last found position

#define CREATE_RENDERER_OBJECT_HANDLE(x)	\
	struct x								\
	{										\
		int id_ = {};						\
		x (int id=0):id_(id){};				\
	}

	CREATE_RENDERER_OBJECT_HANDLE(Material);
	CREATE_RENDERER_OBJECT_HANDLE(Entity);
	CREATE_RENDERER_OBJECT_HANDLE(Model);
	CREATE_RENDERER_OBJECT_HANDLE(Texture);
	CREATE_RENDERER_OBJECT_HANDLE(SpotLight);
	CREATE_RENDERER_OBJECT_HANDLE(PointLight);
	CREATE_RENDERER_OBJECT_HANDLE(DirectionalLight);

#undef CREATE_RENDERER_OBJECT_HANDLE(x)

	struct PBRTexture
	{
		Texture texture = {};  //rough metalness ambient oclusion

		// (roughnessLoaded) -> 0b100
		// (metallicLoaded)  -> 0b010
		// (ambientLoaded)   -> 0b001
		int RMA_loadedTextures = {};
	};

	struct TextureDataForMaterial
	{
		Texture albedoTexture = {};
		Texture normalMapTexture = {};
		Texture emissiveTexture= {};
		PBRTexture pbrTexture = {};

		bool operator==(const TextureDataForMaterial& other)
		{
			return
				(albedoTexture.id_ == other.albedoTexture.id_)
				&& (normalMapTexture.id_ == other.normalMapTexture.id_)
				&& (emissiveTexture.id_ == other.emissiveTexture.id_)
				&& (pbrTexture.texture.id_ == other.pbrTexture.texture.id_)
				&& (pbrTexture.RMA_loadedTextures == other.pbrTexture.RMA_loadedTextures)
				;
		};

		bool operator!=(const TextureDataForMaterial& other)
		{
			return !(*this == other);
		};
	};
	
	//note this is the gpu material
	struct MaterialValues
	{
		glm::vec4 kd = glm::vec4(1);
		
		//rma
		float roughness = 0.5f;
		float metallic = 0.1;
		float ao = 1;
		float emmisive = 0;
		//rma

		GLuint64 albedoSampler = 0;
		GLuint64 rmaSampler = 0;
		GLuint64 emmissiveSampler = 0;
		int rmaLoaded = {};
		int notUsed;

		MaterialValues setDefaultMaterial()
		{
			*this = MaterialValues();

			return *this;
		}

		bool operator==(const MaterialValues& other)
		{
			return
				(kd == other.kd)
				&& (roughness == other.roughness)
				&& (metallic == other.metallic)
				&& (ao == other.ao)
				&& (emmisive == other.emmisive)
				;
		};

		bool operator!=(const MaterialValues& other)
		{
			return !(*this == other);
		};
	};

	//todo move
	namespace internal
	{
		

		//todo move
		struct GpuPointLight
		{
			glm::vec3 position = {};
			float dist = 20;
			glm::vec3 color = { 1,1,1 };
			float attenuation = 2;
			int castShadowsIndex = 1;
			float hardness = 1;
			int castShadows = 1;
			int changedThisFrame = 1;
		};

		struct GpuDirectionalLight
		{
			glm::vec3 direction = {0,-1,0};
			int castShadowsIndex = 1;
			
			int changedThisFrame = 1;
			int castShadows = 1;
			int notUsed1 = 0;
			int notUsed2 = 0;

			glm::vec3 color = { 1,1,1 };
			float hardness = 1;
			glm::mat4 lightSpaceMatrix[3]; //magic number (cascades)

		};

		struct GpuSpotLight
		{
			glm::vec3 position = {};
			float cosHalfAngle = std::cos(3.14159/4.f);
			glm::vec3 direction = { 0,-1,0 };
			float dist = 20;
			glm::vec3 color = { 1, 1, 1 };
			float attenuation = 2;
			float hardness = 1;
			int shadowIndex = 0;
			int castShadows = 1;
			int	changedThisFrame = 1; //this is sent to the gpu but not used there
			float nearPlane = 0.1;
			float farPlane = 10;
			float notUsed1 = 0;
			float notUsed2 = 0;
			glm::mat4 lightSpaceMatrix;
		};


	};

	void GLAPIENTRY glDebugOutput(GLenum source,
								GLenum type,
								unsigned int id,
								GLenum severity,
								GLsizei length,
								const char *message,
								const void *userParam);

	void assertFunc(const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");

};

#define gl3dAssert(expression) (void)(											\
			(!!(expression)) ||													\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
		)

#define gl3dAssertComment(expression, comment) (void)(								\
			(!!(expression)) ||														\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__), comment), 0)\
		)
