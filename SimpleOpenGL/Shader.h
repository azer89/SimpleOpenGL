#ifndef __SHADER_H__
#define __SHADER_H__

#include <iostream>

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

	// Activate the shader
	void Use();

	void Delete();

	// Utility uniform functions
	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;

private:
	void CheckCompileErrors(unsigned int shader, ObjectType objectType);
};

#endif
