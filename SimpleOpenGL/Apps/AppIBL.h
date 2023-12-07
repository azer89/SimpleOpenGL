#ifndef __IBL_H__
#define __IBL_H__

#include "AppBase.h"
#include "Shader.h"
#include "PipelineIBL.h"

#include <memory>

class AppIBL : AppBase
{
public:
	int MainLoop() override;

private:
	// Debugging purpose
	std::unique_ptr<Shader> simpleCubeShader;
	void InitDebugCubes();
	void RenderDebugCubes(const Cube& cube, const PipelineIBL& ibl);

};

#endif