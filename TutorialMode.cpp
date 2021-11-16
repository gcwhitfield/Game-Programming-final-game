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

TutorialMode::TutorialMode() {
    state = State::SCREEN_0;
}

TutorialMode::~TutorialMode() {

}

bool TutorialMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    if (evt.type == SDL_KEYDOWN) {
        return true;
    } 
    else if (evt.key.keysym.sym == SDLK_SPACE) {
        // if we are on the last screen and the player presses spacebar, transition 
        // to the gameplay scene
        if (state == SCREEN_5) {
            // transition to PlayMode
            Mode::set_current(std::make_shared<PlayMode>());
            return true;
        }
    } 
    else if (evt.key.keysym.sym == SDLK_e) {
        switch (state) {
            case SCREEN_0:
                state = SCREEN_1;
                break;
            case SCREEN_1:
                state = SCREEN_2;
                break;
            case SCREEN_2:
                state = SCREEN_3;
                break;
            case SCREEN_3:
                state = SCREEN_4;
                break;
            case SCREEN_4:
                state = SCREEN_5;
                break;
            default:
                // do nothing
                break;
        }
        return true;
    }
    return false;
}

void TutorialMode::update(float elapsed) {

}

void TutorialMode::draw(glm::uvec2 const &drawable_size) {

    // ----- set the background color -----
    glm::uvec4 background_color(37, 12, 10, 0);
    glClearColor(
        background_color.x / 255.0,  // red
        background_color.y / 255.0,  // green
        background_color.z / 255.0,  // blue 
        background_color.w / 255.0); // alpha
    glClear(GL_COLOR_BUFFER_BIT);

    { // ----- use DrawLines to overlay some text -----
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

        float spacing = 0.2f;
        float left_align = -0.8;
        switch (state)
        {
            case SCREEN_0:
            {
                draw_text("COLDBREW", glm::vec3(left_align, spacing * 2, 0), 0.1f);
                draw_text("a Starbucks Simulator", glm::vec3(left_align, 0, 0), 0.1f);
                draw_text(" Press E to continue", glm::vec3(left_align, -spacing * 2, 0), 0.1f);
            } break;
            case SCREEN_1:
            {
                draw_text("Welcome to your first day on the job as a Starbucks barista!", glm::vec3(left_align, spacing * 2, 0), 0.1f);
                draw_text("I'm so glad that the manager ended up hiring you.", glm::vec3(left_align, spacing * 1, 0), 0.1f);
                draw_text("The manager doesn't typically hire cats... so honestly I was ", glm::vec3(left_align, spacing * -0, 0), 0.1f);
                draw_text("pleasantly surprised to hear that you got the job. ", glm::vec3(left_align, spacing * -1, 0), 0.1f);
                draw_text("I'm sure you'll make a great barista!", glm::vec3(left_align, spacing * -2, 0), 0.1f);
                draw_text("Press E to continue", glm::vec3(left_align, spacing * -4, 0), 0.1f);

            } break;
            case SCREEN_2:
            {
                draw_text("This Starbucks is different from other ones that you've been", glm::vec3(left_align, spacing * 2, 0), 0.1f);
                draw_text("to, so let me give you the rundown. The customers here ", glm::vec3(left_align, spacing * 1, 0), 0.1f);
                draw_text("sit down at the chairs and you, the barista, must", glm::vec3(left_align, spacing * -0, 0), 0.1f);
                draw_text("go up to them to take their order. After taking their order ", glm::vec3(left_align, spacing * -1, 0), 0.1f);
                draw_text("You can go back to the counter, collect the ingredients needed...", glm::vec3(left_align, spacing * -2, 0), 0.1f);
                draw_text("Press E to continue", glm::vec3(left_align, spacing * -4, 0), 0.1f);
            } break;
            case SCREEN_3:
            {
                draw_text("... and serve them their order. But be careful! The customers are ", glm::vec3(left_align, spacing * 2, 0), 0.1f);
                draw_text("impatient, so don't let them wait too long! Obviously, their", glm::vec3(left_align, spacing * 1, 0), 0.1f);
                draw_text("orders must be correct as well. In other words, you need to ", glm::vec3(left_align, spacing * -0, 0), 0.1f);
                draw_text("be QUICK ON YOUR FEET and give ACCURATE ORDERS to succeed as", glm::vec3(left_align, spacing * -1, 0), 0.1f);
                draw_text("a barista here. Good luck!", glm::vec3(left_align, spacing * -2, 0), 0.1f);
                draw_text("Press E to continue", glm::vec3(left_align, spacing * -4, 0), 0.1f);

            } break;
            case SCREEN_4:
            {
                draw_text("... and by the way - do NOT let the manager see you in your", glm::vec3(left_align, spacing * 2, 0), 0.1f);
                draw_text("cat form. The manager hates cats!", glm::vec3(left_align, spacing * 1, 0), 0.1f);
                draw_text("... and don't bump into customers while delivering orders.", glm::vec3(left_align, spacing * -1, 0), 0.1f);
                draw_text("You'll spill the drink and you'll have to remake it again!", glm::vec3(left_align, spacing * -2, 0), 0.1f);
     
                draw_text("Press E to continue", glm::vec3(left_align, spacing * -4, 0), 0.1f);
            } break;
            case SCREEN_5:
            {
                draw_text("MOUSE MOVEMENT - move camera", glm::vec3(left_align, spacing * 2, 0), 0.1f);
                draw_text("WASD - move player", glm::vec3(left_align, spacing * 1, 0), 0.1f);
                draw_text("RIGHT CLICK - transition between cat and human", glm::vec3(left_align, spacing * -0, 0), 0.1f);
                draw_text("LEFT CLICK - collect ingredient/serve order", glm::vec3(left_align, spacing * -1, 0), 0.1f);
                draw_text("SPACEBAR - mash spacebar to fly around in cat mode", glm::vec3(left_align, spacing * -2, 0), 0.1f);

                draw_text("Press Spacebar to begin Day 1", glm::vec3(left_align, spacing * -4, 0), 0.1f);
            } break;
        }

	}
}