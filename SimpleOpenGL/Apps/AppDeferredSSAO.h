#ifndef __APP_DEFERRED_SAAO_H_
#define __APP_DEFERRED_SAAO_H_

#include "AppBase.h"
#include "Model.h"
#include "Texture.h"
#include "Light.h"

#include <memory>

class AppDeferredSSAO : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void RenderScene(const Shader& shader);

	void InitLights();
	void RenderLights();
	void UpdateLightPositions();

private:
	// Scene
	std::unique_ptr<Model> sponzaModel;

	// Lights
	std::unique_ptr<Shader> lightSphereShader;
	std::vector<Light> lights;
	std::vector<float> lightAngles;
	std::vector<float> lightRadii;
};

#endif