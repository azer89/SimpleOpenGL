#ifndef PIPELINE_IBL_H
#define PIPELINE_IBL_H

#include "Shader.h"
#include "Light.h"

#include "glm/glm.hpp"

#include <memory>
#include <string>
#include <vector>

class PipelineIBL
{
public:
	PipelineIBL(
		const std::string& pbrShaderFile,
		const std::string& hdrFile,
		int environmentCubeSize, 
		int specularCubeSize,
		int diffuseCubeSize,
		int _textureIndexGap);

	void SetCameraData(
		const glm::mat4& cameraProjectionMatrix, 
		const glm::mat4& cameraViewMatrix, 
		const glm::vec3& cameraPosition);
	void SetLights(const std::vector<Light>& lights);
	void BindTextures();

	[[nodiscard]] Shader* GetPBRShader() { return pbrShader.get(); }

	[[nodiscard]] unsigned int GetEnvironmentCubemap() const { return environmentCubemap; }

	// Diffuse / irradiance
	[[nodiscard]] unsigned int GetDiffuseCubemap() const { return diffuseCubemap; }

	// Specular / prefilter
	[[nodiscard]] unsigned int GetSpecularCubemap() const { return specularCubemap; }

private:
	void Init(
		const std::string& pbrShaderFile,
		const std::string& hdrFile,
		int environmentCubeSize,
		int specularCubeSize,
		int diffuseCubeSize
		);

private:
	int textureIndexGap{};

	uint32_t diffuseCubemap{};
	uint32_t specularCubemap{};
	uint32_t brdfLUTImage{};
	uint32_t environmentCubemap{};

	std::unique_ptr<Shader> pbrShader{};
};

#endif
