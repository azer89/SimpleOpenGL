#include "AppModelLoading.h"
#include "AppSettings.h"
#include "Shader.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int AppModelLoading::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	Shader shader("model_loading.vertex", "model_loading.fragment");
	Model obj(AppSettings::ModelFolder + "DamagedHelmet//DamagedHelmet.gltf");

	glm::vec3 lightPos(0.0f, 10.0f, 0.0f);

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();
		shader.SetMat4("projection", camera->GetProjectionMatrix());
		shader.SetMat4("view", camera->GetViewMatrix());
		shader.SetVec3("viewPos", camera->Position);
		shader.SetVec3("lightPos", lightPos);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
		shader.SetMat4("model", model);
		obj.Draw(shader);

		SwapBuffers();
		PollEvents();
	}

	shader.Delete();

	Terminate();

	return 0;
}