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

			message = new char[l];

			glGetShaderInfoLog(shaderId, l, &l, message);

			message[l - 1] = 0;

			std::cout << source << ": " << message << "\n";

			delete[] message;

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

		//todo probably free the created shader


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

	void Shader::deleteShaderProgram()
	{
		glDeleteProgram(id);
		id = 0;
	}

	void Shader::bind()
	{
		glUseProgram(id);
	}

	void LightShader::create()
	{
		shader.loadShaderProgramFromFile("shaders/normals.vert", "shaders/normals.frag");
		shader.bind();

		auto getUniform = [](GLuint id, const char *name) 
		{
			GLint uniform = glGetUniformLocation(id, name);
			if (uniform == -1)
			{
				std::cout << "uniform error "<< name << "\n";
			}
			return uniform;
		};

		normalShaderLocation = getUniform(shader.id, "u_transform");
		normalShaderNormalTransformLocation = getUniform(shader.id, "u_modelTransform");
		normalShaderLightposLocation = getUniform(shader.id, "u_lightPosition");
		textureSamplerLocation = getUniform(shader.id, "u_albedoSampler");
		normalMapSamplerLocation = getUniform(shader.id, "u_normalSampler");
		eyePositionLocation = getUniform(shader.id, "u_eyePosition");

	}

	void LightShader::bind(const glm::mat4 &viewProjMat, const glm::mat4 &transformMat,
		const glm::vec3 &lightPosition, const glm::vec3 &eyePosition
		)
	{
		shader.bind();
		glUniformMatrix4fv(normalShaderLocation, 1, GL_FALSE, &viewProjMat[0][0]);
		glUniformMatrix4fv(normalShaderNormalTransformLocation, 1, GL_FALSE, &transformMat[0][0]);
		glUniform3fv(normalShaderLightposLocation, 1, &lightPosition[0]);
		glUniform3fv(eyePositionLocation, 1, &eyePosition[0]);
		glUniform1i(textureSamplerLocation, 0);
		glUniform1i(normalMapSamplerLocation, 1);
	}

};
