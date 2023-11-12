#ifndef __SKYBOX_WITH_MODEL_H__
#define __SKYBOX_WITH_MODEL_H__

#include "AppBase.h"

class AppSkyboxWithModel : AppBase
{
public:
	int MainLoop() override;

private:
	void InitCube();
	void InitSkybox();

	unsigned int skyboxVAO;
	unsigned int skyboxVBO;
};

#endif
