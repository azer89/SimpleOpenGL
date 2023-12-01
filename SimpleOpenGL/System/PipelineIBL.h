#ifndef __PIPELINE_IBL_H__
#define __PIPELINE_IBL_H__

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
		std::string pbrShaderFile,
		std::string hdrFile, 
		float cubeSize, 
		int _textureIndexGap);

	void SetCameraData(const glm::mat4& cameraProjectionMatrix, const glm::mat4& cameraViewMatrix, const glm::vec3& cameraPosition);
	void SetLights(const std::vector<Light>& lights);
	void BindTextures();

	Shader* GetPBRShader()
	{
		return pbrShader.get();
	}

	unsigned int GetEnvironmentCubemap()
	{
		return envCubemap;
	}

private:
	void Init(std::string pbrShaderFile, std::string hdrFile, float cubeSize);

private:
	int textureIndexGap;

	unsigned int irradianceMap;
	unsigned int prefilterMap;
	unsigned int brdfLUTTexture;
	unsigned int envCubemap;

	std::unique_ptr<Shader> pbrShader;
};

#endif
