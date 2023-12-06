#ifndef __APP_SAAO_H_
#define __APP_SAAO_H_

#include "AppBase.h"
#include "Model.h"
#include "Texture.h"
#include "Light.h"

#include <memory>

class AppSSAO : AppBase
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
	std::unique_ptr<Model> renderModel;
	std::unique_ptr<Texture> grassTexture;
	unsigned int planeVAO = 0;

	// Lights
	std::unique_ptr<Shader> lightSphereShader;
	std::vector<Light> lights;
	std::vector<float> lightAngles;
	std::vector<float> lightRadii;
};

#endif