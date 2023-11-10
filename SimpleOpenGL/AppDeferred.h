#ifndef __APP_DEFERRED_H__
#define __APP_DEFERRED_H__

#include "AppBase.h"
#include "Shader.h"
#include "Model.h"
#include "Light.h"

class AppDeferred : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void InitLights(); 

	void RenderPlane(const Shader& shader);
	void RenderFoxes(const Shader& shader);
	void RenderQuad();
	

private:
	std::unique_ptr<Model> foxModel;

	std::vector<Light> lights;

	unsigned int planeVAO = 0;
	unsigned int planeVBO = 0;
};


#endif