#include "AppIBL.h"
#include "AppSettings.h"
#include "Shader.h"
#include "Texture.h"
#include "Model.h"
#include "Shape.h"
#include "Light.h"

int AppIBL::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	constexpr float CUBE_SIZE = 1024;

	// Init
	// Set depth function to less than AND equal for skybox depth trick
	glDepthFunc(GL_LEQUAL);
	// Enable seamless cubemap sampling for lower mip levels in the pre-filter map.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	// Cube
	Cube cube;

	// Quad
	float quadRotation = acos(0);
	glm::vec3 quadRotationAxis(1.f, 0.f, 0.f);
	Quad quad(quadRotation, quadRotationAxis);

	Model renderModel(AppSettings::ModelFolder + "DamagedHelmet//DamagedHelmet.gltf");

	Shader pbrShader("IBL//pbr.vertex", "IBL//pbr.fragment");
	Shader equirectangularToCubemapShader("IBL//cubemap.vertex", "IBL//equirectangular_to_cubemap.fragment");
	Shader irradianceShader("IBL//cubemap.vertex", "IBL//irradiance_convolution.fragment");
	Shader prefilterShader("IBL//cubemap.vertex", "IBL//prefilter.fragment");
	Shader brdfShader("IBL//brdf.vertex", "IBL//brdf.fragment");
	Shader backgroundShader("IBL//background.vertex", "IBL//background.fragment");

	pbrShader.Use();
	pbrShader.SetInt("irradianceMap", 6);
	pbrShader.SetInt("prefilterMap", 7);
	pbrShader.SetInt("brdfLUT", 8);

	backgroundShader.Use();
	backgroundShader.SetInt("environmentMap", 0);

	Shader lightSphereShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	std::vector<Light> lights;
	lights.emplace_back(glm::vec3(-1.0f, 1.0f, 5.0f), glm::vec3(1.f));
	lights.emplace_back(glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(1.f));
	lights.emplace_back(glm::vec3(-1.0f, -1.0f, 5.0f), glm::vec3(1.f));
	lights.emplace_back(glm::vec3(1.0f, -1.0f, 5.0f), glm::vec3(1.f));

	// PBR setup framebuffer
	unsigned int captureFBO;
	unsigned int captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CUBE_SIZE, CUBE_SIZE);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// PBR load the HDR environment map
	Texture hdrTexture;
	hdrTexture.CreateFromHDRFile(AppSettings::TextureFolder + "hdr//the_sky_is_on_fire_4k.hdr");

	// PBR setup cubemap to render to and attach to framebuffer
	unsigned int envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, CUBE_SIZE, CUBE_SIZE, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// PBR set up projection and view matrices for capturing data onto the 6 cubemap face directions
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// PBR convert HDR equirectangular environment map to cubemap equivalent
	equirectangularToCubemapShader.Use();
	equirectangularToCubemapShader.SetInt("equirectangularMap", 0);
	equirectangularToCubemapShader.SetMat4("projection", captureProjection);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, hdrTexture);
	hdrTexture.Bind(GL_TEXTURE0);

	glViewport(0, 0, CUBE_SIZE, CUBE_SIZE); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularToCubemapShader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//renderCube();
		cube.Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// PBR create an irradiance cubemap, and re-scale capture FBO to irradiance scale
	unsigned int irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// PBR solve diffuse integral by convolution to create an irradiance (cube)map
	irradianceShader.Use();
	irradianceShader.SetInt("environmentMap", 0);
	irradianceShader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//renderCube();
		cube.Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// PBR create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale
	unsigned int prefilterMap;
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// PBR Run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map
	prefilterShader.Use();
	prefilterShader.SetInt("environmentMap", 0);
	prefilterShader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	constexpr unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// Resize framebuffer according to mip-level size.
		unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.SetFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader.SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//renderCube();
			cube.Draw();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// PBR Generate a 2D LUT from the BRDF equations used
	unsigned int brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);

	// Pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, CUBE_SIZE, CUBE_SIZE, 0, GL_RG, GL_FLOAT, 0);
	// Be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CUBE_SIZE, CUBE_SIZE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, CUBE_SIZE, CUBE_SIZE);
	brdfShader.Use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//renderQuad();
	quad.Draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Initialize static shader uniforms before rendering
	glm::mat4 projection = camera->GetProjectionMatrix();
	pbrShader.Use();
	pbrShader.SetMat4("projection", projection);
	backgroundShader.Use();
	backgroundShader.SetMat4("projection", projection);

	// Configure the viewport to the original framebuffer's screen dimensions
	glViewport(0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight);

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		pbrShader.Use();
		glm::mat4 view = camera->GetViewMatrix();
		pbrShader.SetMat4("view", view);
		pbrShader.SetVec3("camPos", camera->Position);

		// Bind pre-computed IBL data
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

		// Lights
		for (unsigned int i = 0; i < lights.size(); ++i)
		{
			pbrShader.SetVec3("lightPositions[" + std::to_string(i) + "]", lights[i].Position);
			pbrShader.SetVec3("lightColors[" + std::to_string(i) + "]", lights[i].Color);
		}

		// Render
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, static_cast<float>(acos(-1)), glm::vec3(0.0f, 1.0f, 0.0f));
		pbrShader.SetMat4("model", model);
		pbrShader.SetMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
		bool skipTextureBinding = false;
		renderModel.Draw(pbrShader, skipTextureBinding);

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
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
		//glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		//renderCube();
		cube.Draw();

		//brdfShader.Use();
		//renderQuad();
		
			
		SwapBuffers();
	}

	Terminate();

	return 0;
}