#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "AppBase.h"

class AppSkybox : AppBase
{
public:
	int MainLoop() override;

private:
	unsigned int cubeVAO;
	unsigned int cubeVBO;
	unsigned int skyboxVAO;
	unsigned int skyboxVBO;
};

#endif
