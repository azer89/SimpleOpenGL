#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"
#include "AppModelLoading.h"
#include "AppSkyboxWithModel.h"

int main()
{
	AppBlinnPhong app;
	auto returnValue = app.MainLoop();
	return returnValue;
}