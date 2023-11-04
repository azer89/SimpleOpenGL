#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"
#include "AppModelLoading.h"

int main()
{
	AppModelLoading app;
	auto returnValue = app.MainLoop();
	return returnValue;
}