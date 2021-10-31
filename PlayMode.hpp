#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <random>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	float time_until_next_manager_appearance = 1.0f; // seconds

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//player info:
	struct Player {
		Player() {}; // TODO: Implement this
		~Player() {}; // TODO: Implement this
		WalkPoint at;
		//transform is at player's feet and will be yawed by mouse left/right motion:
		Scene::Transform *transform = nullptr;
		//camera is at player's head and will be pitched by mouse up/down motion:
		Scene::Camera *camera = nullptr;
		Scene::Drawable *drawPlayer;
		enum Status {
			Human, Cat
		};
		Status playerStatus;
		glm::vec3 curDir = glm::vec3(0.0f); //Direction player is currently facing, for cat (differnet then vel vec3)
		glm::vec3 catVelocity = glm::vec3(0.0f); //Current momentum, for cat, mass assumed (but could change in transition)
		glm::vec3 humanAcc = glm::vec3(0.0f); 
		//Note, velocity is added with each pump, and lost in y over time

		WalkPoint walkpoint;
		float height = 0.0f; //0 for human, > 0 for cat
	} player;

	struct state //Game state
	{
		int score = 0;
		float stablization = 1.0f;
		float time = 0.0f;

		enum PlayState {
			ongoing, won, lost, menu
		};
		PlayState playing = ongoing; 
		//Put order here
	};




};
