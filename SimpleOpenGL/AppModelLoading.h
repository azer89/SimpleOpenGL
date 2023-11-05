#ifndef __APP_MODEL_LOADING_H__
#define __APP_MODEL_LOADING_H__

#include "AppBase.h"

class AppModelLoading : AppBase
{
public:
	int MainLoop() override;

private:
	void InitCube();

	unsigned int cubeVAO;
	unsigned int cubeVBO;
};

#endif