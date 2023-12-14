#include "AppEdge.h"
#include "Shader.h"
#include "Model.h"
#include "Light.h"
#include "AppSettings.h"

int AppEdge::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	Shader mainShader("ModelLoading//model_loading.vertex", "ModelLoading//model_loading.fragment");
	Shader lightCubeShader("Misc//light_cube.vertex", "Misc//light_cube.fragment");
	Model obj(AppSettings::ModelFolder + "Tachikoma//scene.gltf");

	//glm::vec3 lightPos(0.0f, 0.5f, 5.0f);
	Shader lightShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	Light light(glm::vec3(0.0f, 0.5f, 5.0f), glm::vec3(1.0f));

	auto modelRotation = 0.0f;

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		auto projection = camera->GetProjectionMatrix();
		auto view = camera->GetViewMatrix();

		// Object
		mainShader.Use();
		mainShader.SetMat4("projection", projection);
		mainShader.SetMat4("view", view);
		mainShader.SetVec3("viewPos", camera->Position);
		mainShader.SetVec3("lightPos", light.Position);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::rotate(model, modelRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		modelRotation += deltaTime * 0.2f;
		mainShader.SetMat4("model", model);
		obj.Draw(mainShader);

		// Light
		lightShader.Use();
		lightShader.SetMat4("projection", projection);
		lightShader.SetMat4("view", view);
		light.Render(lightShader);

		SwapBuffers();
	}

	Terminate();

	return 0;
}