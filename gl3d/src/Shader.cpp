#include "Shader.h"
#include <fstream>
#include <iostream>
#include <unordered_map>

namespace gl3d
{
	
	GLint createShaderFromData(const char* data, GLenum shaderType)
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

				std::cout << data << ":\n" << message << "\n";

				delete[] message;

			}
			else
			{
				std::cout << data << ":\n" << "unknown error" << "\n";
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
	
	GLint createShaderFromFile(const char* source, GLenum shaderType)
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

		auto rez = createShaderFromData(headerOnlyShaders[newFileName], shaderType);
		return rez;
	
	}

#else

	GLint createShaderFromFile(const char* source, GLenum shaderType)
	{
		std::ifstream file;
		file.open(source);

		if (!file.is_open())
		{
			std::cout << "Error openning file: " << source << "\n";
			return 0;
		}

		GLint size = 0;
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);

		char* fileContent = new char[size+1] {};

		file.read(fileContent, size);


		file.close();

		auto rez = createShaderFromData(fileContent, shaderType);

		delete[] fileContent;

		return rez;

	}

#endif






	bool Shader::loadShaderProgramFromFile(const char *vertexShader, const char *fragmentShader)
	{

		auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
		auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);


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

			std::cout << "Link error: " << message << "\n";

			delete[] message;

			glDeleteProgram(id);
			id = 0;
			return 0;
		}

		glValidateProgram(id);

		return true;
	}

	bool Shader::loadShaderProgramFromFile(const char *vertexShader, const char *geometryShader, const char *fragmentShader)
	{

		auto vertexId = createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
		auto geometryId = createShaderFromFile(geometryShader, GL_GEOMETRY_SHADER);
		auto fragmentId = createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);

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

			std::cout << "Link error: " << message << "\n";

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

	GLint getUniformSubroutine(GLuint id, GLenum shaderType, const char *name)
	{
		GLint uniform = glGetSubroutineUniformLocation(id, shaderType, name);
		if (uniform == -1)
		{
			std::cout << "uniform subroutine error " << name << "\n";
		}
		return uniform;
	};

	GLint getUniform(GLuint id, const char *name)
	{
		GLint uniform = glGetUniformLocation(id, name);
		if (uniform == -1)
		{
			std::cout << "uniform error " << name << "\n";
		}
		return uniform;
	};

	GLuint getUniformBlock(GLuint id, const char *name)
	{
		GLuint uniform = glGetUniformBlockIndex(id, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "uniform block error " << name << "\n";
		}
		return uniform;
	};

	GLuint getUniformSubroutineIndex(GLuint id, GLenum shaderType, const char *name)
	{
		GLuint uniform = glGetSubroutineIndex(id, shaderType, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "uniform subroutine index error " << name << "\n";
		}
		return uniform;
	};

	GLuint getStorageBlockIndex(GLuint id, const char *name)
	{
		GLuint uniform = glGetProgramResourceIndex(id, GL_SHADER_STORAGE_BLOCK, name);
		if (uniform == GL_INVALID_INDEX)
		{
			std::cout << "storage block index error " << name << "\n";
		}
		return uniform;
	};


	//todo move
	void LightShader::create()
	{
	#pragma region brdf texture
		brdfTexture.loadTextureFromFile("resources/BRDFintegrationMap.png", TextureLoadQuality::leastPossible, 3);
		glBindTexture(GL_TEXTURE_2D, brdfTexture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	#pragma endregion

		prePass.shader.loadShaderProgramFromFile("shaders/deferred/geometryPass.vert", "shaders/deferred/zPrePass.frag");
		prePass.u_transform = getUniform(prePass.shader.id, "u_transform");
		prePass.u_albedoSampler = getUniform(prePass.shader.id, "u_albedoSampler");
		prePass.u_hasTexture = getUniform(prePass.shader.id, "u_hasTexture");

		pointShadowShader.shader.loadShaderProgramFromFile("shaders/shadows/pointShadow.vert",
			"shaders/shadows/pointShadow.geom", "shaders/shadows/pointShadow.frag");
		pointShadowShader.u_albedoSampler = getUniform(pointShadowShader.shader.id, "u_albedoSampler");
		pointShadowShader.u_farPlane = getUniform(pointShadowShader.shader.id, "u_farPlane");
		pointShadowShader.u_hasTexture = getUniform(pointShadowShader.shader.id, "u_hasTexture");
		pointShadowShader.u_lightPos = getUniform(pointShadowShader.shader.id, "u_lightPos");
		pointShadowShader.u_shadowMatrices = getUniform(pointShadowShader.shader.id, "u_shadowMatrices");
		pointShadowShader.u_transform = getUniform(pointShadowShader.shader.id, "u_transform");
		pointShadowShader.u_lightIndex = getUniform(pointShadowShader.shader.id, "u_lightIndex");

		geometryPassShader.loadShaderProgramFromFile("shaders/deferred/geometryPass.vert", "shaders/deferred/geometryPass.frag");
		//geometryPassShader.bind();

		u_transform = getUniform(geometryPassShader.id, "u_transform");
		u_modelTransform = getUniform(geometryPassShader.id, "u_modelTransform");
		u_motelViewTransform = getUniform(geometryPassShader.id, "u_motelViewTransform");
		//normalShaderLightposLocation = getUniform(shader.id, "u_lightPosition");
		textureSamplerLocation = getUniform(geometryPassShader.id, "u_albedoSampler");
		normalMapSamplerLocation = getUniform(geometryPassShader.id, "u_normalSampler");
		//eyePositionLocation = getUniform(shader.id, "u_eyePosition");
		//skyBoxSamplerLocation = getUniform(textureSamplerLocation.id, "u_skybox");
		//gamaLocation = getUniform(shader.id, "u_gama");
		RMASamplerLocation = getUniform(geometryPassShader.id, "u_RMASampler");
		//pointLightCountLocation = getUniform(shader.id, "u_pointLightCount");
		materialIndexLocation = getUniform(geometryPassShader.id, "u_materialIndex");
		u_emissiveTexture = getUniform(geometryPassShader.id, "u_emissiveTexture");
		//pointLightBufferLocation = getUniform(shader.id, "u_pointLights");
		

		materialBlockLocation = getStorageBlockIndex(geometryPassShader.id, "u_material");
		glShaderStorageBlockBinding(geometryPassShader.id, materialBlockLocation, 0);
		glGenBuffers(1, &materialBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);


		lightingPassShader.loadShaderProgramFromFile("shaders/drawQuads.vert", "shaders/deferred/lightingPass.frag");
		lightingPassShader.bind();

		light_u_albedo = getUniform(lightingPassShader.id, "u_albedo");
		light_u_normals = getUniform(lightingPassShader.id, "u_normals");
		light_u_skyboxFiltered = getUniform(lightingPassShader.id, "u_skyboxFiltered");
		light_u_positions = getUniform(lightingPassShader.id, "u_positions");
		light_u_materials = getUniform(lightingPassShader.id, "u_materials");
		light_u_eyePosition = getUniform(lightingPassShader.id, "u_eyePosition");
		light_u_pointLightCount = getUniform(lightingPassShader.id, "u_pointLightCount");
		light_u_directionalLightCount = getUniform(lightingPassShader.id, "u_directionalLightCount");
		light_u_spotLightCount = getUniform(lightingPassShader.id, "u_spotLightCount");
		light_u_ssao = getUniform(lightingPassShader.id, "u_ssao");
		light_u_view = getUniform(lightingPassShader.id, "u_view");
		light_u_skyboxIradiance = getUniform(lightingPassShader.id, "u_skyboxIradiance");
		light_u_brdfTexture = getUniform(lightingPassShader.id, "u_brdfTexture");
		light_u_emmisive = getUniform(lightingPassShader.id, "u_emmisive");
		light_u_cascades = getUniform(lightingPassShader.id, "u_cascades");
		light_u_spotShadows = getUniform(lightingPassShader.id, "u_spotShadows");
		light_u_pointShadows = getUniform(lightingPassShader.id, "u_pointShadows");

	#pragma region uniform buffer

		lightPassShaderData.u_lightPassData = glGetUniformBlockIndex(lightingPassShader.id, "u_lightPassData");
		glGenBuffers(1, &lightPassShaderData.lightPassDataBlockBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, lightPassShaderData.lightPassDataBlockBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(LightPassData), &lightPassUniformBlockCpuData, GL_DYNAMIC_DRAW);
		
		glUniformBlockBinding(lightingPassShader.id, lightPassShaderData.u_lightPassData, 1);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightPassShaderData.lightPassDataBlockBuffer);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	#pragma endregion

	#pragma region block buffers
		pointLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_pointLights");
		glShaderStorageBlockBinding(lightingPassShader.id, pointLightsBlockLocation, 1);
		glGenBuffers(1, &pointLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightsBlockBuffer);

		directionalLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_directionalLights");
		glShaderStorageBlockBinding(lightingPassShader.id, directionalLightsBlockLocation, 2);
		glGenBuffers(1, &directionalLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, directionalLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, directionalLightsBlockBuffer);

		spotLightsBlockLocation = getStorageBlockIndex(lightingPassShader.id, "u_spotLights");
		glShaderStorageBlockBinding(lightingPassShader.id, spotLightsBlockLocation, 3);
		glGenBuffers(1, &spotLightsBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotLightsBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, spotLightsBlockBuffer);

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

	}

	void LightShader::bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition, float gama,
		const MaterialValues &material, std::vector<internal::GpuPointLight> &pointLights)
	{
		geometryPassShader.bind();
		
		this->setData(viewProjMat, transformMat, lightPosition, eyePosition, gama, 
			material, pointLights);
	
	}

	void LightShader::setData(const glm::mat4 &viewProjMat, 
		const glm::mat4 &transformMat, const glm::vec3 &lightPosition, const glm::vec3 &eyePosition,
		float gama, const MaterialValues &material, std::vector<internal::GpuPointLight> &pointLights)
	{
		glUniformMatrix4fv(u_transform, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniformMatrix4fv(u_modelTransform, 1, GL_FALSE, &transformMat[0][0]);
		glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(textureSamplerLocation, 0);
		glUniform1i(normalMapSamplerLocation, 1);
		glUniform1i(skyBoxSamplerLocation, 2);
		glUniform1i(RMASamplerLocation, 3);

		if(pointLights.size())
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsBlockBuffer);

			glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(internal::GpuPointLight)
				,&pointLights[0], GL_STREAM_DRAW); 

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightsBlockBuffer);

		}

		//glUniform1fv(pointLightBufferLocation, pointLights.size() * 8, (float*)pointLights.data());

		glUniform1i(pointLightCountLocation, pointLights.size());

		setMaterial(material);
	}

	void LightShader::setMaterial(const MaterialValues &material)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(material)
			, &material, GL_STREAM_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, materialBlockBuffer);
		glUniform1i(materialIndexLocation, 0);
	}



	void LightShader::getSubroutines()
	{


		normalSubroutine_noMap = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"noNormalMapped");

		normalSubroutine_normalMap = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"normalMapped");

		
		//
		albedoSubroutine_sampled = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"sampledAlbedo");

		albedoSubroutine_notSampled = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				"notSampledAlbedo");

		//
		emissiveSubroutine_sampled = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"sampledEmmision");

		emissiveSubroutine_notSampled= getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"notSampledEmmision");


		//	
		normalSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"getNormalMapFunc");

		materialSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"u_getMaterialMapped");

		getAlbedoSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"u_getAlbedo");

		getEmmisiveSubroutineLocation = getUniformSubroutine(geometryPassShader.id, GL_FRAGMENT_SHADER,
			"u_getEmmisiveFunc");

		const char *materiaSubroutineFunctions[8] = { 
			"materialNone",
			"materialR",
			"materialM",
			"materialA",
			"materialRM",
			"materialRA",
			"materialMA",
			"materialRMA" };

		for(int i=0; i<8; i++)
		{
			materialSubroutine_functions[i] = getUniformSubroutineIndex(geometryPassShader.id, GL_FRAGMENT_SHADER,
				materiaSubroutineFunctions[i]);
		}


	}

};
