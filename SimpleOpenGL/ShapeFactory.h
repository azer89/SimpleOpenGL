#ifndef __SHAPE_FACTORY_H__
#define __SHAPE_FACTORY_H__

#include <vector>

class ShapeFactory
{
public:
	static std::vector<float> GenerateCubeVertices();
	static std::vector<float> GenerateQuadVertices();
};

#endif
