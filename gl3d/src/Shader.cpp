#include "Shader.h"
#include <fstream>
#include <iostream>

namespace gl3d
{

	GLint createShaderFromFile(const char *source, GLenum shaderType)
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

		char *fileContent = new char[size] {};

		file.read(fileContent, size);


		file.close();

		GLuint shaderId = glCreateShader(shaderType);
		glShaderSource(shaderId, 1, &fileContent, &size);
		glCompileShader(shaderId);

		delete[] fileContent;

		GLint rezult = 0;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &rezult);

		if (!rezult)
		{
			char *message = 0;
			int   l = 0;

			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &l);

			if(l)
			{
				message = new char[l];

				glGetShaderInfoLog(shaderId, l, &l, message);

				message[l - 1] = 0;

				std::cout << source << ": " << message << "\n";

				delete[] message;
				
			}else
			{
				std::cout << source << ": " << "unknown error"<< "\n";
			}

			glDeleteShader(shaderId);

			shaderId = 0;
			return shaderId;
		}

		return shaderId;
	}

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

	void LightShader::create()
	{
		shader.loadShaderProgramFromFile("shaders/normals.vert", "shaders/normals.frag");
		shader.bind();

		normalShaderLocation = getUniform(shader.id, "u_transform");
		normalShaderNormalTransformLocation = getUniform(shader.id, "u_modelTransform");
		normalShaderLightposLocation = getUniform(shader.id, "u_lightPosition");
		textureSamplerLocation = getUniform(shader.id, "u_albedoSampler");
		normalMapSamplerLocation = getUniform(shader.id, "u_normalSampler");
		eyePositionLocation = getUniform(shader.id, "u_eyePosition");
		skyBoxSamplerLocation = getUniform(shader.id, "u_skybox");
		gamaLocation = getUniform(shader.id, "u_gama");
		roughnessSamplerLocation = getUniform(shader.id, "u_roughnessSampler");
		ambientSamplerLocation = getUniform(shader.id, "u_aoSampler");
		metallicSamplerLocation = getUniform(shader.id, "u_metallicSampler");
		RMASamplerLocation = getUniform(shader.id, "u_RMASampler");
		
		materialBlockLocation = getUniformBlock(shader.id, "u_material");

		int size = 0;
		glGetActiveUniformBlockiv(shader.id, materialBlockLocation,
			GL_UNIFORM_BLOCK_DATA_SIZE, &size);


		glGenBuffers(1, &materialBlockBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, materialBlockBuffer);
	
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, materialBlockLocation, materialBlockBuffer);


	}

	void LightShader::bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition, float gama, const Material &material)
	{
		shader.bind();
		glUniformMatrix4fv(normalShaderLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniformMatrix4fv(normalShaderNormalTransformLocation, 1, GL_FALSE, &transformMat[0][0]);
		glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(textureSamplerLocation, 0);
		glUniform1i(normalMapSamplerLocation, 1);
		glUniform1i(skyBoxSamplerLocation, 2);
		glUniform1i(roughnessSamplerLocation, 3);
		glUniform1i(ambientSamplerLocation, 4);
		glUniform1i(metallicSamplerLocation, 5);
		glUniform1i(RMASamplerLocation, 6);
		glUniform1f(gamaLocation, gama);

		glBindBuffer(GL_UNIFORM_BUFFER, materialBlockBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(material), &material);


	}

	void LightShader::getSubroutines()
	{
		normalSubroutineLocation = getUniformSubroutine(shader.id, GL_FRAGMENT_SHADER,
			"getNormalMapFunc");

		normalSubroutine_noMap = getUniformSubroutineIndex(shader.id, GL_FRAGMENT_SHADER,
			"noNormalMapped");

		normalSubroutine_normalMap = getUniformSubroutineIndex(shader.id, GL_FRAGMENT_SHADER,
				"normalMapped");

	}

};
