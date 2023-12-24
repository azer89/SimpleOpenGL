#ifndef APP_PBR_MODEL
#define APP_PBR_MODEL

#include "AppBase.h"
#include "Model.h"
#include "Light.h"

#include <memory>

class AppPBRModel final : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void RenderScene(const Shader& shader);

private:
	// Scene
	std::unique_ptr<Model> renderModel1;
	std::unique_ptr<Model> renderModel2;
	std::unique_ptr<Model> renderModel3;

	// Lights
	std::unique_ptr<Shader> lightSphereShader;
	std::vector<Light> lights;
	std::vector<float> lightAngles;
	std::vector<float> lightRadii;
};

#endif