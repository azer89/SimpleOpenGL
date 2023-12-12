#ifndef __PIPELINE_SHADOW_MAPPING_H__
#define __PIPELINE_SHADOW_MAPPING_H__

#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"

#include <memory>

/*glm::vec3 lightPos;
constexpr float lightY = 6.0f;
constexpr float lightRadius = 5.0f;
constexpr float lightSpeed = 0.5f;
float lightTimer = 0.0f;*/

/*
// Render Scene
		mainShader.Use();
		mainShader.SetFloat("minBias", minBias);
		mainShader.SetFloat("maxBias", maxBias);
		mainShader.SetFloat("ambientPower", ambientPower); //
		mainShader.SetInt("specularPower", specularPower); //
		mainShader.SetMat4("projection", camera->GetProjectionMatrix());
		mainShader.SetMat4("view", camera->GetViewMatrix());
		mainShader.SetVec3("viewPos", camera->Position);
		mainShader.SetVec3("lightPos", lightPos);
		mainShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

*/

struct ShadowInfo
{
	glm::vec3 lightPosition;
	glm::vec3 target;

	float minBias;
	float maxBias;
	float shadowNearPlane;
	float shadowFarPlane;

	glm::mat4 cameraProjection;
	glm::mat4 cameraView;
	glm::vec3 cameraPosition;

	float ambientPower;
	int specularPower;
};

class PipelineShadowMapping
{
public:
	PipelineShadowMapping();

	Shader* GetMainShader() { return mainShader.get(); }

private:
	void Init();

	void SetInfo(ShadowInfo info);
	void StartRenderDepth();
	void StopRenderDepth();

	void SetMainShader();

private:
	// Data
	ShadowInfo currInfo;
	glm::mat4 lightSpaceMatrix;

	std::unique_ptr<Shader> mainShader;
	std::unique_ptr<Shader> depthShader;
	std::unique_ptr<Shader> debugShader;

	std::unique_ptr<Texture> depthTexture;
	unsigned int depthFBO;
};

#endif
