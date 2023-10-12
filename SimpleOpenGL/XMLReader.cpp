#include "XMLReader.h"
#include "tinyxml2.h"

#include <iostream>

using namespace tinyxml2;

/*unsigned int AppSettings::ScreenWidth = 0;
unsigned int AppSettings::ScreenHeight = 0;
std::string AppSettings::ScreenTitle = 0;
std::string AppSettings::VertexShaderFile = "";
std::string AppSettings::FragmentShaderFile = "";*/

inline const char* GetString(const XMLElement* elem, const char* name)
{
	return elem->Attribute(name);
}

void XMLReader::LoadSettings()
{
	XMLDocument doc;
	std::cout << "Parsing XML file: " << AppSettings::XMLFilePath << '\n';
	auto eResult = doc.LoadFile(AppSettings::XMLFilePath);
	if (eResult != XML_SUCCESS)
	{
		std::cerr << "Cannot find XML file: " << AppSettings::XMLFilePath << '\n';
	}

	// Root
	auto root = doc.FirstChild();

	// General settings
	auto general_parent = root->FirstChildElement("General");
	auto general_element = general_parent->FirstChildElement("material");
}

template<typename T>
static T XMLReader::GetElementValue(const char* elementName)
{

}

