#include "AppBoxes.h"
#include "Shape.h"
#include "Shader.h"
#include "Texture.h"
#include "AppSettings.h"
#include "XMLReader.h"

#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

AppBoxes::AppBoxes()
{
}

int AppBoxes::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// Shader programs
	Shader shader("Misc//camera.vertex", "Misc//camera.fragment");

	// Texture
	Texture texture;
	texture.CreateFromImageFile(AppSettings::TextureFolder + "neco_coneco.jpg", true);

	// World space positions of our cubes
	glm::vec3 cubePositions[]{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	// Cube
	Cube cube;

	// Texture
	shader.Use();
	shader.SetInt("texture1", 0);

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		texture.Bind(0);

		shader.Use();
		shader.SetMat4("projection", camera->GetProjectionMatrix());
		shader.SetMat4("view", camera->GetViewMatrix());

		// Render boxes
		for (unsigned int i = 0; i < 10; ++i)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f); // Make sure to initialize matrix to identity matrix first
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			model = glm::scale(model, glm::vec3(0.5));
			shader.SetMat4("model", model);
			cube.Draw();
		}

		SwapBuffers();
	}

	shader.Delete();

	// GLFW: terminate, clearing all previously allocated GLFW resources.
	Terminate();

	return 0;
}