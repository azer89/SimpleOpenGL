#include "AppIBL.h"
#include "PipelineIBL.h"
#include "AppSettings.h"
#include "Shader.h"
#include "Texture.h"
#include "Model.h"
#include "Shape.h"
#include "Light.h"

int AppIBL::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	PipelineIBL ibl
	(
		"IBL//pbr_sketchfab.fragment",
		"hdr//neon_photostudio_4k.hdr",
		1024,
		128,
		32,
		6
	);

	InitDebugCubes();

	Cube cube;

	Model renderModel(AppSettings::ModelFolder + "Tachikoma//Tachikoma.gltf");

	Shader backgroundShader("IBL//background.vertex", "IBL//background.fragment");
	backgroundShader.Use();
	backgroundShader.SetInt("environmentMap", 0);
	backgroundShader.SetMat4("projection", camera->GetProjectionMatrix());

	Shader lightSphereShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	std::vector<Light> lights;
	lights.emplace_back(glm::vec3(-1.5f, 0.7f, 1.5f ), glm::vec3(1.f, 1.f, 1.f));
	lights.emplace_back(glm::vec3(1.5f, 0.7f, 1.5f), glm::vec3(1.f, 1.f, 1.f));
	lights.emplace_back(glm::vec3(-1.5f, 0.7f, -1.5f), glm::vec3(1.f, 1.f, 1.f));
	lights.emplace_back(glm::vec3(1.5f, 0.7f, -1.5f), glm::vec3(1.f, 1.f, 1.f));

	// Configure the viewport to the original framebuffer's screen dimensions
	glViewport(0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight);

	float rotation = 0;

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		glm::mat4 projection = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();

		ibl.SetCameraData(projection, view, camera->Position);
		ibl.BindTextures();
		ibl.SetLights(lights);

		Shader* pbrShader = ibl.GetPBRShader();

		// Render
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
		//rotation += deltaTime * 0.2f;
		pbrShader->SetMat4("model", model);
		pbrShader->SetMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
		bool skipTextureBinding = false;
		renderModel.Draw(*pbrShader, skipTextureBinding);

		lightSphereShader.Use();
		lightSphereShader.SetMat4("projection", camera->GetProjectionMatrix());
		lightSphereShader.SetMat4("view", camera->GetViewMatrix());
		for (auto& l : lights)
		{
			l.Render(lightSphereShader);
		}

		backgroundShader.Use();
		backgroundShader.SetMat4("view", camera->GetViewMatrix());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, ibl.GetEnvironmentCubemap());
		cube.Draw();

		// Imgui
		/*ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowSize(ImVec2(500, 100));

		ImGui::Begin("IBL");
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());*/

		// Debugging
		RenderDebugCubes(cube, ibl);
			
		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppIBL::InitDebugCubes()
{
	simpleCubeShader = std::make_unique<Shader>("IBL//simple_cube.vertex", "IBL//simple_cube.fragment");
	simpleCubeShader->Use();
	simpleCubeShader->SetInt("skybox", 0);
}

void AppIBL::RenderDebugCubes(const Cube& cube, const PipelineIBL& ibl)
{
	simpleCubeShader->Use();
	simpleCubeShader->SetMat4("projection", camera->GetProjectionMatrix());
	simpleCubeShader->SetMat4("view", camera->GetViewMatrix());

	glm::mat4 model(1.0f);
	model = glm::scale(model, glm::vec3(0.2f));
	model = glm::translate(model, glm::vec3(-4.f, -8.f, 0.f));
	simpleCubeShader->SetMat4("model", model);
	glBindTextureUnit(0, ibl.GetEnvironmentCubemap());
	cube.Draw();

	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.2f));
	model = glm::translate(model, glm::vec3(0.f, -7.f, 0.f));
	simpleCubeShader->SetMat4("model", model);
	glBindTextureUnit(0, ibl.GetIrradianceCubemap());
	cube.Draw();

	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.2f));
	model = glm::translate(model, glm::vec3(4.f, -6.f, 0.f));
	simpleCubeShader->SetMat4("model", model);
	glBindTextureUnit(0, ibl.GetPrefilterCubemap());
	cube.Draw();
}