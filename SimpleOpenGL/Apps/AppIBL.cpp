#include "AppIBL.h"
#include "PipelineIBL.h"
#include "AppSettings.h"
#include "Shader.h"
#include "Texture.h"
#include "Model.h"
#include "Shape.h"
#include "Light.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int AppIBL::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	PipelineIBL ibl
	(
		AppSettings::TextureFolder + "hdr//the_sky_is_on_fire_4k.hdr",
		1024,
		6
	);

	Cube cube;

	Model renderModel(AppSettings::ModelFolder + "DamagedHelmet//DamagedHelmet.gltf");

	glm::mat4 projection = camera->GetProjectionMatrix();

	Shader backgroundShader("IBL//background.vertex", "IBL//background.fragment");
	backgroundShader.Use();
	backgroundShader.SetInt("environmentMap", 0);
	backgroundShader.SetMat4("projection", projection);

	Shader lightSphereShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	std::vector<Light> lights;
	lights.emplace_back(glm::vec3(-1.0f, 1.0f, 5.0f), glm::vec3(1.f));
	lights.emplace_back(glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(1.f));
	lights.emplace_back(glm::vec3(-1.0f, -1.0f, 5.0f), glm::vec3(1.f));
	lights.emplace_back(glm::vec3(1.0f, -1.0f, 5.0f), glm::vec3(1.f));

	// Configure the viewport to the original framebuffer's screen dimensions
	glViewport(0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight);

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		glm::mat4 view = camera->GetViewMatrix();

		ibl.SetCameraData(projection, view, camera->Position);
		ibl.BindTextures();
		ibl.SetLights(lights);

		Shader* pbrShader = ibl.GetPBRShader();

		// Render
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, static_cast<float>(acos(-1)), glm::vec3(0.0f, 1.0f, 0.0f));
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
		//brdfShader.Use();
		//renderQuad();

		// Imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowSize(ImVec2(500, 100));

		ImGui::Begin("IBL");
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			
		SwapBuffers();
	}

	Terminate();

	return 0;
}