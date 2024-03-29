#ifndef APP_BASE
#define APP_BASE

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
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	
	void InitIMGUI();
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

	std::string glslVersion;

protected:
	// Camera
	std::unique_ptr<Camera> camera;
	float lastX;
	float lastY;
	bool firstMouse;
	bool leftMousePressed;
	bool showImgui;

	// Timing
	float deltaTime; // Time between current frame and last frame
	float lastFrame;
};

#endif
