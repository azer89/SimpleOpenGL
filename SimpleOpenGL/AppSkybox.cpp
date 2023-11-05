#include "AppSkybox.h"

int AppSkybox::MainLoop()
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
}