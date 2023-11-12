#ifndef __APP_DEFERRED_SPONZA_H__
#define __APP_DEFERRED_SPONZA_H__

#include "AppBase.h"
#include "Shader.h"
#include "Model.h"
#include "Light.h"
#include "Texture.h"

class AppDeferredSponza : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void InitLights(); 

	void UpdateLightPositions();

	void RenderScene(const Shader& shader);
	void RenderLights();

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