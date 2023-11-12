#ifndef __SKYBOX_WITH_CUBE_H__
#define __SKYBOX_WITH_CUBE_H__

#include "AppBase.h"

class AppSkyboxWithCube : AppBase
{
public:
	int MainLoop() override;

private:
	void InitCube();
	void InitSkybox();

	unsigned int cubeVAO;
	unsigned int cubeVBO;
	unsigned int skyboxVAO;
	unsigned int skyboxVBO;
};

#endif
