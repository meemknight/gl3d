#pragma once
#include <glm\vec4.hpp>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <gl\glew.h>

namespace gl3d
{
	template <class T>
	struct InterfaceCheckId
	{
		bool isNotNull()
		{
			return static_cast<T*>(this)->id_;
		}

		bool isNull()
		{
			return !this->isNotNull();
		}

	};

	struct Material : public InterfaceCheckId< Material >
	{
		int id_ = {};

		Material(int id = 0):id_(id) {};
	};

	struct Entity : public InterfaceCheckId< Entity >
	{
		int id_ = {};
	
		Entity(int id = 0):id_(id) {};
	};

	struct Model: public InterfaceCheckId< Model >
	{
		int id_ = {};

		Model(int id = 0):id_(id) {};
	};

	

	struct Texture : public InterfaceCheckId< Texture >
	{
		int id_ = {};
		
		Texture(int id = 0):id_(id) {};
	};


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
			glm::vec4 position = {};
			glm::vec4 color = { 1,1,1,0 };
		};

		struct GpuDirectionalLight
		{
			glm::vec4 direction = {0,-1,0, 0};
			glm::vec4 color = { 1,1,1,0 };
			glm::mat4 lightSpaceMatrix = glm::mat4{ 1.f };

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
