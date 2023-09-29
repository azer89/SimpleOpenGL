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

	// Temporary shaders
	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n\0";


public:
	int MainLoop();

private:
	unsigned int CreateShaderProgram(const char* source, GLenum shaderType);
};

#endif
