#ifndef SHADER
#define SHADER

#include <string>

#include "glad/glad.h"
#include "glm/glm.hpp"

enum class ObjectType : uint8_t
{
	Shader = 0u,
	Program = 1u,
};

class Shader
{
public:
	unsigned int ID{};

public:
	Shader(const char* vertexFilename,
		const char* fragmentFilename,
		const char* geometryFilename = nullptr);
	
	void Use() const;  // Activate the shader
	void Delete() const;

	// Utility uniform functions
	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetVec2(const std::string& name, const glm::vec2& value) const;
	void SetVec2(const std::string& name, float x, float y) const;
	void SetVec3(const std::string& name, const glm::vec3& value) const;
	void SetVec3(const std::string& name, float x, float y, float z) const;
	void SetVec4(const std::string& name, const glm::vec4& value) const;
	void SetVec4(const std::string& name, float x, float y, float z, float w) const;
	void SetMat2(const std::string& name, const glm::mat2& mat) const;
	void SetMat3(const std::string& name, const glm::mat3& mat) const;
	void SetMat4(const std::string& name, const glm::mat4& mat) const;

private:
	void CheckCompileErrors(unsigned int shader, ObjectType objectType);
	[[nodiscard]] unsigned int CreateShaderProgram(const char* source, GLenum shaderType);
	[[nodiscard]] std::string LoadTextFile(const char* filePath);
};

#endif
