#pragma once
#include "Core.h"
#include <string>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
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
		glm::quat rotation{ 0.f,0.f,0.f,1.f };
		glm::vec3 trans{ 1.f,1.f,1.f };
		glm::vec3 scale{ 1.f,1.f,1.f };

		//int index{};
		//int root = 0;
	};

	struct KeyFrame
	{
		glm::quat rotation{0.f,0.f,0.f,1.f};
		
		glm::vec3 translation{};
		float timeStamp{};

		glm::vec3 scale{1.f, 1.f, 1.f};
		unsigned char rotationSet = 0;
		unsigned char translationSet = 0;
		unsigned char scaleSet = 0;
		unsigned char notUsed = 0;
	};

	struct KeyFrameRotation
	{
		glm::quat rotation{ 0.f,0.f,0.f,1.f };
		float timeStamp{};
	};

	struct KeyFrameTranslation
	{
		glm::vec3 translation{};
		float timeStamp{};
	};

	struct KeyFrameScale
	{
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		float timeStamp{};
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
		//std::vector<TimeStamps> timeStamps;

		float animationDuration=0;
		std::vector<int> root = {};
		//std::vector<float> timePassed;
		//std::vector<std::vector<KeyFrame>> keyFrames;
	};


};


