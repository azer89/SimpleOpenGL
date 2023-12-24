#ifndef APP_SHADOW_MAPPING
#define APP_SHADOW_MAPPING

#include "AppBase.h"
#include "Shader.h"
#include "Model.h"

class AppShadowMapping final : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void RenderScene(const Shader& shader);

private:
	std::unique_ptr<Model> renderModel;

	unsigned int planeVAO = 0;
	unsigned int planeVBO = 0;
};

#endif
