#include "AppBoxes.h"
#include "AppBlinnPhong.h"
#include "AppShadowMapping.h"
#include "AppModelLoading.h"
#include "AppSkyboxWithModel.h"
#include "AppSkyboxWithCube.h"
#include "AppEdge.h"
#include "AppBloom.h"
#include "AppDeferred.h"
#include "AppDeferredSponza.h"
#include "AppDeferredSSAO.h"
#include "AppGeom.h"
#include "AppSSAO.h"
#include "AppPBRTextured.h"
#include "AppPBRModel.h"
#include "AppIBL.h"

int main()
{
	// You can change the demo by selecting a class from the list above
	AppShadowMapping app;

	auto returnValue = app.MainLoop();
	
	return returnValue;
}