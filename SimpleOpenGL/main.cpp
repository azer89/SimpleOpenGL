#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"

int main()
{
	AppBlinnPhong app;
	auto returnValue = app.MainLoop();
	return returnValue;
}