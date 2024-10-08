#include "AppDeferredSSAO.h"
#include "AppSettings.h"
#include "Utility.h"
#include "PipelineDeferredSSAO.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glm/gtc/matrix_transform.hpp"

#include <random>

int AppDeferredSSAO::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	InitScene();
	InitLights();

	constexpr int noiseSize = 4;
	constexpr int kernelSize = 64;
	float radius = 0.5f;
	float bias = 0.025f;

	PipelineDeferredSSAO pipeline(
		"SSAO//geometry.vertex", "SSAO//geometry.fragment",
		"SSAO//ssao.vertex", "SSAO//lighting.fragment",
		"SSAO//ssao.vertex", "SSAO//ssao.fragment",
		"SSAO//ssao.vertex", "SSAO//blur.fragment",
		kernelSize,
		noiseSize
	);

	// Game loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		glm::mat4 projection = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();

		// 1 Geometry pass: render scene's geometry/color data into G buffer
		pipeline.StartGeometryPass(projection, view);
		RenderScene(*(pipeline.GetGeometryShader()));
		pipeline.EndGeometryPass();

		// 2 SSAO
		pipeline.StartSSAOPass(projection, kernelSize, radius, bias);

		// 3 Blur SSAO texture to remove noise
		pipeline.StartBlurPass();

		//UpdateLightPositions();

		// 4 lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
		pipeline.StartLightingPass(lights, view, camera->Position);

		// 5 Blit
		pipeline.Blit();

		RenderLights();

		if (imguiShow)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::SetNextWindowSize(ImVec2(500, 100));

			ImGui::Begin("SSAO");

			ImGui::SliderFloat("Radius", &radius, 0.0f, 50.0f);
			ImGui::SliderFloat("Bias", &bias, 0.0f, 0.5f);

			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppDeferredSSAO::InitScene()
{
	sponzaModel = std::make_unique<Model>(AppSettings::ModelFolder + "Sponza//Sponza.gltf");
}

void AppDeferredSSAO::RenderScene(const Shader& shader) const
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(8.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(2.0f));
	shader.SetMat4("model", model);
	sponzaModel->Draw(shader);
}

void AppDeferredSSAO::InitLights()
{
	lightSphereShader = std::make_unique<Shader>("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");

	const float pi2 = glm::two_pi<float>();
	constexpr uint32_t NR_LIGHTS = 200;
	for (uint32_t i = 0; i < NR_LIGHTS; ++i)
	{
		const float yPos = Utility::RandomNumber<float>(0.15f, 10.0f);
		const float radius = Utility::RandomNumber<float>(5.0f, 15.0f);
		const float rad = Utility::RandomNumber<float>(0.0f, pi2);
		float xPos = glm::cos(rad);

		glm::vec3 position(
			glm::cos(rad) * radius,
			yPos,
			glm::sin(rad) * radius
		);

		glm::vec3 color(
			Utility::RandomNumber<float>(0.5f, 0.8f),
			Utility::RandomNumber<float>(0.5f, 0.8f),
			Utility::RandomNumber<float>(0.7f, 1.0f)
		);

		lightAngles.push_back(rad);
		lightRadii.push_back(radius);
		lights.emplace_back(position, color, true, 0.1f);
	}

	lightSphereShader->Use();
	lightSphereShader->SetFloat("radius", 0.2f);
}

void AppDeferredSSAO::RenderLights() const
{
	lightSphereShader->Use();
	lightSphereShader->SetMat4("projection", camera->GetProjectionMatrix());
	lightSphereShader->SetMat4("view", camera->GetViewMatrix());
	for (const Light& light : lights)
	{
		light.Render(*lightSphereShader);
	}
}

void AppDeferredSSAO::UpdateLightPositions()
{
	for (unsigned int i = 0; i < lights.size(); ++i)
	{
		const float step = deltaTime * 0.2f;
		const float yPos = lights[i].Position.y;
		lightAngles[i] += step;
		lights[i].Position = glm::vec3(
			glm::cos(lightAngles[i]) * lightRadii[i],
			yPos,
			glm::sin(lightAngles[i]) * lightRadii[i]
		);
	}
}