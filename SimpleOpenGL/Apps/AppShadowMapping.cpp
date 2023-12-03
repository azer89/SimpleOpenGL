#include "AppShadowMapping.h"
#include "Texture.h"
#include "AppSettings.h"
#include "Shape.h"

int AppShadowMapping::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	InitScene();

	Shader mainShader("ShadowMapping//shadow_mapping.vertex", "ShadowMapping//shadow_mapping.fragment");
	Shader depthShader("ShadowMapping//shadow_mapping_depth.vertex", "ShadowMapping//shadow_mapping_depth.fragment");
	Shader debugShader("ShadowMapping//shadow_mapping_debug.vertex", "ShadowMapping//shadow_mapping_debug.fragment");
	Shader lightCubeShader("Misc//light_cube.vertex", "Misc//light_cube.fragment");

	mainShader.Use();
	mainShader.SetInt("diffuseTexture", 0);
	mainShader.SetInt("shadowMap", 1);
	debugShader.Use();
	debugShader.SetInt("depthMap", 0);

	// Textures
	Texture planeTexture;
	planeTexture.CreateFromImageFile(AppSettings::TextureFolder + "White1x1.png");

	// Depth
	constexpr unsigned int DEPTH_WIDTH = 2000;
	constexpr unsigned int DEPTH_HEIGHT = 2000;
	Texture depthTexture;
	depthTexture.CreateDepthMap(DEPTH_WIDTH, DEPTH_HEIGHT);

	// Light debugging
	Cube cube;

	// FBO
	unsigned int depthFBO;
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.GetID(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Light
	glm::vec3 lightPos;
	constexpr float lightY = 6.0f;
	constexpr float lightRadius = 5.0f;
	constexpr float lightSpeed = 0.5f;
	float lightTimer = 0.0f;

	// For depth rendering
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::mat4 lightSpaceMatrix;
	
	// Parameters
	float minBias = 0.005;
	float maxBias = 0.05;
	float shadowNearPlane = 1.0f;
	float shadowFarPlane = 20;
	bool moveLight = true;

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.02f));

		// Calculate light position
		if (moveLight)
		{
			lightPos = glm::vec3(
				glm::sin(lightTimer) * lightRadius,
				lightY,
				glm::cos(lightTimer) * lightRadius);
			lightTimer += deltaTime * lightSpeed;
		}

		// Render depth
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, shadowNearPlane, shadowFarPlane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		depthShader.Use();
		depthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, DEPTH_WIDTH, DEPTH_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderPlane(depthShader);
		RenderFoxes(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Reset
		glViewport(0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render Scene
		mainShader.Use();
		mainShader.SetFloat("minBias", minBias);
		mainShader.SetFloat("maxBias", maxBias);
		mainShader.SetMat4("projection", camera->GetProjectionMatrix());
		mainShader.SetMat4("view", camera->GetViewMatrix());
		mainShader.SetVec3("viewPos", camera->Position);
		mainShader.SetVec3("lightPos", lightPos);
		mainShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		
		planeTexture.Bind(GL_TEXTURE0);
		depthTexture.Bind(GL_TEXTURE1);
		RenderPlane(mainShader);
		
		mainShader.SetMat4("model", model);
		depthTexture.Bind(GL_TEXTURE1);
		RenderFoxes(mainShader);

		// Debug light
		lightCubeShader.Use();
		lightCubeShader.SetMat4("projection", camera->GetProjectionMatrix());
		lightCubeShader.SetMat4("view", camera->GetViewMatrix());
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightCubeShader.SetMat4("model", model);
		cube.Draw();

		// Debug depth map
		/*debugShader.Use();
		debugShader.SetFloat("near_plane", near_plane);
		debugShader.SetFloat("far_plane", far_plane);
		depthTexture.Bind(GL_TEXTURE0);
		RenderQuad();*/

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowSize(ImVec2(500, 200));

		ImGui::Begin("Shadow Mapping");

		ImGui::SliderFloat("Min Bias", &minBias, 0.0001f, 0.01f);
		ImGui::SliderFloat("Max Bias", &maxBias, 0.001f, 0.1f);
		ImGui::SliderFloat("Shadow Near Plane", &shadowNearPlane, 0.1f, 5.0f);
		ImGui::SliderFloat("Shadow Far Plane", &shadowFarPlane, 10.0f, 500.0f);
		ImGui::Checkbox("Move Light", &moveLight);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppShadowMapping::RenderPlane(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AppShadowMapping::RenderFoxes(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
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

void AppShadowMapping::RenderQuad()
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void AppShadowMapping::InitScene()
{
	// Fox
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

	// Quad (for debugging)
	constexpr float quadVertices[] = {
		// positions		// texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
}
