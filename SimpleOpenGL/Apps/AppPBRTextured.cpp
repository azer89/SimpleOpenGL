#include "AppPBRTextured.h"
#include "AppSettings.h"
#include "Utility.h"
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
	shader.SetInt("albedoMap", 0);
	shader.SetInt("normalMap", 1);
	shader.SetInt("metallicMap", 2);
	shader.SetInt("roughnessMap", 3);
	shader.SetInt("aoMap", 4);

	// Sphere
	constexpr int nrRows = 7;
	constexpr int nrColumns = 7;
	constexpr float spacing = 2.5;
	Sphere sphere;

	glm::mat4 projection = camera->GetProjectionMatrix();
	shader.Use();
	shader.SetMat4("projection", projection);

	//InitScene();
	//InitLights();
	glm::vec3 lightPositions[] = {
		glm::vec3(0.0f, 0.0f, 10.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(150.0f, 150.0f, 150.0f),
	};

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

		glm::mat4 model = glm::mat4(1.0f);
		for (int row = 0; row < nrRows; ++row)
		{
			for (int col = 0; col < nrColumns; ++col)
			{
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(
					(float)(col - (nrColumns / 2)) * spacing,
					(float)(row - (nrRows / 2)) * spacing,
					0.0f
				));
				shader.SetMat4("model", model);
				shader.SetMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
				sphere.Draw();
			}
		}

		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
			newPos = lightPositions[i];
			shader.SetVec3("lightPositions[" + std::to_string(i) + "]", newPos);
			shader.SetVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
		}

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppPBRTextured::InitScene()
{
	sponzaModel = std::make_unique<Model>(AppSettings::ModelFolder + "Sponza//Sponza.gltf");
	adamModel = std::make_unique<Model>(AppSettings::ModelFolder + "adamHead//adamHead.gltf");
}

void AppPBRTextured::RenderScene(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(8.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(2.0f));
	shader.SetMat4("model", model);
	sponzaModel->Draw(shader);

	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(0.0f, 0.75f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.SetMat4("model", model);
	adamModel->Draw(shader);
}

void AppPBRTextured::InitLights()
{
	lightSphereShader = std::make_unique<Shader>("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");

	float pi2 = glm::two_pi<float>();

	const int NR_LIGHTS = 200;
	for (unsigned int i = 0; i < NR_LIGHTS; ++i)
	{
		float yPos = Utility::RandomNumber<float>(0.15f, 10.0f);
		float radius = Utility::RandomNumber<float>(0.0f, 20.0f);
		float rad = Utility::RandomNumber<float>(0.0f, pi2);
		float xPos = glm::cos(rad);

		glm::vec3 position(
			glm::cos(rad) * radius,
			yPos,
			glm::sin(rad) * radius
		);

		glm::vec3 color(
			Utility::RandomNumber<float>(0.2f, 0.5f),
			Utility::RandomNumber<float>(0.2f, 0.5f),
			Utility::RandomNumber<float>(0.5f, 1.0f)
		);

		lightAngles.push_back(rad);
		lightRadii.push_back(radius);
		lights.emplace_back(position, color, true, 0.1f);
	}

	lightSphereShader->Use();
	lightSphereShader->SetFloat("radius", 0.2f);
}

void AppPBRTextured::RenderLights()
{
	lightSphereShader->Use();
	lightSphereShader->SetMat4("projection", camera->GetProjectionMatrix());
	lightSphereShader->SetMat4("view", camera->GetViewMatrix());
	for (Light& light : lights)
	{
		light.Render(*lightSphereShader);
	}
}