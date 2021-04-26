#pragma once
#include <glm\vec4.hpp>
#include <glm\vec3.hpp>
#include <GL\glew.h>
#include <iostream>

namespace gl3d
{
	inline void clearglErrors()
	{
		GLenum errorCode;
		while ((errorCode = glGetError()) != GL_NO_ERROR) {}
	}

	//https://learnopengl.com/In-Practice/Debugging
	inline GLenum checkglError(const char *file, int line, const char *command = "")
	{
		GLenum errorCode;
		while ((errorCode = glGetError()) != GL_NO_ERROR)
		{
			const char* error = "";
			switch (errorCode)
			{
				case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
				case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
				case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
				case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
				case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
				case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
				case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			}
			std::cout << error << " | " << file << " (" << line << ")" << " " << command << std::endl;
		}
		return errorCode;
	}

#define glCheck(x) clearglErrors(); x; checkglError(__FILE__, __LINE__, #x);
#define glAnyCheck() checkglError(__FILE__, __LINE__);




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

	struct Object : public InterfaceCheckId< Object >
	{
		int id_ = {};

		Object(int id = 0):id_(id) {};
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
		int RMA_loadedTextures = {};
	};

	struct GpuMaterial
	{
		glm::vec4 kd = glm::vec4(1);; //= 0.45;//w component not used
		float roughness = 0.5f;
		float metallic = 0.1;
		float ao = 1;
		float notUsed;

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

	};



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
