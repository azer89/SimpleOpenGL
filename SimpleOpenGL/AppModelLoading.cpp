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

	// Shader programs
	Shader shader("model_loading.vertex", "model_loading.fragment");

	Model obj(AppSettings::ModelFolder + "DamagedHelmet//DamagedHelmet.gltf");

	glm::vec3 lightPos(0.0f, 2.0f, 0.0f);

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();

		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)AppSettings::ScreenWidth / (float)AppSettings::ScreenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix();
		shader.SetMat4("projection", projection);
		shader.SetMat4("view", view);
		shader.SetVec3("viewPos", camera->Position);
		shader.SetVec3("lightPos", lightPos);

		// Render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
		//glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		shader.SetMat4("model", model);
		obj.Draw(shader);

		SwapBuffers();
		PollEvents();
	}

	shader.Delete();

	// GLFW: terminate, clearing all previously allocated GLFW resources.
	Terminate();

	return 0;
}