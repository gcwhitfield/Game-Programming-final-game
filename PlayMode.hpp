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

#define PI_F 3.1415926f

#include <unordered_set>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();
	

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	std::map<std::string, Scene::Drawable> ingredients_drawables;

	float manager_next_appearance_timer = 12.5f; // seconds

	enum ManagerState {
		AWAY, 
		ARRIVING,
		HERE
	} manager_state = AWAY;
	float manager_stay_timer = 5.5f; // seconds
	std::shared_ptr< Sound::PlayingSample > manager_footstep_sfx;
	float manager_footstep_volume_max = 3.0f;
	float manager_footstep_volume_min = 0.1f;

	Scene::Drawable* manager = NULL; // the drawable of the manager in scene.drawables
	glm::vec3 manager_here_pos; // the location to place the manager when it is HERE

	// ----- customers -----
	Scene::Drawable *customer_base; // the base object from which new customers will get cloned 

	// In starbucks.blend, there are various 'CustomerWaypoint' objects placed throughout the scene
	// when the value is set to 'true' it means that the waypoint is UNOCCUPIED and a customer caan
	// go to it.
	// when the value is set to 'false' it means that the waypoint is OCCUPIED and there is a customer using the
	// waypoint
	std::map<Scene::Transform*, bool> customer_waypoints;
	// std::list<Scene::Transform> customer_open_waypoints; // a set of transforms of unoccupied seats in starbucks
	// std::list<Scene::Transform> customer_occupied_waypoints; // a set of occupied seats in starbucks
	Scene::Transform* customer_spawn_point = NULL; // the place where new customers get spawned. This will eventually be outside the 
	// front door of the starbucks
	float customer_spawn_timer = 5.0f; // a new customer is spawned once this timer reaches 0.
	
	struct Customer{


		// initialize various variables related to animation
		void init() {
			t_new = 0.0f;
			t_wait = 0.0f;
			t_finished = 0.0f;
			status = Status::New;
		}
		
		//Todo max_wait_time scaling, happiness score
		int happiness() {
			return (int)(100.0f * (t_wait - max_wait_time) / max_wait_time);
		}

		// the customer travels towards the waypoint in 'New' state and away from the 
		// waypoint in 'Finished' state
		Scene::Transform *waypoint;

		// ----- New -----
		// the amount of time that it takes for the customer to arrive at their seat
		float new_animation_time = 2.0f; // in seconds
		float t_new = 0.0f; // current timestamp of New animation

		// ----- Wait -----
		float t_wait = 0; // the current amount of time that the customer has waited
		float max_wait_time = 45; // the maximum time that a customer will wait for an order
		
		// ----- Finished ----
		// the amount of time that it takes for the customer to fly away from their seat
		float finished_animation_time = 2.0f; // in seconds
		float t_finished = 0.0f; // cirrent timestamp of Finished animation
		
		enum Status{
			New,
			Wait,
			Finished,
			Inactive // state used for customers that are no longer in-play
		};
		std::string name;
		Scene::Transform* transform;
		Status status;
		StarbuckItem order;
		Customer(){}
		Customer(std::string nam,Scene::Transform* trans) : name(nam),transform(trans), status(Status::New){}
	};
	
	std::map<std::string, Customer> customers;

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
		WalkPoint outOfBounds;
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
			float distance = 8.0f;
			float truePitch = 75.0f*2*PI_F/360.f;
			float curPitch = 75.0f * 2 * PI_F / 360.f;

			void updateCamera();
			void walkCamera();
			WalkPoint at;
		};

		OrbitCamera orbitCamera;


		glm::vec3 catVelocity = glm::vec3(0.0f); //Current momentum, for cat, mass assumed (but could change in transition)11
		//Note, velocity is added with each pump, and lost in y over time
		float height = 0.0f; //0 for human, > 0 for cat
		float flapVelocity = 1.0f; //What speed should a flap add?
		float airTime = 0.0f; //Time since last landed
		bool grounded = true;
		bool lastCollision = false;
		bool firstHit = false;

		Status playerStatus = Human;
		glm::vec3 humanAcc = glm::vec3(0.0f);
		WalkPoint walkpoint;
		glm::vec2 posDelt;
		int iter = 0;

		StarbuckItem cur_order;
		StarbuckItem bag;
		std::string cur_customer = std::string("");

		float fallTime = 0.f;
		glm::vec2 capturePos;


	} player;
	float gravity = -9.81f / 2.f;
	void decidePos(glm::vec3 bounds, glm::vec3 at);
	void getBoundedPos(glm::vec2 move, WalkMesh const* boundWalkmesh, WalkMesh const* walkmesh);
	float objectHeight = 3.0f;

	enum PlayState {
		ongoing, won, lost, menu
	};	

	enum Proximity {
		CustomerProx, IngredientProx, NoProx
	};


	struct State //Game state
	{
		int score = 0;	// player's total score until now
		int goal = 0;	// the goal score need to achieve before game_timer times out
		float stablization = 1.0f;
		float game_timer = 0.0f;
		const float day_period_time = 300.0f; // 300s for vertical slice demo
		float time = 0.0f;
		float flapTimer = 0.0f;
		float flapCooldown = 0.1f;//In seconds
		float catchTimer = 0.0f; //time for the deduction of cat caught by the manager 
		PlayState playing = ongoing; 
		Proximity proximity = NoProx;
	} ;

	void updateProximity();


	std::map<std::string, Scene::Transform*> ingredient_transforms;

	State state;

	//Move
	struct Keys
	{
		bool space, up, down, left, right;
	};
	void updateCat(Keys keys, float elapsed, float gravity);
	void transition(float elapsed, float gravity, WalkMesh const* boundWalkmesh, WalkMesh const* walkmesh);


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
	std::string catch_message;
	std::string closest_customer_name;
	std::string closest_ingredient_name;
};


