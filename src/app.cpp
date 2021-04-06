#include "app.hpp"
#include <glm/gtx/rotate_vector.hpp>

App::App(int clientWidth, int clientHeight) :
	graphics(std::make_unique<Graphics>(clientWidth, clientHeight))
{
	// window aspect ratio
	float aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);	
	
	// create scene with solids and surfaces
	scenes.push_back(std::make_shared<Scene>("tree"));
	scenes.at(scenes.size()-1)->addCamera(aspectRatio, glm::vec3(0.0f, 3.0f, 4.0f), glm::vec3(0.0f), glm::normalize(glm::vec3(0.0f, 4.0f, -3.0f)), 45.0f, 0.1f, 100.0f );
	scenes.at(scenes.size()-1)->setActiveCamera(0);

	scenes.at(scenes.size()-1)->addDirectionalLight(glm::vec3(-6.0f, 10.0f, 2.0f), glm::vec3(0.025f), glm::vec3(5.0f), glm::vec3(1.0f), glm::vec3(0.5f, -1.5f, -0.25f));
	
	scenes.at(scenes.size()-1)->addObject("../assets/tree/tree.glb", glm::mat4(1.0f));
	
	scenes.at(scenes.size()-1)->setGridAxis(8);
	
	// create scene with solids
	scenes.push_back(std::make_shared<Scene>("composition"));
	scenes.at(scenes.size()-1)->addCamera(aspectRatio, glm::vec3(0.0f, 3.0f, 4.0f), glm::vec3(0.0f), glm::normalize(glm::vec3(0.0f, 4.0f, -3.0f)), 45.0f, 0.1f, 100.0f );
	scenes.at(scenes.size()-1)->setActiveCamera(0);

	scenes.at(scenes.size()-1)->addSpotLight(glm::vec3(-5.0f, 15.0f, 5.0f), glm::vec3(0.025f), glm::vec3(100.0f), glm::vec3(1.0f), glm::vec3(0.25f, -1.0f, -0.25f), 30.0f, 35.0f);
	
	scenes.at(scenes.size()-1)->addObject("../assets/composition/composition.glb", glm::mat4(1.0f));
	
	scenes.at(scenes.size()-1)->setGridAxis(8);
}

void App::drawScene(float& delta, int index, int width, int height, DRAWING_MODE mode, bool debug)
{
	// get shader
	//Shader s = graphics->getBlinnPhongShader();
	Shader s = graphics->getPBRShader();

	if(index < scenes.size())
	{
		if(graphics->getShadowQuality() != SHADOW_QUALITY::OFF)
		{
			s.use();
			s.setInt("shadowOn", 1);
			s.setInt("shadowMethod", static_cast<int>(graphics->getShadowMethod()));

			// SHADOW PASS : directional & spot light sources
			directionalShadowPass(index, mode);

			// COLOR PASS : multisampling
			colorMultisamplePass(index, width, height, mode, debug);

			// blit to normal framebuffer (resolve multisampling)
			graphics->getNormalFBO()->bind();
			graphics->getMultisampleFBO()->blitFramebuffer(graphics->getNormalFBO(), width, height);

			// bind to default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// draw post processing quad
			graphics->getQuadMesh()->draw(graphics->getPostProcessingShader());
		}
		else
		{
			s.use();
			s.setInt("shadowOn", 0);
			s.setInt("shadowMethod", static_cast<int>(graphics->getShadowMethod()));
			
			// render to multisample framebuffer
			glViewport(0, 0, width, height);
			graphics->getMultisampleFBO()->bind();

			// draw scene
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			s.setVec3f("cam.viewPos", scenes.at(index)->getActiveCamera()->getPosition());
			s.setMatrix("view", scenes.at(index)->getActiveCamera()->getViewMatrix());
			s.setMatrix("proj", scenes.at(index)->getActiveCamera()->getProjectionMatrix());
			s.setLighting(scenes.at(index)->getDLights(), scenes.at(index)->getSLights());
			scenes.at(index)->draw(s, graphics, false, mode, debug);

			// blit to normal framebuffer (resolve multisampling)
			graphics->getNormalFBO()->bind();
			graphics->getMultisampleFBO()->blitFramebuffer(graphics->getNormalFBO(), width, height);

			// bind to default framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// draw post processing quad
			graphics->getQuadMesh()->draw(graphics->getPostProcessingShader());
		}
	}
	else
	{
		std::cerr << "Error: wrong scene index supplied for draw command.\n";
	}
}

void App::resizeScreen(int clientWidth, int clientHeight)
{
	// window aspect ratio
	float aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);	

	#pragma omp for
	for(int i{0}; i < scenes.size(); ++i)
	{
		scenes.at(i)->getActiveCamera()->updateProjectionMatrix(clientWidth, clientHeight);
	}

	graphics->resizeScreen(clientWidth, clientHeight);
}

void App::updateSceneActiveCameraView(int index, const std::bitset<16> & inputs, std::array<int, 3> & mouse, float delta)
{
	if(index < scenes.size())
	{
		scenes.at(index)->getActiveCamera()->updateViewMatrix(inputs, mouse, delta);
	}
}

std::vector<std::shared_ptr<Scene>> & App::getScenes()
{
	return scenes;
}

