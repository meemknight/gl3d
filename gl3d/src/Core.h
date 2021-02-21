#pragma once
#include <glm\vec4.hpp>
#include <glm\vec3.hpp>

namespace gl3d
{
	struct Material
	{
		int _id = {};

	};

	struct Object
	{
		int _id = {};

	};

	struct GpuMaterial
	{
		glm::vec4 kd = glm::vec4(1);; //= 0.45;//w component not used
		float roughness = 0.5f;
		float metallic = 0.1;
		float ao = 1;
		float notUdes;
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
