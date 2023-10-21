#ifndef __MAIN_GL_H__
#define __MAIN_GL_H__

#include <memory>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"

class MainGL
{
public:
	int MainLoop();

private:
	void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
	void MouseCallback(GLFWwindow* window, double xpos, double ypos);
	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	void ProcessInput(GLFWwindow* window);

private:
	// Camera
	std::unique_ptr<Camera> camera;
	float lastX;
	float lastY;
	bool firstMouse;

	// Timing
	float deltaTime; // Time between current frame and last frame
	float lastFrame;

};

#endif
