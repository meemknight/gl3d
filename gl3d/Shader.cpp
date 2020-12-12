#include "Shader.h"
#include <fstream>
#include <iostream>


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

		std::cout  << source << ": "<< message <<"\n" ;

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


	if(vertexId == 0 || fragmentId == 0)
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

		std::cout << "Link error: " <<  message << "\n";

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
