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
		int prefilterCubeSize,
		int irradianceCubeSize,
		int _textureIndexGap);

	void SetCameraData(
		const glm::mat4& cameraProjectionMatrix, 
		const glm::mat4& cameraViewMatrix, 
		const glm::vec3& cameraPosition);
	void SetLights(const std::vector<Light>& lights);
	void BindTextures();

	Shader* GetPBRShader()
	{
		return pbrShader.get();
	}

	unsigned int GetEnvironmentCubemap() const
	{
		return envCubemap;
	}

	unsigned int GetIrradianceCubemap() const
	{
		return irradianceMap;
	}

	unsigned int GetPrefilterCubemap() const
	{
		return prefilterMap;
	}

private:
	void Init(
		const std::string& pbrShaderFile,
		const std::string& hdrFile,
		int environmentCubeSize,
		int prefilterCubeSize,
		int irradianceCubeSize
		);

private:
	int textureIndexGap;

	unsigned int irradianceMap;
	unsigned int prefilterMap;
	unsigned int brdfLUTTexture;
	unsigned int envCubemap;

	std::unique_ptr<Shader> pbrShader;
};

#endif
