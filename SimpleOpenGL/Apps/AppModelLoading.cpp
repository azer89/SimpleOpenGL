#include "AppModelLoading.h"
#include "AppSettings.h"
#include "Shader.h"
#include "Model.h"
#include "ShapeFactory.h"

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

	InitLightCube();

	Shader mainShader("ModelLoading//model_loading.vertex", "ModelLoading//model_loading.fragment");
	Shader lightCubeShader("Misc//light_cube.vertex", "Misc//light_cube.fragment");
	Model obj(AppSettings::ModelFolder + "DamagedHelmet//DamagedHelmet.gltf");

	glm::vec3 lightPos(0.0f, 1.5f, 1.5f);

	auto modelRotation = 0.0f;

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto projection = camera->GetProjectionMatrix();
		auto view = camera->GetViewMatrix();

		// Object
		mainShader.Use();
		mainShader.SetMat4("projection", projection);
		mainShader.SetMat4("view", view);
		mainShader.SetVec3("viewPos", camera->Position);
		mainShader.SetVec3("lightPos", lightPos);
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	
		model = glm::rotate(model, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, modelRotation, glm::vec3(0.0f, 0.0f, 1.0f));
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
		glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		SwapBuffers();
		PollEvents();
	}

	mainShader.Delete();

	Terminate();

	return 0;
}

void AppModelLoading::InitLightCube()
{
	auto vertices = ShapeFactory::GenerateCubeVertices();

	glGenBuffers(1, &lightCubeVBO);

	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
}