#pragma once
#include <glm\mat4x4.hpp>
#include <Core.h>
#include <vector>
#include "Texture.h"
#include <string>
#include "ErrorReporting.h"

namespace gl3d
{
	
	namespace internal
	{
		enum ShaderStorageBlockBindings
		{
			MaterialBlockBinding = 0,
			PointLightsBlockBinding = 1,
			DirectionalLightsBlockBinding = 2,
			SpotLightsBlockBinding = 3,
			JointsTransformBlockBinding = 4,
		};

		enum UniformBlockBindings
		{
			LightPassDataBlockBinding = 0,
			SSAODataBlockBinding = 1,
			FXAADataBlockBinding = 2,
		};
	}

	struct Shader
	{
		GLuint id = 0;

		bool loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader, ErrorReporter &errorReporter, FileOpener &fileOpener);
		bool loadShaderProgramFromFile(const char *vertexShader, 
			const char *geometryShader, const char *fragmentShader, ErrorReporter &errorReporter, FileOpener &fileOpener);

		void bind();

		void clear();
	};

	GLint getUniform(GLuint id, const char *name, ErrorReporter &errorReporter);

	//todo this will probably dissapear
	struct LightShader
	{
		std::string create(ErrorReporter &errorReporter, FileOpener &fileOpener, const char *BRDFIntegrationMapFileLocation);

		void getSubroutines(ErrorReporter &errorReporter);

		struct
		{
			GLuint quadBuffer = 0;
			GLuint quadVAO = 0;
		}quadDrawer;

		GLint u_transform = -1;
		GLint u_hasAnimations = -1;
		GLint u_modelTransform = -1;
		GLint u_motelViewTransform = -1;
		GLint normalShaderLightposLocation = -1;
		GLint normalMapSamplerLocation = -1;
		GLint eyePositionLocation = -1;
		GLint skyBoxSamplerLocation = -1;
		GLint gamaLocation = -1;
		GLint pointLightCountLocation = -1;
		GLint pointLightBufferLocation = -1;
		GLint materialIndexLocation = -1;

		GLint light_u_normals = -1;
		GLint light_u_skyboxFiltered = -1;
		GLint light_u_eyePosition = -1;
		GLint light_u_pointLightCount = -1;
		GLint light_u_directionalLightCount = -1;
		GLint light_u_spotLightCount = -1;
		GLint light_u_skyboxIradiance = -1;
		GLint light_u_brdfTexture = -1;
		GLint light_u_cascades = -1;
		GLint light_u_spotShadows = -1;
		GLint light_u_pointShadows = -1;
		GLint light_u_materialIndex = -1;
		GLint light_u_textureUV = -1;
		GLint light_u_textureDerivates = -1;
		GLint light_u_lastTexture = -1;
		GLint light_u_positionViewSpace = -1;
		GLint light_u_transparentPass = -1;
		GLint light_u_hasLastFrameTexture = -1;
		GLint light_u_cameraProjection = -1;
		GLint light_u_view = -1;
		GLint light_u_inverseView = -1;
		
		GLint light_materialBlockLocation = GL_INVALID_INDEX;

		GLuint materialBlockLocation = GL_INVALID_INDEX;
		GLuint materialBlockBuffer = 0;

		GLuint u_jointTransforms = GL_INVALID_INDEX;

		GLuint pointLightsBlockLocation = GL_INVALID_INDEX;
		GLuint pointLightsBlockBuffer = 0;

		GLuint directionalLightsBlockLocation = GL_INVALID_INDEX;
		GLuint directionalLightsBlockBuffer = 0;

		GLuint spotLightsBlockLocation = GL_INVALID_INDEX;
		GLuint spotLightsBlockBuffer = 0;

		GLint normalSubroutineLocation = -1;

		GLuint normalSubroutine_noMap = GL_INVALID_INDEX;
		GLuint normalSubroutine_normalMap = GL_INVALID_INDEX;
		
		GLuint materialSubroutine_functions[8] = {
			GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX,
			GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX, GL_INVALID_INDEX,
		};

		//todo refactor and move things here
		struct
		{
			//the uniform block stuff
			GLuint u_lightPassData;
			GLuint lightPassDataBlockBuffer;
			//

		}lightPassShaderData;


		//to pass to the shader as an uniform block (light pass shader)
		struct LightPassData
		{
			glm::vec4 ambientLight = glm::vec4(1, 1, 1, 0); //last value is not used
			float bloomTresshold = 0.91f;
			int lightSubScater = 1;
			float exposure = 1.7;
			int skyBoxPresent = 0;

		}lightPassUniformBlockCpuData;

		struct
		{
			Shader shader;
			GLint u_transform;
			GLint u_hasTexture;
			GLint u_albedoSampler;
			GLint u_lightIndex;
			GLint u_shadowMatrices;
			GLint u_lightPos;
			GLint u_farPlane;
			GLint u_hasAnimations;
			GLuint u_jointTransforms = GL_INVALID_INDEX;
		}pointShadowShader;

		struct
		{
			Shader shader;
			GLint u_transform;
			GLint u_hasTexture;
			GLint u_albedoSampler;
			GLint u_hasAnimations;
			GLuint u_jointTransforms = GL_INVALID_INDEX;
		}prePass;

		Shader geometryPassShader;
		Shader lightingPassShader;

		bool normalMap = 1; 
		bool useSSAO = 1;
		
		//todo split stuff into separate things
		bool bloom = 1;

		GpuTexture brdfTexture;

		void clear();
	};



};