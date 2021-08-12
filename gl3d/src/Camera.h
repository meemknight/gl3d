#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include <cmath>

namespace gl3d
{

	constexpr float PI = 3.1415926535897932384626433;

	void generateTangentSpace(glm::vec3 v, glm::vec3& outUp, glm::vec3& outRight);

	//near w,h   far w, h     center near, far
	void computeFrustumDimensions(glm::vec3 position, glm::vec3 viewDirection,
		float fovRadians, float aspectRatio, float nearPlane, float farPlane,
		glm::vec2& nearDimensions, glm::vec2& farDimensions, glm::vec3& centerNear,
		glm::vec3& centerFar);

	void computeFrustumSplitCorners(glm::vec3 directionVector, 
		glm::vec2 nearDimensions, glm::vec2 farDimensions, glm::vec3 centerNear, glm::vec3 centerFar,
		glm::vec3& nearTopLeft, glm::vec3& nearTopRight, glm::vec3& nearBottomLeft, glm::vec3& nearBottomRight,
		glm::vec3& farTopLeft, glm::vec3& farTopRight, glm::vec3& farBottomLeft, glm::vec3& farBottomRight
		);

	glm::vec3 fromAnglesToDirection(float zenith, float azimuth);
	glm::vec2 fromDirectionToAngles(glm::vec3 direction);

	struct Camera
	{
		Camera() = default;
		Camera(float aspectRatio, float fovRadians)
			:aspectRatio(aspectRatio),
			fovRadians(fovRadians)
		{}

		glm::vec3 up = { 0.f,1.f,0.f };

		float aspectRatio = 1;
		float fovRadians = glm::radians(60.f);

		float closePlane = 0.01f;
		float farPlane = 200.f;


		glm::vec3 position = {};
		glm::vec3 viewDirection = {0,0,-1};

		glm::mat4x4 getProjectionMatrix();

		glm::mat4x4 getWorldToViewMatrix();

		void rotateCamera(const glm::vec2 delta);


		void moveFPS(glm::vec3 direction);


	};

};