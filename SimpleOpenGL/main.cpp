#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"
#include "AppModelLoading.h"
#include "AppSkybox.h"

int main()
{
	AppSkybox app;
	auto returnValue = app.MainLoop();
	return returnValue;
}