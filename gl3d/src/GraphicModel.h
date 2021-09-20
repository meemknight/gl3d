#pragma once
#include "GL/glew.h"
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "OBJ_Loader.h"

#include "Shader.h"
#include "Texture.h"
#include "Core.h"

namespace gl3d
{


	struct Transform
	{
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = { 1,1,1 };

		glm::mat4 getTransformMatrix();

		bool operator==(const Transform& other)
		{
			return 
				(position == other.position)
				&&(rotation == other.rotation)
				&&(scale == other.scale)
				;
		};

		bool operator!=(const Transform& other)
		{
			return !(*this == other);
		};
	};

	glm::mat4 getTransformMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	glm::mat4 getTransformMatrix(const Transform &t);

	struct LoadedModelData
	{
		LoadedModelData() = default;
		LoadedModelData(const char *file, float scale = 1.f) { load(file, scale); }

		void load(const char *file, float scale = 1.f);

		objl::Loader loader;
		std::string path;
	};
	

	struct DebugGraphicModel
	{
		std::string name = {};

		GLuint vertexArray = 0;

		GLuint vertexBuffer = 0;
		GLuint indexBuffer = 0;

		GLsizei primitiveCount = 0;

		void loadFromComputedData(size_t vertexSize, const float * vercies, size_t indexSize = 0, const unsigned int * indexes = nullptr, bool noTexture = false);

		void clear();

		void draw();

		//todo probably move this in the final version
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = {1,1,1};
		
		glm::mat4 getTransformMatrix();

	};


	struct GraphicModel
	{
		std::string name = "";

		GLuint vertexArray = 0;

		GLuint vertexBuffer = 0;
		GLuint indexBuffer = 0;

		GLsizei primitiveCount = 0;

		void loadFromComputedData(size_t vertexSize, const float *vertices, size_t indexSize = 0,
			const unsigned int *indexes = nullptr, bool noTexture = false, bool hasAnimationData = false,
			std::vector<Animation> animation = {},
			std::vector<Joint> joints = {}
			);
	
		void clear();

		bool hasBones = 0;

		glm::vec3 minBoundary = {};
		glm::vec3 maxBoundary = {};

		Material material = 0;
		unsigned char ownMaterial = 0;
		unsigned char culledThisFrame= 0;
		
	};

	struct Renderer3D;

	struct ModelData
	{

		std::vector < GraphicModel >models;
		std::vector < char* > subModelsNames; //for imgui
		std::vector <Material> createdMaterials;
		void clear(Renderer3D &renderer);

		std::vector<gl3d::Animation> animations;
		std::vector<gl3d::Joint> joints;

	};

	//the data for an entity
	//todo move to internal
	struct CpuEntity
	{
		Transform transform;

		std::vector < GraphicModel >models;
		std::vector < char* > subModelsNames; //for imgui
		void clear();

		int animationIndex = 0;
		float animationSpeed = 1.f;
		std::vector<Animation> animations;
		std::vector<Joint> joints;


		unsigned char flags = {}; // lsb -> 1 static, visible, shadows

		bool castShadows() {return (flags & 0b0000'0100); }
		void setCastShadows(bool s)
		{
			if (s)
			{
				flags = flags | 0b0000'0100;
			}
			else
			{
				flags = flags & ~(0b0000'0100);
			}
		}

		bool isVisible() { return (flags & 0b0000'0010); }
		void setVisible(bool v)
		{
			if (v)
			{
				flags = flags | 0b0000'0010;
			}
			else
			{
				flags = flags & ~(0b0000'0010);
			}
		}

		bool isStatic() { return (flags & 0b0000'0001); }
		void setStatic(bool s)
		{
			if (s)
			{
				flags = flags | 0b0000'0001;
			}
			else
			{
				flags = flags & ~(0b0000'0001);
			}
		}


	};

	struct LoadedTextures
	{
		std::string name;
		GpuTexture t;
	};

#pragma region skyBox

	struct SkyBox
	{
		GLuint texture = 0;				//environment cubemap
		GLuint convolutedTexture = 0;	//convoluted environment (used for difuse iradiance)
		GLuint preFilteredMap = 0;		//multiple mipmaps used for speclar 
		glm::vec3 color = { 1,1,1 };
		void clearTextures();
	};

	struct SkyBoxLoaderAndDrawer
	{
		GLuint vertexArray = 0;
		GLuint vertexBuffer = 0;
		GLuint captureFBO;

		void createGpuData();

		struct
		{
			Shader shader;
			GLuint samplerUniformLocation;
			GLuint modelViewUniformLocation;
			GLuint u_skyBoxPresent;
			GLuint u_ambient;

		}normalSkyBox;

		struct
		{
			Shader shader;
			GLuint u_equirectangularMap;
			GLuint modelViewUniformLocation;

		}hdrtoCubeMap;

		struct
		{
			Shader shader;
			GLuint u_environmentMap;
			GLuint modelViewUniformLocation;

		}convolute;

		struct
		{
			Shader shader;
			GLuint u_environmentMap;
			GLuint u_roughness;
			GLuint modelViewUniformLocation;

		}preFilterSpecular;

		struct
		{
			Shader shader;
			//GLuint u_lightPos;
			//GLuint u_g;
			//GLuint u_g2;
			GLuint modelViewUniformLocation;

		}atmosphericScatteringShader;

		enum CrossskyBoxFormats
		{
			BottomOfTheCrossRight,
			BottomOfTheCrossDown,
			BottomOfTheCrossLeft,
		};

		void loadTexture(const char *names[6], SkyBox &skyBox);
		void loadTexture(const char *name, SkyBox &skyBox, int format = 0);
		void loadHDRtexture(const char *name, SkyBox &skyBox);
		void atmosphericScattering(glm::vec3 sun, float g, float g2, SkyBox& skyBox);

		void createConvolutedAndPrefilteredTextureData(SkyBox &skyBox);

		//void clearGpuData();
		void draw(const glm::mat4& viewProjMat, SkyBox& skyBox, float exposure,
			glm::vec3 ambient);
		void drawBefore(const glm::mat4 &viewProjMat, SkyBox &skyBox, float exposure,
			glm::vec3 ambient);

	};

	/*
	
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"

	*/

#pragma endregion


};