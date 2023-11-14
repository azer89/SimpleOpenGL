#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"
#include "AppModelLoading.h"
#include "AppSkyboxWithModel.h"
#include "AppSkyboxWithCube.h"
#include "AppDeferred.h"
#include "AppDeferredSponza.h"
#include "AppGeom.h"
#include "AppSSAO.h"

int main()
{
	AppSSAO app;
	auto returnValue = app.MainLoop();
	return returnValue;
}