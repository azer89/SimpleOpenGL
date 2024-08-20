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
	const auto vsFullFilepath = AppSettings::ShaderFolder + vertexFilename;
	const auto vertexCode = LoadTextFile(vsFullFilepath.c_str());
	const auto vertex = CreateShaderProgram(vertexCode.c_str(), GL_VERTEX_SHADER);
	glAttachShader(ID, vertex);

	const auto fsFullFilepath = AppSettings::ShaderFolder + fragmentFilename;
	const auto fragmentCode = LoadTextFile(fsFullFilepath.c_str());
	const auto fragment = CreateShaderProgram(fragmentCode.c_str(), GL_FRAGMENT_SHADER);
	glAttachShader(ID, fragment);

	if (geometryFilename)
	{
		const auto gFullFilepath = AppSettings::ShaderFolder + geometryFilename;
		const auto geomCode = LoadTextFile(gFullFilepath.c_str());
		const auto geom = CreateShaderProgram(geomCode.c_str(), GL_GEOMETRY_SHADER);
		glAttachShader(ID, geom);
	}

	glLinkProgram(ID);
	CheckCompileErrors(ID, ObjectType::Program);

	// Delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use() const
{
	glUseProgram(ID);
}

void Shader::Delete() const
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

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) const
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
	const unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// Check for shader compile errors
	CheckCompileErrors(shader, ObjectType::Shader);

	return shader;
}

std::string Shader::LoadTextFile(const char* filePath)
{
	const char whitespace = ' ';
	const std::string includeIdentifier = "#include ";

	std::string shaderCode{};
	std::ifstream file(filePath);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to load shader file " + std::string(filePath));
	}

	std::string lineBuffer;
	while (std::getline(file, lineBuffer))
	{
		if (lineBuffer.find(includeIdentifier) != lineBuffer.npos)
		{
			const std::size_t index = lineBuffer.find_last_of(whitespace);
			std::string includeFullPath = AppSettings::ShaderFolder + lineBuffer.substr(index + 1);
			shaderCode += LoadTextFile(includeFullPath.c_str());
		}
		else
		{
			shaderCode += lineBuffer + '\n';
		}
	}

	file.close();

	return shaderCode;
}

void Shader::CheckCompileErrors(unsigned int shader, ObjectType objectType)
{
	int success;
	char infoLog[1024];
	if (objectType == ObjectType::Shader)
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