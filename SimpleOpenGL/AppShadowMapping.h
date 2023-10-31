#ifndef __APP_SHADOW_MAPPING_H_
#define __APP_SHADOW_MAPPING_H_

#include "AppBase.h"
#include "Shader.h"

class AppShadowMapping : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();

	void RenderScene(const Shader& shader);
	void RenderCube();
	void RenderQuad();

private:
	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	unsigned int planeVAO = 0;
	unsigned int planeVBO = 0;
};

#endif
