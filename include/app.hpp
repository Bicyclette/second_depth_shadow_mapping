#ifndef APP_HPP
#define APP_HPP

#include <iostream>
#include <memory>
#include <utility>
#include "scene.hpp"
#include "graphics.hpp"

class App
{
	public:

		App(int clientWidth, int clientHeight);
		void drawScene(float& delta, int index, int width, int height, DRAWING_MODE mode = DRAWING_MODE::SOLID, bool debug = false);
		void resizeScreen(int clientWidth, int clientHeight);
		void updateSceneActiveCameraView(int index, const std::bitset<16> & inputs, std::array<int, 3> & mouse, float delta);
		std::vector<std::shared_ptr<Scene>> & getScenes();
		std::unique_ptr<Graphics> & getGraphics();

	private:

		std::vector<std::shared_ptr<Scene>> scenes;
		std::unique_ptr<Graphics> graphics;

		void directionalShadowPass(int index, DRAWING_MODE mode = DRAWING_MODE::SOLID);
		void colorMultisamplePass(int index, int width, int height, DRAWING_MODE mode = DRAWING_MODE::SOLID, bool debug = false);
};


#endif
