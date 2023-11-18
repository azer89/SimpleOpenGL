#include "AppPBRTextured.h"
#include "AppSettings.h"
#include "Utility.h"

int AppPBRTextured::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader shader("PBRTextured//pbr.vertex", "PBRTextured//pbr.fragment");

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
	InitLights();

	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();
		shader.SetMat4("view", camera->GetViewMatrix());
		shader.SetVec3("camPos", camera->Position);

		for (unsigned int i = 0; i < lights.size(); i++)
		{
			shader.SetVec3("lightPositions[" + std::to_string(i) + "]", lights[i].Position);
			shader.SetVec3("lightColors[" + std::to_string(i) + "]", lights[i].Color);
		}

		RenderScene(shader);
		RenderLights();

		SwapBuffers();
		PollEvents();
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