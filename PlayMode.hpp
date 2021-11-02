#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();
	

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up,space;

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

		glm::vec3 catVelocity = glm::vec3(0.0f); //Current momentum, for cat, mass assumed (but could change in transition)11
		//Note, velocity is added with each pump, and lost in y over time
		float height = 0.0f; //0 for human, > 0 for cat
		float flapVelocity = 1.0f; //What speed should a flap add?
		float airTime = 0.0f; //Time since last landed
		bool grounded = true;

		enum Status {
			Human, Cat
		};
		Status playerStatus = Cat;
		glm::vec3 humanAcc = glm::vec3(0.0f);
		WalkPoint walkpoint;
		glm::vec2 posDelt;
		int iter = 0;
	} player;



	struct State //Game state
	{
		int score = 0;
		float stablization = 1.0f;
		float time = 0.0f;
		float flapTimer = 0.00f;
		float flapCooldown = 0.1f;//In seconds


		enum PlayState {
			ongoing, won, lost, menu
		};
		PlayState playing = ongoing; 
		//Put order here
	};

	float gravity = -9.81f/2.f; //Likely should be lower to make more floaty

	State state;

	//Move
	struct Keys
	{
		bool space, up, down, left, right;
	};
	void updateCat(Keys keys, float elapsed, float gravity);



};
