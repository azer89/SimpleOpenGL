#ifndef IBL
#define IBL

#include "AppBase.h"
#include "Shader.h"
#include "PipelineIBL.h"

#include <memory>

class AppIBL final : AppBase
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