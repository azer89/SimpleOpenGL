#include "AppBase.h"
#include "AppSettings.h"
#include "XMLReader.h"

#include <iostream>

AppBase::AppBase()
{
	XMLReader::LoadSettings();
	SetupGLFW();
}

void AppBase::SetupGLFW()
{
	// GLFW: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// GLFW window creation
	glfwWindow = glfwCreateWindow(AppSettings::ScreenWidth,
		AppSettings::ScreenHeight,
		AppSettings::ScreenTitle.c_str(),
		NULL,
		NULL);
	if (glfwWindow == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(glfwWindow);

	glfwSetWindowUserPointer(glfwWindow, this);
	{
		auto func = [](GLFWwindow* window, int width, int height)
		{
			static_cast<AppBase*>(glfwGetWindowUserPointer(window))->FrameBufferSizeCallback(window, width, height);
		};
		glfwSetFramebufferSizeCallback(glfwWindow, func);
	}
	{
		auto func = [](GLFWwindow* window, double xpos, double ypos)
		{
			static_cast<AppBase*>(glfwGetWindowUserPointer(window))->MouseCallback(window, xpos, ypos);
		};
		glfwSetCursorPosCallback(glfwWindow, func);
	}
	{
		auto func = [](GLFWwindow* window, double xoffset, double yoffset)
		{
			static_cast<AppBase*>(glfwGetWindowUserPointer(window))->ScrollCallback(window, xoffset, yoffset);
		};
		glfwSetScrollCallback(glfwWindow, func);
	}

	// Tell GLFW to capture our mouse
	glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void AppBase::FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// Make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void AppBase::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;
	camera->ProcessMouseMovement(xoffset, yoffset);
}

void AppBase::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void AppBase::ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera->ProcessKeyboard(CameraForward, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera->ProcessKeyboard(CameraBackward, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera->ProcessKeyboard(CameraLeft, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera->ProcessKeyboard(CameraRight, deltaTime);
	}
}