#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"
#include "AppModelLoading.h"
#include "AppSkyboxWithModel.h"
#include "AppSkyboxWithCube.h"

int main()
{
	AppShadowMapping app;
	auto returnValue = app.MainLoop();
	return returnValue;
}