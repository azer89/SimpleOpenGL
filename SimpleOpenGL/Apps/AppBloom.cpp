#include "AppBloom.h"
#include "AppSettings.h"
#include "Shader.h"
#include "Model.h"
#include "Shape.h"
#include "Light.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int AppBloom::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	Cube cube;

	Shader mainShader("Bloom//first_pass.vertex", "Bloom//first_pass.fragment");
	Shader lightShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	Model obj(AppSettings::ModelFolder + "Zaku//scene.gltf");

	Light light(glm::vec3(0.0f, 0.5f, 5.0f), glm::vec3(1.f));

	auto modelRotation = acos(-1.f);

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
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
		mainShader.SetMat4("model", model);
		obj.Draw(mainShader);

		// Cube light
		lightShader.Use();
		lightShader.SetMat4("projection", projection);
		lightShader.SetMat4("view", view);
		light.Render(lightShader);

		SwapBuffers();
	}

	mainShader.Delete();

	Terminate();

	return 0;
}