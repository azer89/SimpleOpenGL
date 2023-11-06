#ifndef __APP_BASE_H__
#define __APP_BASE_H__

#include <memory>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
	bool IsGLFWWindowCreated();
	bool IsGLADLoaded();
	int GLFWWindowShouldClose();
	void ProcessTiming();
	void ProcessInput();
	void SwapBuffers();
	void PollEvents();
	void Terminate();

	std::vector<float> GenerateCubeVertices();
	std::vector<float> GenerateQuadVertices();

private:
	GLFWwindow* glfwWindow;
	bool isGladLoaded = false;

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
