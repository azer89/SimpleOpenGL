#ifndef __APP_SETTINGS_H__
#define __APP_SETTINGS_H__

#include <iostream>

namespace AppSettings
{
	const char* XMLFilePath = "C:/Users/azer/workspace/SimpleOpenGL/Settings/main.xml";

	extern unsigned int ScreenWidth;
	extern unsigned int ScreenHeight;
	extern std::string ScreenTitle;
	extern std::string VertexShaderFile;
	extern std::string FragmentShaderFile;
};

static class XMLReader
{
private:
	template<typename T>
	static T GetElementValue(const char* elementName);

public:
	static void LoadSettings();
};

#endif