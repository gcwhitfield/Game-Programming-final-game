#pragma once
#include "Mode.hpp"
#include "PlayMode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "Sound.hpp"
#include "catcoffee.hpp"
#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <random>

#define PI_F 3.1415926f

struct TutorialMode : Mode {
	TutorialMode(std::shared_ptr<Mode> current);
	virtual ~TutorialMode();
	

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

    std::shared_ptr<Mode> current; // the current mode from main.cpp. See TutorialMode.cpp for more comments
};