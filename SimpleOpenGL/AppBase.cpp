#include "AppBase.h"
#include "AppSettings.h"
#include "XMLReader.h"

#include <iostream>

AppBase::AppBase()
{
	XMLReader::LoadSettings();
	InitGLFW();
	InitGlad();
	InitCamera();
	InitTiming();
}

void AppBase::InitGLFW()
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

void AppBase::InitGlad()
{
	// GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		isGladLoaded = false;
		return;
	}
	isGladLoaded = true;
}

void AppBase::InitCamera()
{
	// Can change the camera position later
	camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
	lastX = AppSettings::ScreenWidth / 2.0f;
	lastY = AppSettings::ScreenHeight / 2.0f;
	firstMouse = true;
}

void AppBase::InitTiming()
{
	deltaTime = 0.0f;	// Time between current frame and last frame
	lastFrame = 0.0f;
}

bool AppBase::IsGLFWWindowCreated()
{
	return glfwWindow != NULL;
}

bool AppBase::IsGLADLoaded()
{
	return isGladLoaded;
}

int AppBase::GLFWWindowShouldClose()
{
	return glfwWindowShouldClose(glfwWindow);
}

void AppBase::ProcessTiming()
{
	// Per-frame time
	float currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

void AppBase::SwapBuffers()
{
	// GLFW: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(glfwWindow);
}

void AppBase::PollEvents()
{
	glfwPollEvents();
}

void AppBase::Terminate()
{
	glfwTerminate();
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
void AppBase::ProcessInput()
{
	if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(glfwWindow, true);
	}

	if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera->ProcessKeyboard(CameraForward, deltaTime);
	}

	if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera->ProcessKeyboard(CameraBackward, deltaTime);
	}

	if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera->ProcessKeyboard(CameraLeft, deltaTime);
	}

	if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera->ProcessKeyboard(CameraRight, deltaTime);
	}
}

// Example here
/*int AppDerived::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SwapBuffers();
		PollEvents();
	}

	Terminate();

	return 0;
}*/