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
	TutorialMode();
	virtual ~TutorialMode();
	

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

    enum State {
        SCREEN_0, // welcome, press 'e' to continue
        SCREEN_1, // explanation of the game premise
        SCREEN_2, // explanation of game controls, 
        SCREEN_3, // explanation of game controls, 
        SCREEN_4, // 'press spacebar to begin your first day on the jobb'
    } state;
};