#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include "window.hpp"
#include "app.hpp"
#include "framebuffer.hpp"

void render(std::unique_ptr<WindowManager> client, std::unique_ptr<App> app)
{
	ImGuiIO & io = ImGui::GetIO();
	int scene{0};
	
	int shadow_method{static_cast<int>(app->getGraphics()->getShadowMethod())};
	//int resolution{static_cast<int>(app->getGraphics()->getShadowQuality())};
	int resolution{5};
	float bias{app->getGraphics()->getBias()};
	
	float domain{app->getGraphics()->getOrthoDimension()};
	glm::vec3 dPos{app->getScenes().at(0)->getDLights().at(0)->getPosition()};
	glm::vec3 dDir{app->getScenes().at(0)->getDLights().at(0)->getDirection()};
	float cutoff{glm::degrees(app->getScenes().at(1)->getSLights().at(0)->getCutOff())};
	glm::vec3 sPos{app->getScenes().at(1)->getSLights().at(0)->getPosition()};
	glm::vec3 sDir{app->getScenes().at(1)->getSLights().at(0)->getDirection()};

	// delta
	double currentFrame{0.0f};
	double lastFrame{0.0};
	float delta{0.0f};
	
	while(client->isAlive())
	{
		client->checkEvents();
		currentFrame = omp_get_wtime();
		delta = static_cast<float>(currentFrame - lastFrame);

		// ImGui frame init code
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(client->getWindowPtr());
		ImGui::NewFrame();
	
		// >>>>>>>>>>IMGUI
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("Shadow mapping method");
		ImGui::SetWindowSize(ImVec2(350, 80));
		ImGui::RadioButton("lance williams", &shadow_method, 0);
		ImGui::RadioButton("second depth", &shadow_method, 1);
		ImGui::End();
		
		ImGui::SetNextWindowPos(ImVec2(0, 80));
		ImGui::Begin("Shadowmap resolution");
		ImGui::SetWindowSize(ImVec2(350, 170));
		ImGui::RadioButton("OFF", &resolution, 0);
		ImGui::RadioButton("TINY (256)", &resolution, 1);
		ImGui::RadioButton("SMALL (512)", &resolution, 2);
		ImGui::RadioButton("MEDIUM (1024)", &resolution, 3);
		ImGui::RadioButton("HIGH (2048)", &resolution, 4);
		ImGui::RadioButton("ULTRA (4096)", &resolution, 5);
		ImGui::End();
		
		ImGui::SetNextWindowPos(ImVec2(0, 250));
		if(scene == 0 || (scene == 1 && shadow_method == 0))
		{
			ImGui::Begin("Bias");
			ImGui::SetWindowSize(ImVec2(350, 60));
			ImGui::InputFloat("bias", &bias, 0.000025f, 0.000025f, "%.6f");
			ImGui::End();
		}

		ImGui::SetNextWindowPos(ImVec2(0, 310));
		ImGui::Begin("Scene state");
		ImGui::SetWindowSize(ImVec2(350, 50));
		if(app->getScenes().at(scene)->getMixed())
			ImGui::Text("Mix of solid and surface objects !");
		else
			ImGui::Text("Only solid objects !");
		ImGui::End();
		
		ImGui::SetNextWindowPos(ImVec2(0, 360));
		if(scene == 0)
		{
			ImGui::Begin("Directional light");
			ImGui::SetWindowSize(ImVec2(350, 210));
			ImGui::InputFloat("domain", &domain, 1.0f);
			ImGui::InputFloat("position X", &dPos.x, 0.1f);
			ImGui::InputFloat("position Y", &dPos.y, 0.1f);
			ImGui::InputFloat("position Z", &dPos.z, 0.1f);
			ImGui::InputFloat("direction X", &dDir.x, 0.1f);
			ImGui::InputFloat("direction Y", &dDir.y, 0.1f);
			ImGui::InputFloat("direction Z", &dDir.z, 0.1f);
			ImGui::End();
		}
		else if(scene == 1)
		{
			ImGui::Begin("Spot light");
			ImGui::SetWindowSize(ImVec2(350, 210));
			ImGui::InputFloat("cutoff", &cutoff, 5.0f);
			ImGui::InputFloat("position X", &sPos.x, 0.1f);
			ImGui::InputFloat("position Y", &sPos.y, 0.1f);
			ImGui::InputFloat("position Z", &sPos.z, 0.1f);
			ImGui::InputFloat("direction X", &sDir.x, 0.1f);
			ImGui::InputFloat("direction Y", &sDir.y, 0.1f);
			ImGui::InputFloat("direction Z", &sDir.z, 0.1f);
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			ImGui::End();
		}
		
		ImGui::SetNextWindowPos(ImVec2(0, 570));
		ImGui::Begin("Scene");
		ImGui::SetWindowSize(ImVec2(350, 80));
		ImGui::RadioButton("tree", &scene, 0);
		ImGui::RadioButton("composition", &scene, 1);
		ImGui::End();
		// <<<<<<<<<<IMGUI

		app->updateSceneActiveCameraView(scene, client->getUserInputs(), client->getMouseData(), delta);

		if(client->getUserInputs().test(5))
		{
			app->resizeScreen(client->getWidth(), client->getHeight());
		}
		
		// draw scene
		app->drawScene(delta, scene, client->getWidth(), client->getHeight(), DRAWING_MODE::SOLID, true);

		client->resetEvents();
		// render imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		SDL_GL_SwapWindow(client->getWindowPtr());
		lastFrame = currentFrame;

		// send IMGUI data to application
		if(shadow_method == 0)
		{
			app->getGraphics()->setShadowMethod(SHADOW_METHOD::LANCE_WILLIAMS);
		}
		else if(shadow_method == 1)
		{
			app->getGraphics()->setShadowMethod(SHADOW_METHOD::SECOND_DEPTH);
		}

		switch(resolution)
		{
			case 0: app->getGraphics()->setShadowQuality(SHADOW_QUALITY::OFF); break;
			case 1: app->getGraphics()->setShadowQuality(SHADOW_QUALITY::TINY); break;
			case 2: app->getGraphics()->setShadowQuality(SHADOW_QUALITY::SMALL); break;
			case 3: app->getGraphics()->setShadowQuality(SHADOW_QUALITY::MED); break;
			case 4: app->getGraphics()->setShadowQuality(SHADOW_QUALITY::HIGH); break;
			case 5: app->getGraphics()->setShadowQuality(SHADOW_QUALITY::ULTRA); break;
			default: break;
		}

		app->getGraphics()->setBias(bias);

		if(scene == 0)
		{
			app->getGraphics()->setOrthoDimension(domain);
			app->getScenes().at(scene)->getDLights().at(0)->setPosition(dPos);
			app->getScenes().at(scene)->getDLights().at(0)->setDirection(dDir);
		}
		else if(scene == 1)
		{
			app->getScenes().at(scene)->getSLights().at(0)->setCutOff(cutoff);
			app->getScenes().at(scene)->getSLights().at(0)->setOuterCutOff(cutoff + 5.0f);
			app->getScenes().at(scene)->getSLights().at(0)->setPosition(glm::vec3(sPos));
			app->getScenes().at(scene)->getSLights().at(0)->setDirection(sDir);
		}
	}

}

int main(int argc, char* argv[])
{
	std::unique_ptr<WindowManager> client{std::make_unique<WindowManager>("Second Depth Shadow Mapping")};
	std::unique_ptr<App> app{std::make_unique<App>(client->getWidth(), client->getHeight())};
	render(std::move(client), std::move(app));

	return 0;
}
