#ifndef __APP_BASE_H__
#define __APP_BASE_H__

#include <memory>
#include <string>
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Camera.h"

class AppBase
{
public:
	AppBase();
	virtual int MainLoop() = 0; // Abstract class

protected:
	void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
	void MouseCallback(GLFWwindow* window, double xpos, double ypos);
	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	
	void InitGLFW();
	void InitGlad();
	void InitCamera();
	void InitTiming();
	int GLFWWindowShouldClose();
	void SwapBuffers();
	void Terminate();

	void ProcessLoop(glm::vec4 clearColor, GLbitfield mask);
	void PollEvents();
	void ProcessTiming();
	void ProcessInput();

private:
	GLFWwindow* glfwWindow;

protected:
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
