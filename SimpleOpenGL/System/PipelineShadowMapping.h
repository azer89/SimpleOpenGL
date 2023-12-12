#ifndef __PIPELINE_SHADOW_MAPPING_H__
#define __PIPELINE_SHADOW_MAPPING_H__

#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"

#include <memory>

class PipelineShadowMapping
{
public:
	PipelineShadowMapping(int depthWidth_, int depthHeight_);

	Shader* GetDepthShader() { return depthShader.get(); }
	void BindDepthTexture(unsigned int index) { depthTexture->BindDSA(index); }
	glm::mat4 GetLightSpaceMatrix() { return lightSpaceMatrix; }

	void StartRenderDepth(float nearPlane_, float farPlane_, glm::vec3 lightPosition, glm::vec3 target);
	void StopRenderDepth();
	
	void DebugDepth();

private:
	void Init();
	void InitQuad();

private:
	// Data
	glm::mat4 lightSpaceMatrix;

	std::unique_ptr<Shader> depthShader;
	std::unique_ptr<Shader> debugShader;

	int depthWidth;
	int depthHeight;
	std::unique_ptr<Texture> depthTexture;
	unsigned int depthFBO;

	float nearPlane;
	float farPlane;

	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
};

#endif
