#pragma once
#include "Core.h"
#include <string>
#include <glm/mat4x4.hpp>
#include <vector>

namespace gl3d
{

	struct Joint
	{
		glm::mat4 inversBindLocalTransform{};
		std::string name{};
		std::vector<int> children;
		int index{};
	};


};


