#pragma once
#include <glm\vec4.hpp>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <gl\glew.h>

#undef min
#undef max
#define GLM_ENABLE_EXPERIMENTAL

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
		glm::vec4 kd = glm::vec4(1); //w component not used //rename to albedo or color
		
		//rma
		float roughness = 0.5f;
		float metallic = 0.1;
		float ao = 1;
		float emmisive = 0;
		//rma

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

		};

		struct GpuDirectionalLight
		{
			glm::vec3 direction = {0,-1,0};
			int castShadowsIndex = 1;
			glm::vec3 color = { 1,1,1 };
			float hardness = 1;
			glm::mat4 lightSpaceMatrix[3]; //todo magic number
		
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
			int castShadows = 1;	//todo implement
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
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), comment)\
		)
