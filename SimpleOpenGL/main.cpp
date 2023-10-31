#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"

int main()
{
	AppShadowMapping app;
	auto returnValue = app.MainLoop();
	return returnValue;
}