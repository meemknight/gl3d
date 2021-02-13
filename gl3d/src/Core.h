#pragma once
#include <glm\vec4.hpp>

namespace gl3d
{
	
	void assertFunc(const char *expression,
	const char *file_name,
	unsigned const line_number,
	const char *comment = "---");

	//todo move
	struct Material
	{
		glm::vec4 kd = glm::vec4(1);; //= 0.45;//w component not used
		float roughness = 0.2f;
		float metallic = 0.1;
		float ao = 0.5;
		Material setDefaultMaterial()
		{
			*this = Material();
			
			return *this;
		}
	};

	//todo move
	struct PointLight
	{
		glm::vec4 position = {};
		glm::vec4 color = {1,1,1,0};
	};

};

#define gl3dAssert(expression) (void)(											\
			(!!(expression)) ||													\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
		)

#define gl3dAssertComment(expression, comment) (void)(								\
			(!!(expression)) ||														\
			(gl3d::assertFunc(#expression, __FILE__, (unsigned)(__LINE__)), comment)\
		)
