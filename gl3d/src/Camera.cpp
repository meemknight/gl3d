
#include "Camera.h"

#define GLM_GTC_matrix_transform
#include <glm/gtc/matrix_transform.hpp>

namespace gl3d
{

	glm::mat4x4 Camera::getProjectionMatrix()
	{
		auto mat = glm::perspective(this->fovRadians, this->aspectRatio, this->closePlane,
		this->farPlane);

		return mat;
	}

	glm::mat4x4 Camera::getWorldToViewMatrix()
	{
		glm::vec3 lookingAt = this->position;
		lookingAt.z -= 1.f;


		auto mat = glm::lookAt(this->position, lookingAt, this->up);
		return mat;
	}

};