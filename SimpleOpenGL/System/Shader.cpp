#include "Shader.h"
#include "AppSettings.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexFilename,
	const char* fragmentFilename,
	const char* geometryFilename)
{
	// Shader Program
	ID = glCreateProgram();

	// Retrieve the vertex/fragment source code from filePath
	auto vsFullFilepath = AppSettings::ShaderFolder + vertexFilename;
	auto vertexCode = LoadTextFile(vsFullFilepath.c_str());
	auto vertex = CreateShaderProgram(vertexCode.c_str(), GL_VERTEX_SHADER);
	glAttachShader(ID, vertex);

	auto fsFullFilepath = AppSettings::ShaderFolder + fragmentFilename;
	auto fragmentCode = LoadTextFile(fsFullFilepath.c_str());
	auto fragment = CreateShaderProgram(fragmentCode.c_str(), GL_FRAGMENT_SHADER);
	glAttachShader(ID, fragment);

	if (geometryFilename)
	{
		auto gFullFilepath = AppSettings::ShaderFolder + geometryFilename;
		auto geomCode = LoadTextFile(gFullFilepath.c_str());
		auto geom = CreateShaderProgram(geomCode.c_str(), GL_GEOMETRY_SHADER);
		glAttachShader(ID, geom);
	}
	
	glLinkProgram(ID);
	CheckCompileErrors(ID, ObjectType::ProgramObject);
	
	// Delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use() const
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}

void Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::SetMat2(const std::string& name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
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
			std::cerr << "Error Shader compilation of type: " << infoLog << '\n';
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << "Error program linking error of type: " << infoLog << '\n';
		}
	}
}