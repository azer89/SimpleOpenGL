#include "AppPBRTextured.h"
#include "AppSettings.h"
#include "Utility.h"
#include "Light.h"
#include "Shape.h"

int AppPBRTextured::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	const Shader shader("PBR//pbr.vertex", "PBR//pbr.fragment");

	const glm::mat4 projection = camera->GetProjectionMatrix();
	shader.Use();
	shader.SetMat4("projection", projection);
	InitScene();

	std::vector<Light> lights{};
	lights.emplace_back(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(150.0f, 150.0f, 150.0f));

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
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

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppPBRTextured::InitScene()
{
	dragonModel = std::make_unique<Model>(AppSettings::ModelFolder + "Dragon//Dragon.obj");

	dragonModel->AddTextureIfEmpty(TextureType::DIFFUSE, AppSettings::TextureFolder + "pbr//rusted_iron//albedo.png");
	dragonModel->AddTextureIfEmpty(TextureType::NORMAL, AppSettings::TextureFolder + "pbr//rusted_iron//normal.png");
	dragonModel->AddTextureIfEmpty(TextureType::METALNESS, AppSettings::TextureFolder + "pbr//rusted_iron//metallic.png");
	dragonModel->AddTextureIfEmpty(TextureType::ROUGHNESS, AppSettings::TextureFolder + "pbr//rusted_iron//roughness.png");
	dragonModel->AddTextureIfEmpty(TextureType::AO, AppSettings::TextureFolder + "pbr//rusted_iron//ao.png");
	dragonModel->AddTextureIfEmpty(TextureType::EMISSIVE, AppSettings::TextureFolder + "Black1x1.png");
}

void AppPBRTextured::RenderScene(const Shader& shader) const
{
	const glm::mat4 model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	constexpr bool skipTextureBinding = false;
	dragonModel->Draw(shader, skipTextureBinding);
}