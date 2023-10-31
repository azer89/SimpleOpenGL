#include "AppShadowMapping.h"

int AppShadowMapping::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	Shader shader("shadow_mapping.vertex", "shadow_mapping.fragment");
	Shader depthShader("shadow_mapping_depth.vertex", "shadow_mapping_depth.fragment");
	Shader debugShader("shadow_mapping_debug.vertex", "shadow_mapping_debug.fragment");

	// Render loop
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