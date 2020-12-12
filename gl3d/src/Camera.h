#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include <cmath>

namespace gl3d
{

	constexpr float PI = 3.1415926535897932384626433;

	struct Camera
	{
		Camera() = default;
		Camera(float aspectRatio, float fovRadians)
			:aspectRatio(aspectRatio),
			fovRadians(fovRadians)
		{}

		glm::vec3 up = { 0.f,1.f,0.f };

		float aspectRatio = 1;
		float fovRadians = glm::radians(100.f);

		float closePlane = 0.1f;
		float farPlane = 100.f;


		glm::vec3 position = {};

		glm::mat4x4 getProjectionMatrix();

		glm::mat4x4 getWorldToViewMatrix();

	};

};