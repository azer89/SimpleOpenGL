#ifndef __MAIN_GL_H__
#define __MAIN_GL_H__

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class MainGL
{
public:
	int MainLoop();

private:
	void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
	void ProcessInput(GLFWwindow* window);
};

#endif
