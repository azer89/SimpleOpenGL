#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"
#include "AppModelLoading.h"
#include "AppSkyboxWithModel.h"
#include "AppSkyboxWithCube.h"
#include "AppDeferred.h"

int main()
{
	AppDeferred app;
	auto returnValue = app.MainLoop();
	return returnValue;
}