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

/*struct ShadowInfo
{
	int specularPower;
	float ambientPower;

	float minBias;
	float maxBias;
	float shadowNearPlane;
	float shadowFarPlane;

	glm::vec3 lightPosition;
	glm::vec3 target;

	glm::vec3 cameraPosition;
	glm::mat4 cameraView;
	glm::mat4 cameraProjection;
};*/

class PipelineShadowMapping
{
public:
	PipelineShadowMapping(int depthWidth_, int depthHeight_);

	Shader* GetDepthShader() { return depthShader.get(); }
	//Shader* GetMainShader() { return mainShader.get(); }
	void BindDepthTexture(unsigned int index) { depthTexture->BindDSA(index); }
	glm::mat4 GetLightSpaceMatrix() { return lightSpaceMatrix; }

	//void SetInfo(ShadowInfo info);
	void StartRenderDepth(float nearPlane_, float farPlane_, glm::vec3 lightPosition, glm::vec3 target);
	void StopRenderDepth();
	
	void DebugDepth();

private:
	void Init();
	void InitQuad();
	//void SetMainShader();

private:
	// Data
	//ShadowInfo currInfo;
	glm::mat4 lightSpaceMatrix;

	//std::unique_ptr<Shader> mainShader;
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
