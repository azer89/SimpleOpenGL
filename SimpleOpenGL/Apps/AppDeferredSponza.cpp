#include "AppDeferredSponza.h"
#include "AppSettings.h"
#include "PipelineDeferred.h"
#include "Utility.h"

#include <memory>

int AppDeferredSponza::MainLoop()
{
	glEnable(GL_DEPTH_TEST);
	
	PipelineDeferred pipeline(
		"Deferred//deferred_g_buffer.vertex", "Deferred//deferred_g_buffer.fragment",
		"Deferred//deferred_lighting.vertex", "Deferred//deferred_lighting.fragment"
	);
	InitLights();
	InitScene();

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);


		glm::mat4 projection = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();

		// 1 Geometry pass
		pipeline.StartGeometryPass(projection, view);
		const Shader* gBufferShaderPtr = pipeline.GetGBufferShader();
		RenderScene(*gBufferShaderPtr);
		pipeline.EndGeometryPass();

		// 2 Lighting Pass
		UpdateLightPositions();
		pipeline.StartLightingPass(lights, camera->Position);

		// 3 Copy content of geometry's depth buffer to default framebuffer's depth buffer
		pipeline.Blit();

		// Render lights
		RenderLights();

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppDeferredSponza::InitLights()
{
	lightSphereShader = std::make_unique<Shader>("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	
	float pi2 = glm::two_pi<float>();

	constexpr unsigned int NR_LIGHTS = 100;
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
			Utility::RandomNumber<float>(0.5f, 1.0f),
			Utility::RandomNumber<float>(0.5f, 1.0f),
			Utility::RandomNumber<float>(0.5f, 1.0f)
		);

		lightAngles.push_back(rad);
		lightRadii.push_back(radius);
		lights.emplace_back(position, color, true, 0.1f);
	}

	lightSphereShader->Use();
}

void AppDeferredSponza::UpdateLightPositions()
{
	for (unsigned int i = 0; i < lights.size(); ++i)
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
	for (unsigned int i = 0; i < lights.size(); ++i)
	{
		lights[i].Render(*lightSphereShader);
	}
}

void AppDeferredSponza::RenderScene(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	sponzaModel->Draw(shader);
}