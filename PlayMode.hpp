#pragma once
#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"
#include "Sound.hpp"
#include "catcoffee.hpp"
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

	float manager_next_appearance_timer = 45.0f; // seconds
	enum ManagerState {
		AWAY, 
		ARRIVING,
		HERE
	} manager_state = AWAY;
	float manager_stay_timer = 2.0f; // seconds
	std::shared_ptr< Sound::PlayingSample > manager_footstep_sfx;
	float manager_footstep_volume_max = 1.0f;
	float manager_footstep_volume_min = 0.1f;

	Scene::Drawable* manager = NULL; // the drawable of the manager in scene.drawables
	glm::vec3 manager_here_pos; // the location to place the manager when it is HERE

	// ----- input tracking -----
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up,space;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//player info:
	enum Status {
		Human, Cat, toCat, toHuman
	};
	struct Player {
		Player() {}; // TODO: Implement this
		~Player() {}; // TODO: Implement this
		WalkPoint at;
		//transform is at player's feet and will be yawed by mouse left/right motion:
		Scene::Transform *transform = nullptr;

		Scene::Drawable* cat;
		Scene::Drawable* human;
		void updateDrawable();

		//Camera is an orbit camera
		struct OrbitCamera {
			Scene::Camera* camera = nullptr;
			glm::vec3 focalPoint;
			glm::vec3 direction;
			float distance = 1.0f;

			void updateCamera(glm::vec3 newPos);
		};

		OrbitCamera orbitCamera;


		glm::vec3 catVelocity = glm::vec3(0.0f); //Current momentum, for cat, mass assumed (but could change in transition)11
		//Note, velocity is added with each pump, and lost in y over time
		float height = 0.0f; //0 for human, > 0 for cat
		float flapVelocity = 1.0f; //What speed should a flap add?
		float airTime = 0.0f; //Time since last landed
		bool grounded = true;

		Status playerStatus = Human;
		glm::vec3 humanAcc = glm::vec3(0.0f);
		WalkPoint walkpoint;
		glm::vec2 posDelt;
		int iter = 0;

		StarbuckItem cur_order;
		StarbuckItem bag;

		float fallTime = 0.f;
		glm::vec2 capturePos;


	} player;
	float gravity = -9.81f / 2.f;

	enum PlayState {
		ongoing, won, lost, menu
	};	


	struct State //Game state
	{
		int score = 0;	// player's total score until now
		int goal = 0;	// the goal score need to achieve before game_timer times out
		float stablization = 1.0f;
		float game_timer = 0.0f;
		const float day_period_time = 60.0f; // set 60s for a day in game, temporarily
		float time = 0.0f;
		float flapTimer = 0.00f;
		float flapCooldown = 0.1f;//In seconds
		PlayState playing = ongoing; 
	} ;

	//struct customers 
	struct Customer{
		//Todo wait_time scaling, happiness score
		int happiness = 0;
		int wait_time = 30;
		enum Status{
			New,
			Wait,
			Finished
		};
		std::string name;
		Scene::Transform* transform;
		Status status;
		StarbuckItem order;
		Customer(){}
		Customer(std::string nam,Scene::Transform* trans) : name(nam),transform(trans), status(Status::New){}
		
	};

	std::map<std::string, Scene::Transform*> ingredient_transforms;
	std::map<std::string, Customer> customers;

	State state;

	//Move
	struct Keys
	{
		bool space, up, down, left, right;
	};
	void updateCat(Keys keys, float elapsed, float gravity);
	void transition(float elapsed, float gravity);


	//order relevant
	enum OrderStatus{
		Empty,
		Executing,
		Finished
	};
	OrderStatus order_status = Empty;
	bool take_order();
	bool serve_order();
	bool grab_ingredient();
	std::string order_message;
};


