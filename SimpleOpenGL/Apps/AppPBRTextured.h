#ifndef __APP_PBR_TEXTURED_H_
#define __APP_PBR_TEXTURED_H_

#include "AppBase.h"
#include "Model.h"
#include "Light.h"


#include <memory>

class AppPBRTextured : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void RenderScene(const Shader& shader);

	void InitLights();
	void RenderLights();

private:
	// Scene
	std::unique_ptr<Model> sponzaModel;
	std::unique_ptr<Model> adamModel;

	// Lights
	std::unique_ptr<Shader> lightSphereShader;
	std::vector<Light> lights;
	std::vector<float> lightAngles;
	std::vector<float> lightRadii;
};

#endif