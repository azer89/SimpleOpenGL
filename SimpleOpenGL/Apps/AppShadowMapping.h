#ifndef __APP_SHADOW_MAPPING_H_
#define __APP_SHADOW_MAPPING_H_

#include "AppBase.h"
#include "Shader.h"
#include "Model.h"

class AppShadowMapping : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();

	void RenderPlane(const Shader& shader);
	void RenderFoxes(const Shader& shader);
	void RenderQuad();

private:
	std::unique_ptr<Model> foxModel;

	unsigned int lightCubeVAO = 0;
	unsigned int lightCubeVBO = 0;

	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;

	unsigned int planeVAO = 0;
	unsigned int planeVBO = 0;
};

#endif
