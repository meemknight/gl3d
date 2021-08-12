#pragma once
#include <glm\vec4.hpp>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <gl\glew.h>

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

#undef CREATE_RENDERER_OBJECT_HANDLE(x)



	struct TextureDataForModel
	{
		Texture albedoTexture = {};
		Texture normalMapTexture = {};

		Texture RMA_Texture = {}; //rough metalness ambient oclusion
		Texture emissiveTexture= {};
		int RMA_loadedTextures = {};
	};

	struct GpuMaterial
	{
		glm::vec4 kd = glm::vec4(1); //w component not used
		
		//rma
		float roughness = 0.5f;
		float metallic = 0.1;
		float ao = 1;
		float emmisive = 0;
		//rma

		GpuMaterial setDefaultMaterial()
		{
			*this = GpuMaterial();

			return *this;
		}
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
			glm::vec4 direction = {0,-1,0, 0};
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
			int	changedThisFrame = true; //this is sent to the gpu but not used there
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