std::unique_ptr<Graphics> & App::getGraphics()
{
	return graphics;
}

void App::directionalShadowPass(int index, DRAWING_MODE mode)
{
	glViewport(0, 0, static_cast<int>(graphics->getShadowQuality()), static_cast<int>(graphics->getShadowQuality()));
	graphics->getShadowMappingShader().use();
	graphics->getShadowMappingShader().setMatrix("proj", graphics->getOrthoProjection());

	// render directional depth maps
	int sLightsOffset{0};
	for(int i{0}; i < scenes.at(index)->getDLights().size(); ++i, ++sLightsOffset)
	{
		graphics->getStdDepthFBO(i)->bind();
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::vec3 lightPosition = scenes.at(index)->getDLights().at(i)->getPosition();
		glm::vec3 lightTarget = lightPosition + scenes.at(index)->getDLights().at(i)->getDirection();
		glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));

		graphics->getShadowMappingShader().setMatrix("view", lightView);

		// draw scene
		scenes.at(index)->draw(graphics->getShadowMappingShader(), graphics, true, mode);
	}

	for(int i{0}; i < scenes.at(index)->getSLights().size(); ++i)
	{
		graphics->getStdDepthFBO(sLightsOffset + i)->bind();
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::vec3 lightPosition = scenes.at(index)->getSLights().at(i)->getPosition();
		glm::vec3 lightDirection = scenes.at(index)->getSLights().at(i)->getDirection();
		glm::vec3 lightTarget = lightPosition + lightDirection;
		glm::vec3 up = (lightDirection == glm::vec3(0.0f, -1.0f, 0.0f)) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, up);
		float outerCutOff = scenes.at(index)->getSLights().at(i)->getOuterCutOff();

		glm::mat4 spotProj = glm::perspective(
					outerCutOff * 2.0f, 1.0f,
					scenes.at(index)->getActiveCamera()->getNearPlane(),
					scenes.at(index)->getActiveCamera()->getFarPlane()
					);

		graphics->getShadowMappingShader().setMatrix("proj", spotProj);
		graphics->getShadowMappingShader().setMatrix("view", lightView);

		// draw scene
		scenes.at(index)->draw(graphics->getShadowMappingShader(), graphics, true, mode);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_CULL_FACE);
}

void App::colorMultisamplePass(int index, int width, int height, DRAWING_MODE mode, bool debug)
{
	// render to multisample framebuffer
	glViewport(0, 0, width, height);
	graphics->getMultisampleFBO()->bind();

	// get shader
	//Shader s = graphics->getBlinnPhongShader();
	Shader s = graphics->getPBRShader();

	// draw scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	s.use();
	s.setVec3f("cam.viewPos", scenes.at(index)->getActiveCamera()->getPosition());
	s.setMatrix("view", scenes.at(index)->getActiveCamera()->getViewMatrix());
	s.setMatrix("proj", scenes.at(index)->getActiveCamera()->getProjectionMatrix());
	s.setLighting(scenes.at(index)->getDLights(), scenes.at(index)->getSLights());

	// set shadow maps (point first, dir second and spot last)
	int nbDLights = scenes.at(index)->getDLights().size();
	int nbSLights = scenes.at(index)->getSLights().size();

	int textureOffset{4};
	int depthMapIndex{0};

	for(int i{0}; i < scenes.at(index)->getDLights().size(); ++i)
	{
		glm::vec3 lightPosition = scenes.at(index)->getDLights().at(i)->getPosition();
		glm::vec3 lightTarget = lightPosition + scenes.at(index)->getDLights().at(i)->getDirection();
		glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));

		glActiveTexture(GL_TEXTURE0 + textureOffset);
		glBindTexture(GL_TEXTURE_2D, graphics->getStdDepthFBO(depthMapIndex)->getAttachments().at(0).id);
		s.setInt("depthMap[" + std::to_string(depthMapIndex) + "]", textureOffset);
		s.setMatrix("light[" + std::to_string(i) + "].lightSpaceMatrix", graphics->getOrthoProjection() * lightView);
		depthMapIndex++;
		textureOffset++;
	}

	for(int i{0}; i < scenes.at(index)->getSLights().size(); ++i)
	{
		float outerCutOff = scenes.at(index)->getSLights().at(i)->getOuterCutOff();
		glm::vec3 lightPosition = scenes.at(index)->getSLights().at(i)->getPosition();
		glm::vec3 lightTarget = lightPosition + scenes.at(index)->getSLights().at(i)->getDirection();
		glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 spotProj = glm::perspective(
					outerCutOff * 2.0f, 1.0f,
					scenes.at(index)->getActiveCamera()->getNearPlane(),
					scenes.at(index)->getActiveCamera()->getFarPlane()
					);

		glActiveTexture(GL_TEXTURE0 + textureOffset);
		glBindTexture(GL_TEXTURE_2D, graphics->getStdDepthFBO(depthMapIndex)->getAttachments().at(0).id);
		s.setInt("depthMap[" + std::to_string(depthMapIndex) + "]", textureOffset);
		s.setMatrix("light[" + std::to_string(i + nbDLights) + "].lightSpaceMatrix", spotProj * lightView);
		depthMapIndex++;
		textureOffset++;
	}

	scenes.at(index)->draw(s, graphics, false, mode, debug);
}
