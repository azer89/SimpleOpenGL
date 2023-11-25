#ifndef __APP_PBR_MODEL_H_
#define __APP_PBR_MODEL_H_

#include "AppBase.h"
#include "Model.h"
#include "Light.h"

#include <memory>

class AppPBRModel : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void RenderScene(const Shader& shader);

private:
	// Scene
	std::unique_ptr<Model> renderModel;

	// Lights
	std::unique_ptr<Shader> lightSphereShader;
	std::vector<Light> lights;
	std::vector<float> lightAngles;
	std::vector<float> lightRadii;
};

#endif