#ifndef __SHADER_H__
#define __SHADER_H__

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum ObjectType
{
	ShaderObject = 0,
	ProgramObject = 1,
};

class Shader
{
public:
	unsigned int ID;

public:
	Shader(const char* vertexPath, const char* fragmentPath);
	
	void Use(); // Activate the shader
	void Delete();

	// Utility uniform functions
	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetVec2(const std::string& name, const glm::vec2& value) const;
	void SetVec2(const std::string& name, float x, float y) const;
	void SetVec3(const std::string& name, const glm::vec3& value) const;
	void SetVec3(const std::string& name, float x, float y, float z) const;
	void SetVec4(const std::string& name, const glm::vec4& value) const;
	void SetVec4(const std::string& name, float x, float y, float z, float w);
	void SetMat2(const std::string& name, const glm::mat2& mat) const;
	void SetMat3(const std::string& name, const glm::mat3& mat) const;
	void SetMat4(const std::string& name, const glm::mat4& mat) const;

private:
	void CheckCompileErrors(unsigned int shader, ObjectType objectType);
	unsigned int CreateShaderProgram(const char* source, GLenum shaderType);
	std::string LoadTextFile(const char* filePath);
};

#endif
