#include "AppDeferred.h"
#include "AppSettings.h"
#include "ShapeFactory.h"

#include <memory>

int AppDeferred::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader gBufferShader("deferred_g_buffer.vertex", "deferred_g_buffer.fragment");
	Shader lightingShader("deferred_lighting.vertex", "deferred_lighting.fragment");
	lightingShader.Use();
	lightingShader.SetInt("gPosition", 0);
	lightingShader.SetInt("gNormal", 1);
	lightingShader.SetInt("gAlbedoSpec", 2);

	Shader lightSphereShader("light_sphere.vertex", "light_sphere.fragment");

	Texture grassTexture;
	grassTexture.CreateFromImageFile(AppSettings::TextureFolder + "grass.png");

	// G buffer
	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedoSpec;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	
	// Position
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	
	// Normal
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	
	// Color + Specular
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
	
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// Render buffer
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	
	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!" << '\n';
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	InitLights();
	InitScene();

	lightSphereShader.Use();
	lightSphereShader.SetFloat("radius", 0.2f);

	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1 Geometry pass
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		gBufferShader.Use();
		gBufferShader.SetMat4("projection", projection);
		gBufferShader.SetMat4("view", view);
		grassTexture.Bind(GL_TEXTURE0);
		RenderPlane(gBufferShader);
		RenderFoxes(gBufferShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2 Lighting Pass
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightingShader.Use();
		lightingShader.SetVec3("viewPos", camera->Position);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		// Send light relevant uniforms
		const float linear = 0.7f;
		const float quadratic = 1.8f;
		for (unsigned int i = 0; i < lights.size(); i++)
		{
			lightingShader.SetVec3("lights[" + std::to_string(i) + "].Position", lights[i].Position);
			lightingShader.SetVec3("lights[" + std::to_string(i) + "].Color", lights[i].Color);
			// Update attenuation parameters and calculate radius
			lightingShader.SetFloat("lights[" + std::to_string(i) + "].Linear", linear);
			lightingShader.SetFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
		}
		// Render quad
		RenderQuad();

		// 3 Copy content of geometry's depth buffer to default framebuffer's depth buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		// Blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
		// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
		// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
		glBlitFramebuffer(0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Render lights on top of scene
		lightSphereShader.Use();
		lightSphereShader.SetMat4("projection", projection);
		lightSphereShader.SetMat4("view", view);
		for (unsigned int i = 0; i < lights.size(); i++)
		{
			lights[i].Render(lightSphereShader);
		}

		SwapBuffers();
		PollEvents();
	}

	Terminate();

	return 0;
}

void AppDeferred::InitLights()
{
	const unsigned int NR_LIGHTS = 64;
	srand(time(NULL));
	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		glm::vec3 position(
			static_cast<float>(((rand() % 100) / 100.0) * 12.0 - 6.0),
			static_cast<float>(((rand() % 100) / 100.0) * 1.0 + 0.15),
			static_cast<float>(((rand() % 100) / 100.0) * 12.0 - 6.0)
		);
		glm::vec3 color(
			static_cast<float>(((rand() % 100) / 200.0f) + 0.5), // Between 0.5 and 1.0
			static_cast<float>(((rand() % 100) / 200.0f) + 0.5), // Between 0.5 and 1.0
			static_cast<float>(((rand() % 100) / 200.0f) + 0.5) // Between 0.5 and 1.0
		);
		lights.emplace_back(position, color, true, 0.1f);
	}
}

void AppDeferred::InitScene()
{
	// Fox
	foxModel = std::make_unique<Model>(AppSettings::ModelFolder + "Fox//Fox.gltf");

	float halfWidth = 50.0f;

	// Plane
	float planeVertices[] = {
		// Positions				   // Normals			// Texcoords
		 halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
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

void AppDeferred::RenderPlane(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AppDeferred::RenderFoxes(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.025f));
	shader.SetMat4("model", model);
	foxModel->Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.0f));
	model = glm::scale(model, glm::vec3(0.02f));
	shader.SetMat4("model", model);
	foxModel->Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.03f));
	shader.SetMat4("model", model);
	foxModel->Draw(shader);
}

void AppDeferred::RenderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// Positions		// Texture Coords
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
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}