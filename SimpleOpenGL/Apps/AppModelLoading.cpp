#include "AppModelLoading.h"
#include "AppSettings.h"
#include "Shader.h"
#include "Model.h"
#include "Shape.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

int AppModelLoading::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	const Cube cube{};

	Shader mainShader("ModelLoading//model_loading.vertex", "ModelLoading//model_loading.fragment");
	const Shader lightCubeShader("Misc//light_cube.vertex", "Misc//light_cube.fragment");
	Model obj(AppSettings::ModelFolder + "Tachikoma//Tachikoma.gltf");

	const glm::vec3 lightPos(0.0f, 0.5f, 5.0f);

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
		mainShader.SetVec3("lightPos", lightPos);
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::rotate(model, modelRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		modelRotation += deltaTime;
		mainShader.SetMat4("model", model);
		obj.Draw(mainShader);

		// Cube light
		lightCubeShader.Use();
		lightCubeShader.SetMat4("projection", projection);
		lightCubeShader.SetMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightCubeShader.SetMat4("model", model);
		cube.Draw();

		SwapBuffers();
	}

	mainShader.Delete();

	Terminate();

	return 0;
}