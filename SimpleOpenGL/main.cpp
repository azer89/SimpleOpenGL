#include "AppBoxes.h"
#include "AppBlinnPhong.h"

int main()
{
	AppBlinnPhong app;
	auto returnValue = app.MainLoop();
	return returnValue;
}