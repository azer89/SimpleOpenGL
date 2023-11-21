#include "AppPBRTextured.h"
#include "AppSettings.h"
#include "Utility.h"
#include "Light.h"
#include "Shape.h"

int AppPBRTextured::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	Shader shader("PBRTextured//pbr.vertex", "PBRTextured//pbr.fragment");

	Texture albedo(AppSettings::TextureFolder + "pbr//rusted_iron//albedo.png");
	Texture normal(AppSettings::TextureFolder + "pbr//rusted_iron//normal.png");
	Texture metallic(AppSettings::TextureFolder + "pbr//rusted_iron//metallic.png");
	Texture roughness(AppSettings::TextureFolder + "pbr//rusted_iron//roughness.png");
	Texture ao(AppSettings::TextureFolder + "pbr//rusted_iron//ao.png");

	shader.Use();
	shader.SetInt("texture_diffuse1", 0);
	shader.SetInt("texture_normal1", 1);
	shader.SetInt("texture_metalness1", 2);
	shader.SetInt("texture_roughness1", 3);
	shader.SetInt("texture_ao1", 4);

	glm::mat4 projection = camera->GetProjectionMatrix();
	shader.Use();
	shader.SetMat4("projection", projection);
	InitScene();

	std::vector<Light> lights;
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

		albedo.Bind(GL_TEXTURE0);
		normal.Bind(GL_TEXTURE1);
		metallic.Bind(GL_TEXTURE2);
		roughness.Bind(GL_TEXTURE3);
		ao.Bind(GL_TEXTURE4);

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

void AppPBRTextured::InitScene()
{
	dragonModel = std::make_unique<Model>(AppSettings::ModelFolder + "Dragon//Dragon.obj");
}

void AppPBRTextured::RenderScene(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	bool skipTextureBinding = true;
	dragonModel->Draw(shader, skipTextureBinding);
}