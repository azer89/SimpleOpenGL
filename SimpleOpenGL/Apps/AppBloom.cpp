#include "AppBloom.h"
#include "AppSettings.h"
#include "PipelineBloom.h"
#include "Shader.h"
#include "Model.h"
#include "Shape.h"
#include "Light.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>

int AppBloom::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	Shader lightShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	Light light(glm::vec3(0.0f, 0.5f, 5.0f), glm::vec3(1.f));

	Model obj(AppSettings::ModelFolder + "Zaku//scene.gltf");
	auto modelRotation = acos(-1.f);

	unsigned int blurIter = 50;
	PipelineBloom pipeline(blurIter);

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		auto projection = camera->GetProjectionMatrix();
		auto view = camera->GetViewMatrix();

		// First pass
		pipeline.StartFirstPass(
			projection,
			view,
			camera->Position,
			light.Position
		);
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::rotate(model, modelRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		Shader* mainShader = pipeline.GetMainShader();
		mainShader->SetMat4("model", model);
		obj.Draw(*mainShader);

		// End first pass
		pipeline.EndFirstPass();

		// Second pass
		pipeline.StartBlurPass();

		// Third pass
		pipeline.RenderComposite();

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