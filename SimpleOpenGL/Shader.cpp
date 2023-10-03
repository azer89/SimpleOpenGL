#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	// Retrieve the vertex/fragment source code from filePath
	auto vertexCode = LoadTextFile(vertexPath);
	auto fragmentCode = LoadTextFile(fragmentPath);
	auto vertex = CreateShaderProgram(vertexCode.c_str(), GL_VERTEX_SHADER);
	auto fragment = CreateShaderProgram(fragmentCode.c_str(), GL_FRAGMENT_SHADER);
	
	// Shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	CheckCompileErrors(ID, ObjectType::ProgramObject);
	
	// Delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use()
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}

void Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

unsigned int Shader::CreateShaderProgram(const char* source, GLenum shaderType)
{
	// Create shader
	unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	// Check for shader compile errors
	CheckCompileErrors(shader, ObjectType::ShaderObject);

	return shader;
}

std::string Shader::LoadTextFile(const char* filePath)
{
	std::ifstream file;
	// Ensure ifstream objects can throw exceptions:
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// Open files
		file.open(filePath);

		// Read file's buffer contents into stream
		std::stringstream sStream;
		sStream << file.rdbuf();

		// Close file handler
		file.close();

		auto str = sStream.str();

		// Convert stream into string
		return sStream.str().c_str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cerr << "Error cannot read file: " << e.what() << std::endl;
	}
}

void Shader::CheckCompileErrors(unsigned int shader, ObjectType objectType)
{
	int success;
	char infoLog[1024];
	if (objectType == ObjectType::ShaderObject)
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "Error Shader compilation of type: " << infoLog << '\n';
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "Error program linking error of type: " << infoLog << '\n';
		}
	}
}