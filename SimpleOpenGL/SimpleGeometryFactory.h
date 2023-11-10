#ifndef __SIMPLE_GEOMETRY_FACTORY_H__
#define __SIMPLE_GEOMETRY_FACTORY_H__

#include <vector>

class SimpleGeometryFactory
{
public:
	static std::vector<float> GenerateCubeVertices();
	static std::vector<float> GenerateQuadVertices();
};

#endif
