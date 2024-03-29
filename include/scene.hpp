#ifndef SCENE_HPP
#define SCENE_HPP

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <utility>
#include "object.hpp"
#include "camera.hpp"
#include "color.hpp"
#include "grid_axis.hpp"
#include "graphics.hpp"

class Scene
{
	public:

		Scene(std::string pName);
		void addObject(std::string filePath, glm::mat4 aModel = glm::mat4(1.0f), const std::vector<glm::mat4> & instanceModel = std::vector<glm::mat4>());
		void addCamera(float aspectRatio, glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 target = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float fov = 45.0f, float near = 0.1f, float far = 100.0f);
		void addDirectionalLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, glm::vec3 dir);
		void addSpotLight(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, glm::vec3 dir, float innerAngle, float outerAngle);
		void setGridAxis(int gridDim);
		void setActiveCamera(int index);
		void updateCameraPerspective(float aspectRatio);
		std::vector<std::shared_ptr<Camera>> & getCameras();
		std::shared_ptr<Camera> & getActiveCamera();
		std::string & getName();
		void draw(Shader & shader, std::unique_ptr<Graphics> & graphics, bool shadowPass = false, DRAWING_MODE mode = DRAWING_MODE::SOLID, bool debug = false, bool lance_williams = false);
		std::vector<std::shared_ptr<DirectionalLight>> & getDLights();
		std::vector<std::shared_ptr<SpotLight>> & getSLights();
		bool getMixed();

	private:

		std::string name;

		std::shared_ptr<Camera> activeCamera;
		std::vector<std::shared_ptr<Camera>> cameras;

		std::vector<std::shared_ptr<Object>> objects;
		bool mixed;
		
		std::vector<std::shared_ptr<DirectionalLight>> dLights;
		std::vector<std::shared_ptr<SpotLight>> sLights;
		
		std::unique_ptr<GridAxis> gridAxis;
};

#endif
