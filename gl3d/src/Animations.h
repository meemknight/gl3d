#pragma once
#include "Core.h"
#include <string>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace gl3d
{
	//glm::mat4 animatedTransform{ 1.f }; //transform current default state to desired pos (model space)

	struct Joint
	{
		glm::mat4 inverseBindTransform{ 1.f };
		glm::mat4 localBindTransform{1.f};
		std::string name{};
		std::vector<int> children;
		//int index{};
		int root = 0;
		int used = 0;
	};

	struct KeyFrameRotation
	{
		glm::quat rotation{};
		float timeStemp{};
	};

	struct KeyFrameTranslation
	{
		glm::vec3 translation{};
		float timeStemp{};
	};

	struct KeyFrameScale
	{
		glm::vec3 scale{};
		float timeStemp{};
	};

	struct TimeStamps
	{
		float passedTimeRot = 0;
		float passedTimeTrans = 0;
		float passedTimeScale = 0;
	};

	struct Animation
	{
		std::string name;

		//for each joint we have keyframes
		std::vector<std::vector<KeyFrameRotation>> keyFramesRot;
		std::vector<std::vector<KeyFrameTranslation>> keyFramesTrans;
		std::vector<std::vector<KeyFrameScale>> keyFramesScale;
		
		std::vector<TimeStamps> timeStamps;
	};


};


