#include "scene.hpp"

Scene::Scene(std::string pName) : name(pName), mixed(false) {}

void Scene::addObject(std::string filePath, glm::mat4 aModel, const std::vector<glm::mat4> & instanceModel)
{
	objects.push_back(std::make_shared<Object>(filePath, aModel));

	if(instanceModel.size() > 0)
	{
		objects.at(objects.size() - 1)->setInstancing(instanceModel);
	}

	bool solid{objects.at(0)->isSolid()};
	if(solid)
		mixed = false;
	else
		mixed = true;
	for(int i{1}; i < objects.size(); ++i)
	{
		if(objects.at(i)->isSolid() != solid)
		{
			mixed = true;
			break;
		}
	}
}

void Scene::addCamera(float aspectRatio, glm::vec3 pos, glm::vec3 target, glm::vec3 up, float fov, float near, float far)
{
	cameras.push_back(std::make_shared<Camera>(aspectRatio, pos, target, up, fov, near, far));
}

void Scene::addDirectionalLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, glm::vec3 dir)
{
	dLights.push_back(std::make_shared<DirectionalLight>(pos, amb, diff, spec, dir));
	dLights.at(dLights.size() - 1)->setModelMatrix(glm::mat4(1.0f));
}

void Scene::addSpotLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, glm::vec3 dir, float innerAngle, float outerAngle)
{
	sLights.push_back(std::make_shared<SpotLight>(pos, amb, diff, spec, dir, innerAngle, outerAngle));
	sLights.at(sLights.size() - 1)->setModelMatrix(glm::mat4(1.0f));
}

void Scene::setGridAxis(int gridDim)
{
	gridAxis = std::make_unique<GridAxis>(8);
}

void Scene::setActiveCamera(int index)
{
	activeCamera = cameras.at(index);
}

void Scene::updateCameraPerspective(float aspectRatio)
{
	for(int i{0}; i < cameras.size(); ++i)
	{
		std::shared_ptr<Camera> cam = cameras.at(i);
		cameras.at(i)->setProjection(aspectRatio, cam->getNearPlane(), cam->getFarPlane());
	}
}

std::vector<std::shared_ptr<Camera>> & Scene::getCameras()
{
	return cameras;
}

std::shared_ptr<Camera> & Scene::getActiveCamera()
{
	return activeCamera;
}

std::string & Scene::getName()
{
	return name;
}

void Scene::draw(Shader & shader, std::unique_ptr<Graphics> & graphics, bool shadowPass, DRAWING_MODE mode, bool debug, bool lance_williams)
{
	shader.use();
	shader.setFloat("bias", graphics->getBias());

	if(debug)
	{
		if(gridAxis)
		{
			gridAxis->draw(activeCamera->getViewMatrix(), activeCamera->getProjectionMatrix());
		}

		for(int i{0}; i < dLights.size(); ++i)
		{
			dLights.at(i)->setViewMatrix(activeCamera->getViewMatrix());
			dLights.at(i)->setProjMatrix(activeCamera->getProjectionMatrix());
			dLights.at(i)->draw(graphics->getOrthoDimension());
		}
		for(int i{0}; i < sLights.size(); ++i)
		{
			sLights.at(i)->setViewMatrix(activeCamera->getViewMatrix());
			sLights.at(i)->setProjMatrix(activeCamera->getProjectionMatrix());
			sLights.at(i)->draw();
		}
	}

	for(int i{0}; i < objects.size(); ++i)
	{
		objects.at(i)->draw(shader, shadowPass, mode, lance_williams);
	}
}

std::vector<std::shared_ptr<DirectionalLight>> & Scene::getDLights()
{
	return dLights;
}

std::vector<std::shared_ptr<SpotLight>> & Scene::getSLights()
{
	return sLights;
}

bool Scene::getMixed()
{
	return mixed;
}
