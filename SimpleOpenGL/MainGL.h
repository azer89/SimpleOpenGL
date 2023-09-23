#ifndef __MAIN_GL_H__
#define __MAIN_GL_H__

#include <string>

class MainGL
{
private:
	// Settings
	const unsigned int SCREEN_WIDTH = 800;
	const unsigned int SCREEN_HEIGHT = 600;
	const std::string SCREEN_TITLE = "Simple OpenGL";

public:
	int MainLoop();
};

#endif
