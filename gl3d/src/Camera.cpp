#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/transform.hpp>


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
		lookingAt += viewDirection;


		auto mat = glm::lookAt(this->position, lookingAt, this->up);
		return mat;
	}

	void Camera::rotateCamera(const glm::vec2 delta)
	{

		glm::vec3 rotateYaxe = glm::cross(viewDirection, up);

		viewDirection = glm::mat3(glm::rotate(delta.x, up)) * viewDirection;

		if (delta.y < 0)
		{	//down
			if (viewDirection.y < -0.99)
				goto noMove;
		}
		else
		{	//up
			if (viewDirection.y > 0.99)
				goto noMove;
		}

		viewDirection = glm::mat3(glm::rotate(delta.y, rotateYaxe)) * viewDirection;
	noMove:

		viewDirection = glm::normalize(viewDirection);
	}


	void generateTangentSpace(glm::vec3 v, glm::vec3& outUp, glm::vec3& outRight)
	{
		glm::vec3 up(0, 1, 0);

		if (v == up)
		{
			outRight = glm::vec3(1, 0, 0);
		}
		else
		{
			outRight = normalize(glm::cross(v, up));
		}

		outUp = normalize(cross(outRight, v));

	}

	//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
	void computeFrustumDimensions(
		glm::vec3 position, glm::vec3 viewDirection,
		float fovRadians, float aspectRatio, float nearPlane, float farPlane, 
		glm::vec2& nearDimensions, glm::vec2& farDimensions, glm::vec3& centerNear, glm::vec3& centerFar)
	{
		float tanFov2 = tan(fovRadians) * 2;

		nearDimensions.y = tanFov2 * nearPlane;			//hNear
		nearDimensions.x = nearDimensions.y * aspectRatio;	//wNear

		farDimensions.y = tanFov2 * farPlane;			//hNear
		farDimensions.x = farDimensions.y * aspectRatio;	//wNear

		centerNear = position + viewDirection * farPlane;
		centerFar  = position + viewDirection * farPlane;

	}

	//https://www.youtube.com/watch?v=lUo7s-i9Gy4&ab_channel=OREONENGINE
	void computeFrustumSplitCorners(glm::vec3 directionVector, 
		glm::vec2 nearDimensions, glm::vec2 farDimensions, glm::vec3 centerNear, glm::vec3 centerFar, 
		glm::vec3& nearTopLeft, glm::vec3& nearTopRight, glm::vec3& nearBottomLeft, glm::vec3& nearBottomRight, 
		glm::vec3& farTopLeft, glm::vec3& farTopRight, glm::vec3& farBottomLeft, glm::vec3& farBottomRight)
	{

		glm::vec3 rVector = {};
		glm::vec3 upVectpr = {};

		generateTangentSpace(directionVector, upVectpr, rVector);

		nearTopLeft = centerNear + upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
		nearTopRight = centerNear + upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;
		nearBottomLeft = centerNear - upVectpr * nearDimensions.y / 2.f - rVector * nearDimensions.x / 2.f;
		nearBottomRight = centerNear - upVectpr * nearDimensions.y / 2.f + rVector * nearDimensions.x / 2.f;

		farTopLeft = centerNear + upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
		farTopRight = centerNear + upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;
		farBottomLeft = centerNear - upVectpr * farDimensions.y / 2.f - rVector * farDimensions.x / 2.f;
		farBottomRight = centerNear - upVectpr * farDimensions.y / 2.f + rVector * farDimensions.x / 2.f;

	}

	glm::vec3 fromAnglesToDirection(float zenith, float azimuth)
	{
		glm::vec4 vec(0, 1, 0, 0);

		auto zenithRotate = glm::rotate(-zenith, glm::vec3( 1.f,0.f,0.f ));
		vec = zenithRotate * vec;

		auto azimuthRotate = glm::rotate(-azimuth, glm::vec3(0.f, 1.f, 0.f));
		vec = azimuthRotate * vec;

		return glm::normalize(glm::vec3(vec));
	}

	glm::vec2 fromDirectionToAngles(glm::vec3 direction)
	{
		if (direction == glm::vec3(0, 1, 0))
		{
			return glm::vec2(0, 0);
		}
		else
		{
			glm::vec3 zenith(0, 1, 0);
			float zenithCos = glm::dot(zenith, direction);
			float zenithAngle = std::acos(zenithCos);
			
			glm::vec3 north(0, 0, -1);
			glm::vec3 projectedVector(direction.x, 0, direction.z);
			projectedVector = glm::normalize(projectedVector);

			float azmuthCos = glm::dot(north, projectedVector);
			float azmuthAngle = std::acos(azmuthCos);

			return glm::vec2(zenithAngle, azmuthAngle);
		}

	}

	

	void Camera::moveFPS(glm::vec3 direction)
	{
		viewDirection = glm::normalize(viewDirection);

		//forward
		float forward = -direction.z;
		float leftRight = direction.x;
		float upDown = direction.y;

		glm::vec3 move = {};

		move += up * upDown;
		move += glm::normalize(glm::cross(viewDirection, up)) * leftRight;
		move += viewDirection * forward;

		this->position += move;
	
	}


	
};