#include "AppBase.h"
#include "AppSettings.h"
#include "XMLReader.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

void APIENTRY GLDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam);

AppBase::AppBase()
{
	XMLReader::LoadSettings();
	InitGLFW();
	InitGlad();
	InitIMGUI();
	InitCamera();
	InitTiming();
}

void AppBase::InitGLFW()
{
	glslVersion = "#version 130";

	// GLFW: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	// GLFW window creation
	glfwWindow = glfwCreateWindow(AppSettings::ScreenWidth,
		AppSettings::ScreenHeight,
		AppSettings::ScreenTitle.c_str(),
		NULL,
		NULL);
	if (glfwWindow == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
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
		auto func = [](GLFWwindow* window, int button, int action, int mods)
			{
				static_cast<AppBase*>(glfwGetWindowUserPointer(window))->MouseButtonCallback(window, button, action, mods);
			};
		glfwSetMouseButtonCallback(glfwWindow, func);
	}
	{
		auto func = [](GLFWwindow* window, double xoffset, double yoffset)
			{
				static_cast<AppBase*>(glfwGetWindowUserPointer(window))->ScrollCallback(window, xoffset, yoffset);
			};
		glfwSetScrollCallback(glfwWindow, func);
	}
	{
		auto func = [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				static_cast<AppBase*>(glfwGetWindowUserPointer(window))->KeyCallback(window, key, scancode, action, mods);
			};
		glfwSetKeyCallback(glfwWindow, func);
	}
}

void AppBase::InitIMGUI()
{
	imguiShow = true;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
	ImGui_ImplOpenGL3_Init(glslVersion.c_str());
}

void AppBase::InitGlad()
{
	// GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	// Enable OpenGL debug context if context allows for debug context
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
		glDebugMessageCallback(GLDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	// Tell GLFW to capture our mouse
	glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void AppBase::InitCamera()
{
	// Can change the camera position later
	camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
	mouseLastX = static_cast<float>(AppSettings::ScreenWidth) / 2.0f;
	mouseLastY = static_cast<float>(AppSettings::ScreenHeight) / 2.0f;
	mouseFirstUse = true;
}

void AppBase::InitTiming()
{
	deltaTime = 0.0f;	// Time between current frame and last frame
	lastFrame = 0.0f;
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

void AppBase::ProcessLoop(glm::vec4 clearColor, GLbitfield mask)
{
	PollEvents();
	ProcessTiming();
	ProcessInput();

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClear(mask);
}

void AppBase::Terminate()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(glfwWindow);
	glfwTerminate();
}

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void AppBase::FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// Make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void AppBase::MouseCallback(GLFWwindow* window, double xPosIn, double yPosIn)
{
	if (!mousePressLeft)
	{
		return;
	}

	float xpos = static_cast<float>(xPosIn);
	float ypos = static_cast<float>(yPosIn);
	if (mouseFirstUse)
	{
		mouseLastX = xpos;
		mouseLastY = ypos;
		mouseFirstUse = false;
	}
	float xOffset = xpos - mouseLastX;
	float yOffset = mouseLastY - ypos; // reversed since y-coordinates go from bottom to top
	mouseLastX = xpos;
	mouseLastY = ypos;
	camera->ProcessMouseMovement(xOffset, yOffset);
}

void AppBase::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (const auto& io = ImGui::GetIO(); io.WantCaptureMouse)
	{
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		mousePressLeft = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		mousePressLeft = false;
		mouseFirstUse = true;
	}
}

void AppBase::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	camera->ProcessMouseScroll(static_cast<float>(yOffset));
}

void AppBase::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		// Toggle imgui window
		imguiShow = !imguiShow;
	}
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
	glEnable(GL_DEPTH_TEST);

	// Init

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		// Code

		SwapBuffers();
	}

	Terminate();

	return 0;
}*/

void APIENTRY GLDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore these non-significant error codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
	{
		return;
	}
	
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	}
	
	std::cout << std::endl;
	std::cout << std::endl;
}

