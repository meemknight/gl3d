#include "Shader.h"
#include <fstream>
#include <unordered_map>

namespace gl3d
{
	
	GLint createShaderFromData(const char* data, GLenum shaderType, ErrorReporter &errorReporter)
	{
		GLuint shaderId = glCreateShader(shaderType);
		glShaderSource(shaderId, 1, &data, nullptr);
		glCompileShader(shaderId);

		GLint rezult = 0;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &rezult);

		if (!rezult)
		{
			char* message = 0;
			int   l = 0;

			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &l);

			if (l)
			{
				message = new char[l];

				glGetShaderInfoLog(shaderId, l, &l, message);

				message[l - 1] = 0;

				errorReporter.callErrorCallback(data);
				errorReporter.callErrorCallback("----");
				errorReporter.callErrorCallback(message);
				delete[] message;

			}
			else
			{
				errorReporter.callErrorCallback(data);
				errorReporter.callErrorCallback("----");
				errorReporter.callErrorCallback("unknown error :(");
			}

			glDeleteShader(shaderId);

			shaderId = 0;
			return shaderId;
		}

		return shaderId;
	}

#ifdef GL3D_LOAD_SHADERS_FROM_HEADER_ONLY

	std::unordered_map<std::string, const char*> headerOnlyShaders =
	{

		//std::pair< std::string, const char*>{"name", "value"}
		//#pragma shaderSources
	


	};
	
	GLint createShaderFromFile(const char* source, GLenum shaderType, ErrorReporter &errorReporter)
	{
		std::string newFileName;
		std::string strSource = source;
		newFileName.reserve(30);

		for(int i=strSource.size()-1; i >= 0; i--)
		{
			if (strSource[i] != '\\' && strSource[i] != '/') 
			{
				newFileName.insert(newFileName.begin(), strSource[i]);
			}
			else
			{
				break;
			}

		}

		auto rez = createShaderFromData(headerOnlyShaders[newFileName], shaderType, errorReporter);
		return rez;
	
	}

#else

	GLint createShaderFromFile(const char* source, GLenum shaderType, ErrorReporter &errorReporter)
	{
		std::ifstream file;
		file.open(source);

		if (!file.is_open())
		{
			errorReporter.callErrorCallback(std::string("Error openning file: ") + source);
			return 0;
		}

		GLint size = 0;
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);

		char* fileContent = new char[size+1] {};

		file.read(fileContent, size);


		file.close();

		auto rez = createShaderFromData(fileContent, shaderType, errorReporter);

		delete[] fileContent;

		return rez;

	}

