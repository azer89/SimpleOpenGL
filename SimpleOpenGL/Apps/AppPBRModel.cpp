#include "AppPBRModel.h"
#include "AppSettings.h"
#include "Utility.h"
#include "Light.h"
#include "Shape.h"

#include "glm/gtc/matrix_transform.hpp"

int AppPBRModel::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	Shader shader("PBR//pbr.vertex", "PBR//pbr_emissive.fragment");
	Shader lightSphereShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");

	glm::mat4 projection = camera->GetProjectionMatrix();
	shader.Use();
	shader.SetMat4("projection", projection);
	InitScene();

	std::vector<Light> lights;
	lights.emplace_back(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(150.0f, 40.0f, 40.0f));
	lights.emplace_back(glm::vec3(5.f, 0.0f, 5.0f), glm::vec3(40.0f, 150.0f, 40.0f));
	lights.emplace_back(glm::vec3(-5.f, 0.0f, 5.0f), glm::vec3(40.0f, 40.0f, 150.0f));

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		shader.Use();
		shader.SetMat4("view", camera->GetViewMatrix());
		shader.SetVec3("camPos", camera->Position);

		for (unsigned int i = 0; i < lights.size(); ++i)
		{
			shader.SetVec3("lightPositions[" + std::to_string(i) + "]", lights[i].Position);
			shader.SetVec3("lightColors[" + std::to_string(i) + "]", lights[i].Color);
		}

		RenderScene(shader);

		lightSphereShader.Use();
		lightSphereShader.SetMat4("projection", camera->GetProjectionMatrix());
		lightSphereShader.SetMat4("view", camera->GetViewMatrix());
		for (auto& l : lights)
		{
			l.Render(lightSphereShader);
		}

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppPBRModel::InitScene()
{
	renderModel1 = std::make_unique<Model>(AppSettings::ModelFolder + "DamagedHelmet//DamagedHelmet.gltf");
	renderModel2 = std::make_unique<Model>(AppSettings::ModelFolder + "Tachikoma//Tachikoma.gltf");
	renderModel3 = std::make_unique<Model>(AppSettings::ModelFolder + "SciFiHelmet//SciFiHelmet.gltf");
}

void AppPBRModel::RenderScene(const Shader& shader) const
{
	bool skipTextureBinding = false;

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	shader.SetMat4("model", modelMatrix);
	renderModel1->Draw(shader, skipTextureBinding);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.5f, 0.0f, 0.0f));
	shader.SetMat4("model", modelMatrix);
	renderModel2->Draw(shader, skipTextureBinding);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(2.5f, 0.0f, 0.0f));
	shader.SetMat4("model", modelMatrix);
	renderModel3->Draw(shader, skipTextureBinding);
}