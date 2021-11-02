#include "Mode.hpp"

#include "Scene.hpp"
#include "WalkMesh.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <random>

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

	float manager_next_appearance_timer = 1.0f; // seconds
	enum ManagerState {
		AWAY, 
		ARRIVING,
		HERE
	} manager_state = AWAY;
	float manager_stay_timer = 2.0f; // seconds

	Scene::Drawable* manager = NULL; // the drawable of the manager in scene.drawables
	glm::vec3 manager_here_pos; // the location to place the manager when it is HERE

	// ----- customers -----
	// 'customer_waypoints' contains the transforms of all of the "CustomerWaypoint" objects in the starbucks scene
	// When customers are instantiated, whey will automatically move towards a waypoint and then wait for their order to be 
	// taken there
	std::unordered_set<Scene::Transform> customer_waypoints;

	struct Customer {
		float max_order_wait_time = 10.0f; // seconds
		float t = 0.0f; // current wait time

		enum CustomerState {
			TRAVELING_TO_SEAT,
			WAITING_FOR_ORDER
		} state;
		
		// The customer's 'happiness' is just a gradient from 0 - 100, parameterized by 
		// the maximum order wait time and the current time. Feel free to change this to 
		// some other heuristic
		float happiness() {
			return 100.0f * (t - max_order_wait_time) / max_order_wait_time;
		}

		void update(float elapsed) {
			t += elapsed;
		}
	}

	// ----- input tracking -----
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
