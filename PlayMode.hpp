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

	float manager_next_appearance_timer = 1.0f; // seconds
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
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//player info:
	enum Status {
		Human, Cat
	};
	struct Player {
		Player() {}; // TODO: Implement this
		~Player() {}; // TODO: Implement this
		WalkPoint at;
		//transform is at player's feet and will be yawed by mouse left/right motion:
		Scene::Transform *transform = nullptr;
		//camera is at player's head and will be pitched by mouse up/down motion:
		Scene::Camera *camera = nullptr;
		Scene::Drawable *drawPlayer;
		
		Status playerStatus = Human;
		glm::vec3 curDir = glm::vec3(0.0f); //Direction player is currently facing, for cat (differnet then vel vec3)
		glm::vec3 catVelocity = glm::vec3(0.0f); //Current momentum, for cat, mass assumed (but could change in transition)
		glm::vec3 humanAcc = glm::vec3(0.0f); 
		//Note, velocity is added with each pump, and lost in y over time

		WalkPoint walkpoint;
		float height = 0.0f; //0 for human, > 0 for cat

		StarbuckItem cur_order;
		StarbuckItem bag;
	} player;

	enum PlayState {
		ongoing, won, lost, menu
	};
	struct State //Game state
	{
		int score = 0;	// player's total score until now
		int goal = 0;	// the goal score need to achieve before game_timer times out
		float stablization = 1.0f;
		float game_timer = 0.0f;
		const float day_period_time = 600.0f; // set 60s for a day in game, temporarily


		PlayState playing = ongoing; 
		//Put order here
	} game_state;

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


