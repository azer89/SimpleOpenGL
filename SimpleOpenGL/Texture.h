#ifndef __TEXTURE_H__
#define __TEXTURE_H__

class Texture
{
private:
	unsigned int ID;

public:
	Texture(const char* imageFilePath);
	void Bind();
};

#endif