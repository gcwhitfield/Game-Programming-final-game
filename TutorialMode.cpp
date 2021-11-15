#include "TutorialMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>
#include <iostream>

// TutorialMode takes in a std::shared_ptr<Mode> called 'current'. 'current' is 
// current mode that the game is displaying. It is the same 'current' that is defined
// in main.cpp when the gl window is created
//
// After the player completes the tutorial, 'current' be set to PlayMode. Because 
// 'current' is a shared pointer, the value of this->current will alias with the 
// value of 'current' inside of main.cpp. Therefore, to transition to the next mode.
// all we need to do is 'this->current = std::make_shared<PlayMode>();
TutorialMode::TutorialMode(std::shared_ptr < Mode > current) {
    this->current = current;
}

TutorialMode::~TutorialMode() {

}

bool TutorialMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    if (evt.type == SDL_KEYDOWN) {
        return true;
    }
    if (evt.key.keysym.sym == SDLK_SPACE) {
        // transition to the gameplay mode
        Mode::set_current(std::make_shared<PlayMode>());
        return true;
    }
    return false;
}

void TutorialMode::update(float elapsed) {

}

void TutorialMode::draw(glm::uvec2 const &drawable_size) {

    { //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		float ofs = 2.0f / drawable_size.y;
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f));

		// constexpr float H = 0.09f;

		//making draw_text more modular, string, location, size
		auto draw_text = [&](std::string str, glm::vec3 pos, float sz = 0.09f) {
			lines.draw_text(str,
							glm::vec3(pos.x, pos.y, pos.z),
							glm::vec3(sz, 0.0f, 0.0f), glm::vec3(0.0f, sz, 0.0f),
							glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			lines.draw_text(str,
							glm::vec3(pos.x + ofs, pos.y + ofs, pos.z),
							glm::vec3(sz, 0.0f, 0.0f), glm::vec3(0.0f, sz, 0.0f),
							glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		};

        draw_text("COLDBREW - a Starbucks Simulator :^)", glm::vec3(0, 0, 0), 1.0f);

	}
}