#include "AppPBRModel.h"
#include "AppSettings.h"
#include "Utility.h"
#include "Light.h"
#include "Shape.h"

int AppPBRModel::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	Shader shader("PBR//pbr.vertex", "PBR//pbr_emissive.fragment");

	glm::mat4 projection = camera->GetProjectionMatrix();
	shader.Use();
	shader.SetMat4("projection", projection);
	InitScene();

	std::vector<Light> lights;
	lights.emplace_back(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(150.0f, 150.0f, 150.0f));
	lights.emplace_back(glm::vec3(0.0f, 2.0f, 10.0f), glm::vec3(150.0f, 150.0f, 150.0f));
	lights.emplace_back(glm::vec3(0.0f, -2.0f, 10.0f), glm::vec3(150.0f, 150.0f, 150.0f));

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		shader.Use();
		shader.SetMat4("view", camera->GetViewMatrix());
		shader.SetVec3("camPos", camera->Position);

		for (unsigned int i = 0; i < lights.size(); i++)
		{
			shader.SetVec3("lightPositions[" + std::to_string(i) + "]", lights[i].Position);
			shader.SetVec3("lightColors[" + std::to_string(i) + "]", lights[i].Color);
		}

		RenderScene(shader);

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppPBRModel::InitScene()
{
	renderModel = std::make_unique<Model>(AppSettings::ModelFolder + "DamagedHelmet//DamagedHelmet.gltf");
}

void AppPBRModel::RenderScene(const Shader& shader)
{
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	shader.SetMat4("model", modelMatrix);
	bool skipTextureBinding = false;
	renderModel->Draw(shader, skipTextureBinding);
}