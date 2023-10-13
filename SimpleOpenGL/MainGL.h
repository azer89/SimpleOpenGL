#ifndef __MAIN_GL_H__
#define __MAIN_GL_H__

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class MainGL
{
private:

public:
	int MainLoop();

private:
	unsigned int CreateShaderProgram(const char* source, GLenum shaderType) const;
};

#endif
