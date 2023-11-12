#include "AppDeferredSponza.h"
#include "AppSettings.h"
#include "ShapeFactory.h"
#include "GBuffer.h"
#include "UsefulStuff.h"

#include <memory>

const unsigned int NR_LIGHTS = 100;

int AppDeferredSponza::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	
	GBuffer gBuffer;
	InitLights();
	InitScene();

	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();

		// 1 Geometry pass
		gBuffer.StartGeometryPass(projection, view);
		Shader* geomShaderPtr = gBuffer.GetGeometryShader();
		RenderScene(*geomShaderPtr);
		gBuffer.EndGeometryPass();

		// 2 Lighting Pass
		UpdateLightPositions();
		gBuffer.StartLightingPass(lights, camera->Position);

		// 3 Copy content of geometry's depth buffer to default framebuffer's depth buffer
		gBuffer.Blit();

		// Render lights
		RenderLights();

		SwapBuffers();
		PollEvents();
	}

	Terminate();

	return 0;
}

void AppDeferredSponza::InitLights()
{
	lightSphereShader = std::make_unique<Shader>("light_sphere.vertex", "light_sphere.fragment");
	
	float pi2 = glm::two_pi<float>();

	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		float yPos = UsefulStuff::RandomNumber<float>(0.15f, 10.0f);
		float radius = UsefulStuff::RandomNumber<float>(0.0f, 20.0f);
		float rad = UsefulStuff::RandomNumber<float>(0.0f, pi2);
		float xPos = glm::cos(rad);

		glm::vec3 position(
			glm::cos(rad) * radius,
			yPos,
			glm::sin(rad) * radius
		);

		glm::vec3 color(
			UsefulStuff::RandomNumber<float>(0.5f, 1.0f),
			UsefulStuff::RandomNumber<float>(0.5f, 1.0f),
			UsefulStuff::RandomNumber<float>(0.5f, 1.0f)
		);

		lightAngles.push_back(rad);
		lightRadii.push_back(radius);
		lights.emplace_back(position, color, true, 0.1f);
	}

	lightSphereShader->Use();
	lightSphereShader->SetFloat("radius", 0.2f);
}

void AppDeferredSponza::UpdateLightPositions()
{
	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		float step = deltaTime * 0.5f;

		float yPos = lights[i].Position.y;
		lightAngles[i] += step;

		lights[i].Position = glm::vec3(
			glm::cos(lightAngles[i]) * lightRadii[i],
			yPos,
			glm::sin(lightAngles[i]) * lightRadii[i]
		);
	}
}

void AppDeferredSponza::InitScene()
{
	sponzaModel = std::make_unique<Model>(AppSettings::ModelFolder + "Sponza//Sponza.gltf");
}

void AppDeferredSponza::RenderLights()
{
	lightSphereShader->Use();
	lightSphereShader->SetMat4("projection", camera->GetProjectionMatrix());
	lightSphereShader->SetMat4("view", camera->GetViewMatrix());
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		lights[i].Render(*lightSphereShader);
	}
}

void AppDeferredSponza::RenderScene(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(10.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.02f));
	shader.SetMat4("model", model);
	sponzaModel->Draw(shader);
}