#endif



	bool Shader::loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader, ErrorReporter &errorReporter)
	{

		auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER, errorReporter);
		auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER, errorReporter);


		if (vertexId == 0 || fragmentId == 0)
		{
			return 0;
		}

		id = glCreateProgram();

		glAttachShader(id, vertexId);
		glAttachShader(id, fragmentId);

		glLinkProgram(id);

		glDeleteShader(vertexId);
		glDeleteShader(fragmentId);

		GLint info = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &info);

		if (info != GL_TRUE)
		{
			char *message = 0;
			int   l = 0;

			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

			message = new char[l];

			glGetProgramInfoLog(id, l, &l, message);

			errorReporter.callErrorCallback(std::string("Link error: ") + message);

			delete[] message;

			glDeleteProgram(id);
			id = 0;
			return 0;
		}

		glValidateProgram(id);

		return true;
	}

	bool Shader::loadShaderProgramFromFile(const char *vertexShader, const char *geometryShader, const char *fragmentShader,
		ErrorReporter &errorReporter)
	{

		auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER, errorReporter);
		auto geometryId = createShaderFromFile(geometryShader, GL_GEOMETRY_SHADER, errorReporter);
		auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER, errorReporter);

		if (vertexId == 0 || fragmentId == 0 || geometryId == 0)
		{
			return 0;
		}

		id = glCreateProgram();

		glAttachShader(id, vertexId);
		glAttachShader(id, geometryId);
		glAttachShader(id, fragmentId);

		glLinkProgram(id);

		glDeleteShader(vertexId);
		glDeleteShader(geometryId);
		glDeleteShader(fragmentId);

		GLint info = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &info);

		if (info != GL_TRUE)
		{
			char *message = 0;
			int   l = 0;

			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

			message = new char[l];

			glGetProgramInfoLog(id, l, &l, message);

			errorReporter.callErrorCallback(std::string("Link error: ") + message);

			delete[] message;

			glDeleteProgram(id);
			id = 0;
			return 0;
		}

		glValidateProgram(id);

		return true;
	}

	void Shader::bind()
	{
		glUseProgram(id);
	}

	void Shader::clear()
	{
		glDeleteProgram(id);
		id = 0;
	}

	GLint getUniformSubroutine(GLuint id, GLenum shaderType, const char *name, ErrorReporter &errorReporter)
	{
		GLint uniform = glGetSubroutineUniformLocation(id, shaderType, name);
		if (uniform == -1)
		{
			errorReporter.callErrorCallback("uniform subroutine error " + std::string(name));
		}
		return uniform;
	};

	GLint getUniform(GLuint id, const char *name, ErrorReporter &errorReporter)
	{
		GLint uniform = glGetUniformLocation(id, name);
		if (uniform == -1)
		{
			errorReporter.callErrorCallback("uniform error " + std::string(name));
		}
		return uniform;
	};

	GLuint getUniformBlock(GLuint id, const char *name, ErrorReporter &errorReporter)
	{
		GLuint uniform = glGetUniformBlockIndex(id, name);
		if (uniform == GL_INVALID_INDEX)
		{
			errorReporter.callErrorCallback("uniform block error " + std::string(name));
		}
		return uniform;
	};

	GLuint getUniformSubroutineIndex(GLuint id, GLenum shaderType, const char *name, ErrorReporter &errorReporter)
	{
		GLuint uniform = glGetSubroutineIndex(id, shaderType, name);
		if (uniform == GL_INVALID_INDEX)
		{
			errorReporter.callErrorCallback("uniform subroutine index error " + std::string(name));
		}
		return uniform;
	};

	GLuint getStorageBlockIndex(GLuint id, const char *name, ErrorReporter &errorReporter)
	{
		GLuint uniform = glGetProgramResourceIndex(id, GL_SHADER_STORAGE_BLOCK, name);
		if (uniform == GL_INVALID_INDEX)
		{
			errorReporter.callErrorCallback("storage block index error " + std::string(name));
		}
		return uniform;
	};


	//todo move
	std::string LightShader::create(ErrorReporter &errorReporter)
	{
		std::string error = "";

	#pragma region brdf texture
		error += brdfTexture.loadTextureFromFile("resources/BRDFintegrationMap.png", TextureLoadQuality::leastPossible, 3);
		glBindTexture(GL_TEXTURE_2D, brdfTexture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	#pragma endregion

		if (!error.empty()) { error += "\n"; };

		prePass.shader.loadShaderProgramFromFile("shaders/deferred/zPrePass.vert", "shaders/deferred/zPrePass.frag", errorReporter);
		prePass.u_transform = getUniform(prePass.shader.id, "u_transform", errorReporter);
		prePass.u_albedoSampler = getUniform(prePass.shader.id, "u_albedoSampler", errorReporter);
		prePass.u_hasTexture = getUniform(prePass.shader.id, "u_hasTexture", errorReporter);
		prePass.u_hasAnimations = getUniform(prePass.shader.id, "u_hasAnimations", errorReporter);
		prePass.u_jointTransforms = getStorageBlockIndex(prePass.shader.id, "u_jointTransforms", errorReporter);
		glShaderStorageBlockBinding(prePass.shader.id, prePass.u_jointTransforms, internal::JointsTransformBlockBinding);		//todo define or enums for this


		pointShadowShader.shader.loadShaderProgramFromFile("shaders/shadows/pointShadow.vert",
			"shaders/shadows/pointShadow.geom", "shaders/shadows/pointShadow.frag", errorReporter);
		pointShadowShader.u_albedoSampler = getUniform(pointShadowShader.shader.id, "u_albedoSampler", errorReporter);
		pointShadowShader.u_farPlane = getUniform(pointShadowShader.shader.id, "u_farPlane", errorReporter);
		pointShadowShader.u_hasTexture = getUniform(pointShadowShader.shader.id, "u_hasTexture", errorReporter);
		pointShadowShader.u_lightPos = getUniform(pointShadowShader.shader.id, "u_lightPos", errorReporter);
		pointShadowShader.u_shadowMatrices = getUniform(pointShadowShader.shader.id, "u_shadowMatrices", errorReporter);
		pointShadowShader.u_transform = getUniform(pointShadowShader.shader.id, "u_transform", errorReporter);
		pointShadowShader.u_lightIndex = getUniform(pointShadowShader.shader.id, "u_lightIndex", errorReporter);
		pointShadowShader.u_hasAnimations = getUniform(pointShadowShader.shader.id, "u_hasAnimations", errorReporter);
		pointShadowShader.u_jointTransforms = getStorageBlockIndex(pointShadowShader.shader.id, "u_jointTransforms", errorReporter);
		glShaderStorageBlockBinding(pointShadowShader.shader.id, pointShadowShader.u_jointTransforms, internal::JointsTransformBlockBinding);	//todo define or enums for this



		geometryPassShader.loadShaderProgramFromFile("shaders/deferred/geometryPass.vert", "shaders/deferred/geometryPass.frag", errorReporter);
		//geometryPassShader.bind();

		u_transform = getUniform(geometryPassShader.id, "u_transform", errorReporter);
		u_hasAnimations = getUniform(geometryPassShader.id, "u_hasAnimations", errorReporter);
		u_modelTransform = getUniform(geometryPassShader.id, "u_modelTransform", errorReporter);
		u_motelViewTransform = getUniform(geometryPassShader.id, "u_motelViewTransform", errorReporter);
		//normalShaderLightposLocation = getUniform(shader.id, "u_lightPosition", errorReporter);
		normalMapSamplerLocation = getUniform(geometryPassShader.id, "u_normalSampler", errorReporter);
		//eyePositionLocation = getUniform(shader.id, "u_eyePosition", errorReporter);
		//skyBoxSamplerLocation = getUniform(textureSamplerLocation.id, "u_skybox", errorReporter);
		//gamaLocation = getUniform(shader.id, "u_gama", errorReporter);
		//pointLightCountLocation = getUniform(shader.id, "u_pointLightCount", errorReporter);
		materialIndexLocation = getUniform(geometryPassShader.id, "u_materialIndex", errorReporter);
		//pointLightBufferLocation = getUniform(shader.id, "u_pointLights", errorReporter);

		materialBlockLocation = getStorageBlockIndex(geometryPassShader.id, "u_material", errorReporter);
		glShaderStorageBlockBinding(geometryPassShader.id, materialBlockLocation, internal::MaterialBlockBinding);
		glGenBuffers(1, &materialBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);

		u_jointTransforms = getStorageBlockIndex(geometryPassShader.id, "u_jointTransforms", errorReporter);
		glShaderStorageBlockBinding(geometryPassShader.id, u_jointTransforms, internal::JointsTransformBlockBinding);		//todo define or enums for this
		

		lightingPassShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/deferred/lightingPass.frag", errorReporter);
		lightingPassShader.bind();

		light_u_normals = getUniform(lightingPassShader.id, "u_normals", errorReporter);
		light_u_skyboxFiltered = getUniform(lightingPassShader.id, "u_skyboxFiltered", errorReporter);
		light_u_positions = getUniform(lightingPassShader.id, "u_positions", errorReporter);
		light_u_eyePosition = getUniform(lightingPassShader.id, "u_eyePosition", errorReporter);
		light_u_pointLightCount = getUniform(lightingPassShader.id, "u_pointLightCount", errorReporter);
		light_u_directionalLightCount = getUniform(lightingPassShader.id, "u_directionalLightCount", errorReporter);
		light_u_spotLightCount = getUniform(lightingPassShader.id, "u_spotLightCount", errorReporter);
		light_u_skyboxIradiance = getUniform(lightingPassShader.id, "u_skyboxIradiance", errorReporter);
		light_u_brdfTexture = getUniform(lightingPassShader.id, "u_brdfTexture", errorReporter);
		light_u_cascades = getUniform(lightingPassShader.id, "u_cascades", errorReporter);
		light_u_spotShadows = getUniform(lightingPassShader.id, "u_spotShadows", errorReporter);
		light_u_pointShadows = getUniform(lightingPassShader.id, "u_pointShadows", errorReporter);
		light_u_materialIndex = getUniform(lightingPassShader.id, "u_materialIndex", errorReporter);
		light_u_textureUV = getUniform(lightingPassShader.id, "u_textureUV", errorReporter);
		light_u_textureDerivates = getUniform(lightingPassShader.id, "u_textureDerivates", errorReporter);
		light_u_transparentPass = getUniform(lightingPassShader.id, "u_transparentPass", errorReporter);

		light_materialBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_material", errorReporter);
		glShaderStorageBlockBinding(lightingPassShader.id, light_materialBlockLocation, internal::MaterialBlockBinding);

	#pragma region uniform buffer

		lightPassShaderData.u_lightPassData = glGetUniformBlockIndex(lightingPassShader.id, "u_lightPassData");
		glGenBuffers(1, &lightPassShaderData.lightPassDataBlockBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, lightPassShaderData.lightPassDataBlockBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(LightPassData), &lightPassUniformBlockCpuData, GL_DYNAMIC_DRAW);
		
		glUniformBlockBinding(lightingPassShader.id, lightPassShaderData.u_lightPassData, internal::LightPassDataBlockBinding);
		glBindBufferBase(GL_UNIFORM_BUFFER, internal::LightPassDataBlockBinding, lightPassShaderData.lightPassDataBlockBuffer);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	#pragma endregion

	#pragma region block buffers
		pointLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_pointLights", errorReporter);
		glShaderStorageBlockBinding(lightingPassShader.id, pointLightsBlockLocation, internal::PointLightsBlockBinding);
		glGenBuffers(1, &pointLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::PointLightsBlockBinding, pointLightsBlockBuffer);

		directionalLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_directionalLights", errorReporter);
		glShaderStorageBlockBinding(lightingPassShader.id, directionalLightsBlockLocation, internal::DirectionalLightsBlockBinding);
		glGenBuffers(1, &directionalLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, directionalLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::DirectionalLightsBlockBinding, directionalLightsBlockBuffer);

		spotLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_spotLights", errorReporter);
		glShaderStorageBlockBinding(lightingPassShader.id, spotLightsBlockLocation, internal::SpotLightsBlockBinding);
		glGenBuffers(1, &spotLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, internal::SpotLightsBlockBinding, spotLightsBlockBuffer);

	#pragma endregion


		glGenVertexArrays(1, &quadDrawer.quadVAO);
		glBindVertexArray(quadDrawer.quadVAO);

		glGenBuffers(1, &quadDrawer.quadBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quadDrawer.quadBuffer);

		float quadVertices[] = {
		   // positions        // texture Coords
		   -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		   -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

		glBindVertexArray(0);

		return error;
	}


	void LightShader::getSubroutines(ErrorReporter &errorReporter)
	{


		normalSubroutine_noMap = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"noNormalMapped", errorReporter);

		normalSubroutine_normalMap = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"normalMapped", errorReporter);

		//	
		normalSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"getNormalMapFunc", errorReporter);
		
	}

};
