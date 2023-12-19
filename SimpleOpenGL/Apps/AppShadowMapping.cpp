#include "AppShadowMapping.h"
#include "PipelineShadowMapping.h"
#include "Light.h"
#include "Texture.h"
#include "AppSettings.h"
#include "Shape.h"

int AppShadowMapping::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	InitScene();

	constexpr int depthWidth = 2000;
	constexpr int depthHeight = 2000;
	PipelineShadowMapping pipeline(depthWidth, depthHeight);

	Shader lightShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");

	Shader mainShader("ShadowMapping//shadow_mapping.vertex", "ShadowMapping//shadow_mapping.fragment");
	mainShader.Use();
	mainShader.SetInt("shadowMap", 1);

	// Textures
	Texture planeTexture;
	planeTexture.CreateFromImageFile(AppSettings::TextureFolder + "White1x1.png");

	// Light
	constexpr float lightY = 6.0f;
	constexpr float lightRadius = 5.0f;
	constexpr float lightSpeed = 0.5f;
	float lightTimer = 0.0f;
	bool moveLight = true;
	Light light(glm::vec3(0.f), glm::vec3(1.f));
	glm::vec3 target(0.f);
	
	// Shadow parameters
	float minBias = 0.005f;
	float maxBias = 0.05f;
	float shadowNearPlane = 1.0f;
	float shadowFarPlane = 20;

	// Light parameter
	int specularPower = 32;
	float ambientPower = 0.5;

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		// Calculate light position
		if (moveLight)
		{
			light.Position = glm::vec3(
				glm::sin(lightTimer) * lightRadius,
				lightY,
				glm::cos(lightTimer) * lightRadius);
			lightTimer += deltaTime * lightSpeed;
		}

		// Generate depth
		pipeline.StartRenderDepth(
			shadowNearPlane,
			shadowFarPlane,
			light.Position,
			target
		);
		Shader* depthShader = pipeline.GetDepthShader();
		RenderScene(*depthShader);
		pipeline.StopRenderDepth();

		// Render Scene
		mainShader.Use();
		mainShader.SetFloat("minBias", minBias);
		mainShader.SetFloat("maxBias", maxBias);
		mainShader.SetFloat("ambientPower", ambientPower);
		mainShader.SetInt("specularPower", specularPower);
		mainShader.SetMat4("projection", camera->GetProjectionMatrix());
		mainShader.SetMat4("view", camera->GetViewMatrix());
		mainShader.SetVec3("viewPos", camera->Position);
		mainShader.SetVec3("lightPos", light.Position);
		mainShader.SetMat4("lightSpaceMatrix", pipeline.GetLightSpaceMatrix());
		
		planeTexture.Bind(0);
		pipeline.BindDepthTexture(1);

		RenderScene(mainShader);

		// Debug light
		lightShader.Use();
		lightShader.SetMat4("projection", camera->GetProjectionMatrix());
		lightShader.SetMat4("view", camera->GetViewMatrix());
		light.Render(lightShader);

		// Debug depth
		//pipeline.DebugDepth();

		if (showImgui)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::SetNextWindowSize(ImVec2(500, 200));

			ImGui::Begin("Shadow Mapping");

			ImGui::SliderFloat("Min Bias", &minBias, 0.00001f, 0.01f);
			ImGui::SliderFloat("Max Bias", &maxBias, 0.001f, 0.1f);
			ImGui::SliderFloat("Shadow Near Plane", &shadowNearPlane, 0.1f, 5.0f);
			ImGui::SliderFloat("Shadow Far Plane", &shadowFarPlane, 10.0f, 500.0f);

			ImGui::Spacing();
			ImGui::Checkbox("Move Light", &moveLight);
			ImGui::SliderFloat("Ambient", &ambientPower, 0.01f, 1.0f);
			ImGui::SliderInt("Specular", &specularPower, 2, 128);

			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppShadowMapping::RenderScene(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0f));
	shader.SetMat4("model", model);
	renderModel->Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.0f));
	shader.SetMat4("model", model);
	renderModel->Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, 0.0f, 0.0f));
	shader.SetMat4("model", model);
	renderModel->Draw(shader);
}

void AppShadowMapping::InitScene()
{
	// Model
	renderModel = std::make_unique<Model>(AppSettings::ModelFolder + "Zaku//scene.gltf");

	// Plane
	constexpr float planeVertices[] = {
		// Positions			// Normals			// Texcoords
		 25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
}
