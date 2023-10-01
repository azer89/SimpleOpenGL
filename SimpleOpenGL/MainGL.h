#ifndef __MAIN_GL_H__
#define __MAIN_GL_H__

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class MainGL
{
private:
	// Settings
	const unsigned int SCREEN_WIDTH = 800;
	const unsigned int SCREEN_HEIGHT = 600;
	const char* SCREEN_TITLE = "Simple OpenGL";

	const char* VERTEX_SHADER_FILE = "C:/Users/azer/workspace/SimpleOpenGL/Shaders/shader.vertex";
	const char* FRAGMENT_SHADER_FILE = "C:/Users/azer/workspace/SimpleOpenGL/Shaders/shader.fragment";

public:
	int MainLoop();

private:
	unsigned int CreateShaderProgram(const char* source, GLenum shaderType) const;
};

#endif
