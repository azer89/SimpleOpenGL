#include "PipelineIBL.h"
#include "Texture.h"
#include "AppSettings.h"
#include "Utility.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

PipelineIBL::PipelineIBL(
	const std::string& pbrShaderFile,
	const std::string& hdrFile,
	int environmentCubeSize, 
	int specularCubeSize,
	int diffuseCubeSize,
	int _textureIndexGap) :
	textureIndexGap(_textureIndexGap)
{
	Init(pbrShaderFile, hdrFile, environmentCubeSize, specularCubeSize, diffuseCubeSize);
}

void PipelineIBL::Init(
	const std::string& pbrShaderFile,
	const std::string& hdrFile,
	int environmentCubeSize, // 1024
	int specularCubeSize, // 128
	int diffuseCubeSize // 32
)
{
	// Set depth function to less than AND equal for skybox depth trick
	glDepthFunc(GL_LEQUAL);
	// Enable seamless cubemap sampling for lower mip levels in the pre-filter map.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Cube
	Cube cube;

	// Quad
	const float quadRotation = acos(0);
	const glm::vec3 quadRotationAxis(1.f, 0.f, 0.f);
	const Quad quad(quadRotation, quadRotationAxis);

	pbrShader = std::make_unique<Shader>("IBL//pbr.vertex", pbrShaderFile.c_str());
	Shader equirectangularToCubemapShader("IBL//cubemap.vertex", "IBL//equirectangular_to_cubemap.fragment");
	Shader irradianceShader("IBL//cubemap.vertex", "IBL//irradiance_convolution.fragment");
	Shader prefilterShader("IBL//cubemap.vertex", "IBL//prefilter.fragment");
	Shader brdfShader("IBL//brdf.vertex", "IBL//brdf.fragment");

	pbrShader->Use();
	pbrShader->SetInt("irradianceMap", textureIndexGap);
	pbrShader->SetInt("prefilterMap", textureIndexGap + 1);
	pbrShader->SetInt("brdfLUT", textureIndexGap + 2);

	// PBR setup framebuffer
	unsigned int captureFBO;
	unsigned int captureRBO;

	glCreateFramebuffers(1, &captureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glCreateRenderbuffers(1, &captureRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glNamedRenderbufferStorage(captureRBO, GL_DEPTH_COMPONENT24, environmentCubeSize, environmentCubeSize);
	glNamedFramebufferRenderbuffer(captureRBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// PBR load the HDR environment map
	Texture hdrTexture;
	hdrTexture.CreateFromHDRFile(AppSettings::TextureFolder + hdrFile);

	// PBR set up projection and view matrices for capturing data onto the 6 cubemap face directions
	const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	std::vector<glm::mat4> captureViews =
	{
		glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// PBR setup cubemap to render to and attach to framebuffer
	const int envMipmapLevel = Utility::NumMipmap(environmentCubeSize, environmentCubeSize);
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &environmentCubemap);
	glTextureStorage2D(environmentCubemap, envMipmapLevel, GL_RGB16F, environmentCubeSize, environmentCubeSize);
	glTextureParameteri(environmentCubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(environmentCubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(environmentCubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(environmentCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(environmentCubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// PBR convert HDR equirectangular environment map to cubemap equivalent
	equirectangularToCubemapShader.Use();
	equirectangularToCubemapShader.SetInt("equirectangularMap", 0);
	equirectangularToCubemapShader.SetMat4("projection", captureProjection);
	hdrTexture.Bind(0);

	glViewport(0, 0, environmentCubeSize, environmentCubeSize); // Don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < captureViews.size(); ++i)
	{
		equirectangularToCubemapShader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cube.Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
	glGenerateTextureMipmap(environmentCubemap);

	// PBR create an irradiance cubemap, and re-scale capture FBO to irradiance scale
	const int irradianceMipmapLevel = Utility::NumMipmap(diffuseCubeSize, diffuseCubeSize); // Is mipmap needed?
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &diffuseCubemap);
	glTextureStorage2D(diffuseCubemap, irradianceMipmapLevel, GL_RGB16F, diffuseCubeSize, diffuseCubeSize);
	glTextureParameteri(diffuseCubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(diffuseCubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(diffuseCubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(diffuseCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(diffuseCubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, diffuseCubeSize, diffuseCubeSize);

	// PBR solve diffuse integral by convolution to create an irradiance (cube)map
	irradianceShader.Use();
	irradianceShader.SetInt("environmentMap", 0);
	irradianceShader.SetMat4("projection", captureProjection);
	glBindTextureUnit(0, environmentCubemap);

	glViewport(0, 0, diffuseCubeSize, diffuseCubeSize); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < captureViews.size(); ++i)
	{
		irradianceShader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, diffuseCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cube.Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// PBR create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale
	const int prefilterMipmapLevel = Utility::NumMipmap(specularCubeSize, specularCubeSize); 
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &specularCubemap);
	glTextureStorage2D(specularCubemap, prefilterMipmapLevel, GL_RGB16F, specularCubeSize, specularCubeSize);
	glTextureParameteri(specularCubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(specularCubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(specularCubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(specularCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Be sure to set minification filter to mip_linear 
	glTextureParameteri(specularCubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateTextureMipmap(specularCubemap);

	// PBR Run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map
	prefilterShader.Use();
	prefilterShader.SetInt("environmentMap", 0);
	prefilterShader.SetMat4("projection", captureProjection);
	glBindTextureUnit(0, environmentCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	constexpr unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// Resize framebuffer according to mip-level size.
		unsigned int mipWidth = static_cast<unsigned int>(specularCubeSize * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(specularCubeSize * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		const float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.SetFloat("roughness", roughness);
		for (unsigned int i = 0; i < captureViews.size(); ++i)
		{
			prefilterShader.SetMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, specularCubemap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cube.Draw();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// PBR Generate a 2D LUT from the BRDF equations
	glCreateTextures(GL_TEXTURE_2D, 1, &brdfLUTImage);
	glTextureStorage2D(brdfLUTImage, 1, GL_RGB16F, environmentCubeSize, environmentCubeSize);
	glTextureParameteri(brdfLUTImage, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(brdfLUTImage, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(brdfLUTImage, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(brdfLUTImage, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// Then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, environmentCubeSize, environmentCubeSize);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTImage, 0);

	glViewport(0, 0, environmentCubeSize, environmentCubeSize);
	brdfShader.Use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	quad.Draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineIBL::SetCameraData(const glm::mat4& cameraProjectionMatrix, const glm::mat4& cameraViewMatrix, const glm::vec3& cameraPosition)
{
	pbrShader->Use();
	pbrShader->SetMat4("projection", cameraProjectionMatrix);
	pbrShader->SetMat4("view", cameraViewMatrix);
	pbrShader->SetVec3("camPos", cameraPosition);
}

void PipelineIBL::SetLights(const std::vector<Light>& lights)
{
	pbrShader->Use();
	for (unsigned int i = 0; i < lights.size(); ++i)
	{
		pbrShader->SetVec3("lightPositions[" + std::to_string(i) + "]", lights[i].Position);
		pbrShader->SetVec3("lightColors[" + std::to_string(i) + "]", lights[i].Color);
	}
}

void PipelineIBL::BindTextures()
{
	// Bind pre-computed IBL data
	glBindTextureUnit(textureIndexGap, diffuseCubemap);
	glBindTextureUnit(textureIndexGap + 1, specularCubemap);
	glBindTextureUnit(textureIndexGap + 2, brdfLUTImage);
}