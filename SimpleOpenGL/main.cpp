#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"
#include "AppModelLoading.h"
#include "AppSkyboxWithModel.h"
#include "AppSkyboxWithCube.h"
#include "AppDeferred.h"
#include "AppDeferredSponza.h"

int main()
{
	AppDeferredSponza app;
	auto returnValue = app.MainLoop();
	return returnValue;
}