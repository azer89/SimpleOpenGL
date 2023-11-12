#include "XMLReader.h"
#include "AppSettings.h"
#include "tinyxml2.h"

#include <iostream>

using namespace tinyxml2;

inline int GetInt(const XMLElement* elem, const char* name = "value")
{
	int i;
	elem->QueryIntAttribute(name, &i);
	return i;
}

inline double GetDouble(const XMLElement* elem, const char* name = "value")
{
	double d;
	elem->QueryDoubleAttribute(name, &d);
	return d;
}

inline const char* GetString(const XMLElement* elem, const char* name = "value")
{
	return elem->Attribute(name);
}

void XMLReader::LoadSettings()
{
	XMLDocument doc;
	std::cout << "Parsing XML file: " << AppSettings::XMLFilePath << '\n';
	auto eResult = doc.LoadFile(AppSettings::XMLFilePath.c_str());
	if (eResult != XML_SUCCESS)
	{
		std::cerr << "Cannot find XML file: " << AppSettings::XMLFilePath << '\n';
	}

	// Root
	auto root = doc.FirstChild();

	// General settings
	auto general_parent = root->FirstChildElement("General");
	AppSettings::ScreenWidth = GetInt(general_parent->FirstChildElement("ScreenWidth"));
	AppSettings::ScreenHeight = GetInt(general_parent->FirstChildElement("ScreenHeight"));
	AppSettings::ScreenTitle = GetString(general_parent->FirstChildElement("ScreenTitle"));
	AppSettings::ShaderFolder = GetString(general_parent->FirstChildElement("ShaderFolder"));
	AppSettings::TextureFolder = GetString(general_parent->FirstChildElement("TextureFolder"));
	AppSettings::ModelFolder = GetString(general_parent->FirstChildElement("ModelFolder"));
}