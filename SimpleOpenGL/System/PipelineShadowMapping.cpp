#include "PipelineShadowMapping.h"
#include "AppSettings.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

PipelineShadowMapping::PipelineShadowMapping(int depthWidth_, int depthHeight_) :
	depthWidth(depthWidth_),
	depthHeight(depthHeight_)
{
	Init();
	InitQuad();
}

void PipelineShadowMapping::Init()
{
	//mainShader = std::make_unique<Shader>("ShadowMapping//shadow_mapping.vertex", "ShadowMapping//shadow_mapping.fragment");
	depthShader = std::make_unique<Shader>("ShadowMapping//shadow_mapping_depth.vertex", "ShadowMapping//shadow_mapping_depth.fragment");
	debugShader = std::make_unique<Shader>("ShadowMapping//shadow_mapping_debug.vertex", "ShadowMapping//shadow_mapping_debug.fragment");

	//mainShader->Use();
	//mainShader->SetInt("diffuseTexture", 0);
	//mainShader->SetInt("shadowMap", 1);
	debugShader->Use();
	debugShader->SetInt("depthMap", 0);

	// Depth
	depthTexture = std::make_unique<Texture>();
	depthTexture->CreateDepthMap(depthWidth, depthHeight);

	// FBO
	glCreateFramebuffers(1, &depthFBO);
	glNamedFramebufferTexture(depthFBO, GL_DEPTH_ATTACHMENT, depthTexture->GetID(), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void PipelineShadowMapping::SetInfo(ShadowInfo info)
//{
	//currInfo = info;

	/*glm::mat4 lightProjection =
		glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, currInfo.shadowNearPlane, currInfo.shadowFarPlane);
	glm::mat4 lightView =
		glm::lookAt(currInfo.lightPosition, currInfo.target, glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;*/

	// Render Scene
	/*mainShader->Use();
	mainShader->SetFloat("minBias", currInfo.minBias);
	mainShader->SetFloat("maxBias", currInfo.maxBias);
	mainShader->SetFloat("ambientPower", currInfo.ambientPower);
	mainShader->SetInt("specularPower", currInfo.specularPower);
	mainShader->SetMat4("projection", currInfo.cameraProjection);
	mainShader->SetMat4("view", currInfo.cameraView);
	mainShader->SetVec3("viewPos", currInfo.cameraPosition);
	mainShader->SetVec3("lightPos", currInfo.lightPosition);
	mainShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);*/
//}

void PipelineShadowMapping::StartRenderDepth(float nearPlane_, float farPlane_, glm::vec3 lightPosition, glm::vec3 target)
{
	nearPlane = nearPlane_;
	farPlane = farPlane_;

	glm::mat4 lightProjection =
		glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane,farPlane);
	glm::mat4 lightView =
		glm::lookAt(lightPosition, target, glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;

	// Render depth
	depthShader->Use();
	depthShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, depthWidth, depthHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void PipelineShadowMapping::StopRenderDepth()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PipelineShadowMapping::InitQuad()
{
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

void PipelineShadowMapping::DebugDepth()
{
	// Debug depth map
	debugShader->Use();
	debugShader->SetFloat("near_plane", nearPlane);
	debugShader->SetFloat("far_plane", farPlane);
	depthTexture->BindDSA(0);
	
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

/*
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
	glm::mat4 cameraPosition;

	float ambientPower;
	int specularPower;
};
*/

//void PipelineShadowMapping::SetMainShader()
//{
//}
