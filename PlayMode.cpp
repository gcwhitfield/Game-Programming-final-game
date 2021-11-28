#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <iostream>
#include <algorithm>
#define ERROR_F 0.000001f
#define HEIGHT_CLIP 0.255f

// -----------------------------------
// ---------- Asset Loading ----------
// -----------------------------------
GLuint starbucks_meshes_for_lit_color_texture_program = 0;
Load<MeshBuffer> starbucks_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("starbucks.pnct"));
	starbucks_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load<Scene> starbucks_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("starbucks.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name) {
		Mesh const &mesh = starbucks_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = starbucks_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

WalkMesh const *walkmesh = nullptr;
WalkMesh const *boundWalkmesh = nullptr;
Load<WalkMeshes> phonebank_walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
	WalkMeshes *ret = new WalkMeshes(data_path("starbucks.w"));
	walkmesh = &ret->lookup("WalkMesh");
	boundWalkmesh = &ret->lookup("BoundsWalkMesh");
	return ret;
});

// cite: https://www.fesliyanstudios.com/royalty-free-sound-effects-download/footsteps-31
Load<Sound::Sample> manager_footstep_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("manager_footstep.wav"));
});

Load<Sound::Sample> order_complete_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("BellDing.wav"));
});

Load<Sound::Sample> background_music_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("[Loop]WelcomeToStarbucks.wav"));
});

Load<Sound::Sample> mmm1_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("mmm1.wav"));
});

Load<Sound::Sample> mmm2_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("mmm2.wav"));
});

Load<Sound::Sample> slurp_ahhh_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("slurp_ahhh.wav"));
});

Load<Sound::Sample> sip_ahhh_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("sip_ahhh.wav"));
});

// -----------------------------------
// ---- Small Helper Functions ----
// -----------------------------------
//generate a new_item from the item list randomly
std::pair<std::string, StarbuckItem> new_item()
{
	std::random_device r;
	std::default_random_engine e1(r());
	size_t sz = RotationList.size() - 1;
	std::uniform_int_distribution<int> uniform_dist(0, static_cast<int>(sz));
	int choice = uniform_dist(e1);
	auto it = RotationList.begin();
	while (choice--)
		it++;
	return *it;
}
std::string PlayMode::new_customer_name()
{
	std::random_device r;
	std::default_random_engine e1(r());
	size_t sz = customernames.size() - 1;
	std::uniform_int_distribution<int> uniform_dist(0, static_cast<int>(sz));
	int choice = uniform_dist(e1);

	// only generate new customer names
	while (customers.find(customernames[choice]) != customers.end())
	{
		choice = uniform_dist(e1);
	}

	return customernames[choice];
}
//debugging printing information for recipe
std::ostream &operator<<(std::ostream &os, const glm::vec3 &pos)
{
	return os << '(' << pos.x << ' ' << pos.y << ' ' << pos.z << ')';
}
std::ostream &operator<<(std::ostream &os, const StarbuckItem &item)
{
	for (auto &ingredient : item.recipe)
	{
		os << "|" << ingredient.first << "|" << std::endl;
	}
	return os;
}
std::ostream &operator<<(std::ostream &os, const PlayMode::Customer &customer)
{
	os << "item name : " << customer.order.item_name;
	os << ",   status : ";
	if (customer.status == PlayMode::Customer::Status::Finished)
		os << "Finished";
	if (customer.status == PlayMode::Customer::Status::New)
		os << "New";
	if (customer.status == PlayMode::Customer::Status::Wait)
		os << "Wait";
	if (customer.status == PlayMode::Customer::Status::Inactive)
		os << "Inactive";
	return os;
}
bool collide(Scene::Transform *trans_a, Scene::Transform *trans_b, float radius = 10.0f)
{
	auto a_pos = trans_a->position;
	auto b_pos = trans_b->position;
	return distance2(a_pos, b_pos) < radius;
}


PlayMode::PlayMode(int level) : scene(*starbucks_scene), draw_text(data_path("fonts/quicksilver_3/Quicksilver.ttf"))
{
	//create a player transform:
	scene.transforms.emplace_back();
	player.transform = &scene.transforms.back();
	player.transform->position = glm::vec3(-6.0f, 0.0f, 0.1f);

	//create a player camera attached to a child of the player transform:
	scene.transforms.emplace_back();
	scene.cameras.emplace_back(&scene.transforms.back());
	player.orbitCamera.camera = &scene.cameras.back();
	player.orbitCamera.camera->fovy = glm::radians(60.0f);
	player.orbitCamera.camera->near = 0.01f;
	player.orbitCamera.camera->transform->parent = player.transform;

	//player's eyes are 1.8 units above the ground:
	player.orbitCamera.camera->transform->position = glm::vec3(0.0f, 0.0f, 1.8f);

	//rotate camera facing direction (-z) to player facing direction (+y):
	player.orbitCamera.camera->transform->rotation = glm::angleAxis(glm::radians(75.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	player.orbitCamera.updateCamera();

	//start player walking at nearest walk point:
	player.at = walkmesh->nearest_walk_point(player.transform->position);
	player.outOfBounds = boundWalkmesh->nearest_walk_point(player.transform->position);

	for (auto &d : scene.drawables)
	{
		std::string &str = d.transform->name;
		//std::cout <<str <<std::endl;
		if (str == "Manager")
		{
			manager = &d;
			manager_here_pos = d.transform->position;
		}
		else if (str == "Human")
		{
			player.human = &d;
		}
		else if (str == "Cat")
		{
			player.cat = &d;
		}
		//store ingredients information and location
		else if (ingredients.find(str) != ingredients.end())
		{
			ingredient_transforms[str] = d.transform;
		}
		// add the "CustomerWaypoint" transforms from the starbucks.blend scene into a
		// vector
		else if (str.find("CustomerWaypoint") < str.size() && str != "Customer")
		{
			assert(str != "CustomerBase");
			assert(str != "Customer");
			assert(str != "CustomerSpawnPoint");
			std::pair<Scene::Transform *, bool> p = std::make_pair(d.transform, true);
			customer_waypoints.insert(p);
			d.pipeline.count = 0; // do not draw customer waypoints
		}
		else if (str == "CustomerSpawnPoint" && str != "CustomerBase")
		{
			assert(str == "CustomerSpawnPoint");
			//std::cout << "CustomerSpawnPoint has been found" << std::endl;
			customer_spawn_point = d.transform;
			d.pipeline.count = 0; // do not draw customer spawn points
		}
		//store customers ingredients information and location
		else if (str.length() >= 8 && str.substr(0, 8) == "Customer" && str.find("CustomerBase") >= str.size() && str.find("CustomerWaypoint") > str.size() && str != "CustomerSpawnPoint")
		{
			//std::cout << str << std::endl;
			assert(str != "CustomerBase");
			assert(str != "CustomerWaypoint");
			assert(str != "CustomerSpawnPoint");
			auto Cu = Customer(new_customer_name(), d.transform, this->day_index);
			Cu.order = new_item().second;
			customers[str] = Cu;
		}
		// set the "CustomerBase" and "CustomerSpawnPoint" transforms to their corresponding
		// transforms in the starbucks.blend scene
		else if (str.find("CustomerBase") < str.size())
		{
			std::cout << "CustomerBase '" << str << "' has been found!" << std::endl;
			customer_base.push_back(&d);
		}
		// the player starts at the location of the "Player" object in the Blender scene
		else if (str == "Player")
		{
			assert(str == "Player");
			// std::cout << "Player transform has been found in the blender scene" << std::endl;
			player.transform->position = d.transform->position;
			d.transform->position.y = 100000;
		}
	}

	// give the customers in the scene a waypoint
	for (auto &[name, customer] : customers)
	{
		for (auto &[waypoint, is_open] : customer_waypoints)
		{
			if (is_open)
			{
				customer.waypoint = waypoint;
				is_open = false;
				assert(customer_waypoints[waypoint] == false);
				break;
			}
		}
	}

	// PARANOIA none of the customers should have null waypoint
	for (auto &[name, customer] : customers)
	{
		assert(customer.waypoint != NULL);
	}

	for (auto &[w, is_open] : customer_waypoints)
	{
		(void)is_open; // avoid 'variable not used' with is_open
					   //std::cout << "Cusotmer waypoint: " << w->position << std::endl;
	}

	assert(customer_waypoints.size() > 0);
	assert(customer_base.size() > 0);
	assert(customer_spawn_point != NULL);
	assert(manager != NULL);

	assert(player.cat);
	assert(player.cat->transform);
	//assert(player.cat->transform->parent);
	assert(player.human);
	assert(player.human->transform);
	//assert(player.human->transform->parent);

	// set cat/human transform parent
	player.cat->transform->parent = player.transform;
	player.human->transform->parent = player.transform;
	player.cat->transform->position = glm::vec3(0.0f, 0.0f, 0.44f);
	player.human->transform->position = glm::vec3(0.0f, 0.0f, 1.34f);

	/* Different level has different goal and day time*/
	this->day_index = level;
	state.day_period_time = std::min(300.0f, state.day_period_time + (float)(level) * 40.0f) * 100;
	// initialize timer
	state.game_timer = state.day_period_time;
	// mechanism of setting revenue goal
	state.goal = std::min(level * 2 * 50, 1000);
	manager_appearance_frequency = std::max(30.0f,(float)(130.0f - (float)level * 2));
	manager_next_appearance_timer = manager_appearance_frequency;

	//Orders
	player.bag.item_name = "bag";
	/*for (auto& light : scene.lights) {
		std::cout << light.energy.x << " " << light.energy.y << " " << light.energy.z << std::endl;
	}*/

	Sound::loop(*background_music_sample);
}

PlayMode::~PlayMode()
{
}

//Order Related Function
bool PlayMode::take_order()
{
	//printf("freak0, %zu\n", customers.size());
	std::map<std::string, float> close_customers;
	for (auto &[name, customer] : customers)
	{
		if (collide(customer.transform, player.transform) && // distance close
			customer.status == Customer::Status::New &&		 // customer is new, has not given order yet
			order_status == OrderStatus::Empty)				 //player does not have order in hand
			close_customers.insert(std::make_pair(name, distance2(customer.transform->position, player.transform->position)));
	}

	Customer closest_customer = customers[
		std::min_element(close_customers.begin(), close_customers.end(), 
			[] (const std::pair<std::string, float> c1, const std::pair<std::string, float> c2) {
				return c1.second < c2.second; 
			})->first
		]; 

	{ //take the order
		player.cur_order = closest_customer.order;
		player.cur_customer = closest_customer.name;
		order_status = OrderStatus::Executing;
		customers[closest_customer.name].status = Customer::Status::Wait;
		order_message = std::string("Taking order ...... : ") + closest_customer.name + ", " + closest_customer.order.item_name + "!";
		return true;
	}
	return false;
}
bool PlayMode::grab_ingredient()
{
	for (auto &[name, ingredient_transform] : ingredient_transforms)
	{
		if (collide(ingredient_transform, player.transform,5.0f) && // distance close
			order_status == OrderStatus::Executing				  //player has an order in hand
		)
		{
			if (player.bag.add_item(name) != 0)
			{
				player.bag.remove_item(name);
			}
		}
	}
	return false;
}

bool PlayMode::serve_order()
{
	//std::cout<<"name:"<<player.cur_order.item_name<<std::endl;
	for (auto &[name, customer] : customers)
	{
		//std::cout<<customer<<std::endl;
		if (collide(customer.transform, player.transform, 10.0f)) // distance close
		{
			if (customer.status == Customer::Status::Wait)
			{																  //customer is waiting
				if (customer.order.item_name == player.cur_order.item_name && //the order match
					player.cur_order == player.bag							  // actually has all the correct ingredient
				)
				{
					player.cur_order = StarbuckItem(); // empty the order
					customer.status = Customer::Status::Finished;
					order_status = OrderStatus::Empty;
					order_message = std::string("Succeeded in serving : ") + customer.order.item_name + "!";
					// increase score
					auto dist = sqrt(distance2(glm::vec3(-6.0f, 0.0f, customer.transform->position.z),customer.transform->position));
					// std::cout << dist << std::endl;
					state.score += (int)(50.0f * dist / (player.PlayerSpeed * 4.0f));
					if(player.playerStatus == Cat) state.score = (int)((float)state.score * 1.1f);
					// clear player bag, because order is served
					player.bag.clear_item();
					// also clear the last served order
					player.cur_order.clear_item();
					player.cur_customer = std::string("");

					Sound::play(*order_complete_sample, 0.5f);
					play_color_explosion(customer.transform->position);

					// play a random sound from the "customer order served" sounds
					std::vector<Load<Sound::Sample>> customer_order_samples = {
						mmm1_sample, mmm2_sample, slurp_ahhh_sample, sip_ahhh_sample
					};
					size_t r = rand() % customer_order_samples.size(); // index of random sample
					assert(r < customer_order_samples.size());
					Sound::play(*(customer_order_samples[r]));

					return true;
				}
			}
		}
	}
	return false;
}

void PlayMode::Player::OrbitCamera::updateCamera()
{
	focalPoint = glm::vec3(0.0f);
	direction = glm::normalize(camera->transform->rotation * glm::vec3(0.0f, 0.0f, -1.0f));
	camera->transform->position = focalPoint - distance * direction;
	camera->transform->rotation = -camera->transform->rotation;
}

void PlayMode::Player::OrbitCamera::walkCamera()
{
	glm::vec3 worldPos = camera->transform->make_local_to_world() * glm::vec4(camera->transform->position.x, camera->transform->position.y, camera->transform->position.z, 1.f);
	worldPos.z = 0.0f;
	at = boundWalkmesh->nearest_walk_point(worldPos);
	glm::vec3 inBounds = boundWalkmesh->to_world_point(at);
	float curLength = length(inBounds - worldPos);
	if (curLength > 0.0001f)
	{
		float height = camera->transform->position.z;
		camera->transform->position = focalPoint - (distance - curLength) * direction;
		camera->transform->position.z = height;
	}
}

void PlayMode::updateProximity()
{
	auto getDistance = [](Scene::Transform *a, Scene::Transform *b) {
		return glm::length(a->position - b->position);
	};
	std::pair<bool, float> closestC, closestI;
	closestC = std::make_pair(false, INFINITY);
	closestI = std::make_pair(false, INFINITY);
	float orderCustDist = 0.0f;

	//customer_waypoints
	for (auto &[name, customer] : customers)
	{
		if (name == player.cur_customer && !player.bag.recipe.empty()) {
			orderCustDist = getDistance(customer.transform, player.transform);
		}
		if (collide(customer.transform, player.transform, 20.0f))
		{
			float dist = getDistance(customer.transform, player.transform);
			if (!closestC.first || dist < closestC.second)
			{
				closestC = std::make_pair(true, dist);
				closest_customer_name = customer.name;
			}
		}
	}
	if (orderCustDist > 13.5f) {
		for (auto& [waypoint, filled] : customer_waypoints) {
			if (collide(waypoint, player.transform,7.5f)) {
				float dist = getDistance(waypoint, player.transform);
				if ((player.playerStatus != PlayMode::Status::Cat && dist < 5.5f) || 
				    (player.playerStatus == PlayMode::Status::Cat && player.lastCollision == true)) //spill the coffee
				{
					catch_message = "Spilt the coffee! You ran into the wrong customer!";
					player.bag.clear_item();
					state.score -= 15;
					state.score = std::max(state.score, 0);
					//todo Sound::play messed up
				}
			}
		}
	}
	//int cnt = 0;
	for (auto &[name, ingredient_transform] : ingredient_transforms)
	{
		// cnt++;
		if (collide(ingredient_transform, player.transform,5.0f))
		{
			float dist = getDistance(ingredient_transform, player.transform);
			if (!closestI.first || dist < closestI.second)
			{
				closest_ingredient_name = ingredient_transform->name;
				closestI = std::make_pair(true, dist);
			}
		}
	}
	//std::cout<<cnt<<' '<<closestC.second<<' '<<closestI.second<<std::endl;
	if (!(closestC.first || closestI.first))
		state.proximity = Proximity::NoProx;
	else if (!closestC.first || closestI.second < closestC.second)
		state.proximity = Proximity::IngredientProx;
	else
		state.proximity = Proximity::CustomerProx;
}

void PlayMode::play_color_explosion(glm::vec3 location)
{
	color_explosion_timer = 0.0f;
	color_explosion_location = location;
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{
	/*z take order, x serve order, c copy ingredient*/
	if (evt.type == SDL_KEYDOWN)
	{
		if (evt.key.keysym.sym == SDLK_ESCAPE)
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a)
		{
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			if (state.flapTimer > state.flapCooldown && !space.pressed)
			{
				space.downs += 1;
				space.pressed = true;
				state.flapTimer = 0.0f;
			}
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_r)
		{
			if(state.playing == lost){
				Sound::stop_all_samples();
				Mode::set_current(std::make_shared<PlayMode>(1));
			}
			else if (state.playing == won){
				Sound::stop_all_samples();
				Mode::set_current(std::make_shared<PlayMode>(day_index + 1));
			}
				
            return true;
		}
	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.sym == SDLK_a)
		{
			left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			right.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			space.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_1) // print debug stuff
		{
			// print customer data
			std::cout << "Customers" << std::endl;
			std::cout << "Num customers: " << customers.size();
			for (auto &[name, customer] : customers)
			{
				std::cout << name << " " << glm::to_string(customer.transform->position) << std::endl << std::endl;
			}
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN)
	{
		if (SDL_GetRelativeMouseMode() == SDL_FALSE)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
		if (evt.button.button == SDL_BUTTON_LEFT)
		{
			switch (state.proximity)
			{

			case (Proximity::CustomerProx):
				if (order_status == OrderStatus::Empty)
				{
					take_order();
				}

				else if (order_status == OrderStatus::Executing)
				{
					serve_order();
				}

				else 
				{
					std::cerr << "INVALID ORDER STATUS" << std::endl;
					throw;
				}
				break;
			case (Proximity::IngredientProx):
			{
				grab_ingredient();
			}

			break;
			default:
				break;
			}
			return true;
		}
		else if (evt.button.button == SDL_BUTTON_RIGHT)
		{
			if (player.playerStatus == Cat)
			{
				player.playerStatus = toHuman;
				player.capturePos = glm::vec2(player.transform->position.x, player.transform->position.y);
			}
			else if (player.playerStatus == Human)
				player.playerStatus = toCat;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEMOTION)
	{
		if (SDL_GetRelativeMouseMode() == SDL_TRUE)
		{
			SDL_WarpMouseGlobal(window_size.x / 2, window_size.y / 2); //Allows moust to not get caught on window edge
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y));
			glm::vec3 up = walkmesh->to_world_smooth_normal(player.at);
			player.transform->rotation = glm::angleAxis(-motion.x * player.orbitCamera.camera->fovy, up) * player.transform->rotation;
			player.orbitCamera.truePitch += motion.y * player.orbitCamera.camera->fovy;
			//camera looks down -z (basically at the player's feet) when pitch is at zero.
			player.orbitCamera.truePitch = std::min(player.orbitCamera.truePitch, 0.95f * 3.1415926f);
			player.orbitCamera.truePitch = std::max(player.orbitCamera.truePitch, 0.05f * 3.1415926f);
			player.orbitCamera.curPitch = player.orbitCamera.truePitch;
			if (player.height <= player.orbitCamera.distance + ERROR_F)
			{ //Camera clipping check
				//Get minimum angle
				float theta = asin(player.height / player.orbitCamera.distance);
				if (player.orbitCamera.curPitch >= PI_F / 2.f + theta - ERROR_F)
				{ //If at or above minimum angle, cap so can't see below walkmesh
					player.orbitCamera.curPitch = PI_F / 2.f + theta;
				}
			}
			player.orbitCamera.camera->transform->rotation = glm::angleAxis(player.orbitCamera.curPitch, glm::vec3(1.0f, 0.0f, 0.0f));

			player.orbitCamera.updateCamera();

			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed)
{

	assert(player.cat);
	assert(player.cat->transform);
	assert(player.human);
	assert(player.human->transform);

	//Camera update

	if (player.height <= player.orbitCamera.distance + ERROR_F)
	{ //Camera clipping check
		//Get minimum angle
		float theta = asin(player.height / player.orbitCamera.distance);
		if (player.orbitCamera.truePitch >= PI_F / 2.f + theta - ERROR_F - 0.1f)
		{
			player.orbitCamera.curPitch = PI_F / 2.f + theta - 0.1f;
			player.orbitCamera.camera->transform->rotation = glm::angleAxis(player.orbitCamera.curPitch, glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}

	player.orbitCamera.updateCamera();
	player.orbitCamera.walkCamera();

	// win and lose
	if (state.playing == won || state.playing == lost)
	{
		return;
	}
	// global timer count down
	state.game_timer -= elapsed;
	// win loss condition
	if (state.game_timer <= 0.0f)
	{
		if (state.score >= state.goal)
		{
			state.playing = won;
		}
		else
		{
			state.playing = lost;
		}
		return;
	}
	else
	{
		if (manager_state == HERE && player.playerStatus == Cat)
		{
			catch_message = "You were caught by the Manager!";
			state.catchTimer += elapsed;
			if (state.catchTimer > 0.25f)
			{
				state.catchTimer = 0.0f;
				if (state.score > 0)
				{
					state.score -= 1;
				}
			}

			state.playing = lost;
			return;
		}
	}

	state.flapTimer += elapsed;

	//player walking:
	glm::vec2 move = glm::vec2(0.0f);
	//combine inputs into a move:
	
	if (left.pressed && !right.pressed)
		move.x = -1.0f;
	if (!left.pressed && right.pressed)
		move.x = 1.0f;
	if (down.pressed && !up.pressed)
		move.y = -1.0f;
	if (!down.pressed && up.pressed)
		move.y = 1.0f;

	//make it so that moving diagonally doesn't go faster:
	if (move != glm::vec2(0.0f))
		move = glm::normalize(move) * player.PlayerSpeed * elapsed;

	if (player.playerStatus != toCat && player.playerStatus != toHuman)
	{

		if (player.playerStatus == Cat)
		{ //If cat, get move from updateCat if in flight
			Keys sendKeys;
			sendKeys.space = (space.downs > 0);
			sendKeys.up = (up.pressed);
			sendKeys.down = (down.pressed);
			sendKeys.left = (left.pressed);
			sendKeys.right = (right.pressed);
			updateCat(sendKeys, elapsed, gravity);
			if (!player.grounded)
				move = player.posDelt;
		}

		//get move in world coordinate system:
		glm::vec3 remain = player.transform->make_local_to_world() * glm::vec4(move.x, move.y, 0.0f, 0.0f);

		//using a for() instead of a while() here so that if walkpoint gets stuck in
		// some awkward case, code will not infinite loop:
		for (uint32_t iter = 0; iter < 10; ++iter)
		{
			if (remain == glm::vec3(0.0f))
			{
				break;
			}
			WalkPoint end;
			float time;
			walkmesh->walk_in_triangle(player.at, remain, &end, &time);
			player.at = end;
			if (time == 1.0f)
			{
				//finished within triangle:
				remain = glm::vec3(0.0f);
				break;
			}
			//some step remains:
			remain *= (1.0f - time);
			//try to step over edge:
			glm::quat rotation;
			if (walkmesh->cross_edge(player.at, &end, &rotation))
			{
				//stepped to a new triangle:
				player.at = end;
				//rotate step to follow surface:
				remain = rotation * remain;
			}
			else
			{
				//ran into a wall, bounce / slide along it:
				glm::vec3 const &a = walkmesh->vertices[player.at.indices.x];
				glm::vec3 const &b = walkmesh->vertices[player.at.indices.y];
				glm::vec3 const &c = walkmesh->vertices[player.at.indices.z];
				glm::vec3 along = glm::normalize(b - a);
				glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
				glm::vec3 in = glm::cross(normal, along);

				//check how much 'remain' is pointing out of the triangle:
				float d = glm::dot(remain, in);
				if (d < 0.0f)
				{
					//bounce off of the wall:
					remain += (-1.25f * d) * in;
				}
				else
				{
					//if it's just pointing along the edge, bend slightly away from wall:
					remain += 0.01f * d * in;
				}
			}
		}

		if (remain != glm::vec3(0.0f))
		{
			//std::cout << "NOTE: code used full iteration budget for walking." << std::endl;
		}

		//update player's position to respect walking if human:
		if (player.playerStatus != Cat)
			player.transform->position = walkmesh->to_world_point(player.at);
		else //Extra steps to account for cat collision
			getBoundedPos(move, boundWalkmesh, walkmesh);

		{ //update player's rotation to respect local (smooth) up-vector:

			glm::quat adjust = glm::rotation(
				player.transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f), //current up vector
				walkmesh->to_world_smooth_normal(player.at)				  //smoothed up vector at walk location
			);
			player.transform->rotation = glm::normalize(adjust * player.transform->rotation);
		}
		player.transform->position += glm::vec3(0.0f, 0.0f, player.height + HEIGHT_CLIP);

		/*
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
		*/
	}
	else
	{
		transition(elapsed, 2 * gravity, boundWalkmesh, walkmesh);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	// the manager appears periodically
	{
		switch (manager_state)
		{
		case AWAY:
		{
			manager_next_appearance_timer -= elapsed;
			manager->transform->position = glm::vec3(0, 0, -10000); // move the manager super far away when it's AWAY
			if (manager_next_appearance_timer < 5.0f)
			{
				manager_state = ARRIVING;
				// Play manager footstep sound queue
				manager_footstep_sfx = Sound::loop(*manager_footstep_sample, manager_footstep_volume_min, 0.0f);
				// play sound queue 3 seconds before the manager appears
			}
		}
		break;
		case ARRIVING:
		{
			manager_next_appearance_timer -= elapsed;
			if (manager_next_appearance_timer < 0)
			{
				
				manager_state = HERE;
				// stop manager footstep sfx
				manager_footstep_sfx->stop();
			}
			else
			{
				// volume of footstep gradually increase when the manager approach
				float k = -(manager_footstep_volume_max - manager_footstep_volume_min) / 3.0f;
				float new_volume = k * manager_next_appearance_timer + manager_footstep_volume_max;
				manager_footstep_sfx->set_volume(new_volume);
			}
		}
		break;
		case HERE:
		{
			manager->transform->position = manager_here_pos; // move the manager here when it is HERE
			manager_stay_timer -= elapsed;
			if (manager_stay_timer < 0)
			{
				manager_stay_timer = 3.5f;
				// set manager_next_appearance_timer to a random time between 7.5 and 15 seconds
				size_t r = rand() % 100;
				manager_next_appearance_timer = -5.0f + 10.0f * (r / (float)100) + manager_appearance_frequency;
				manager_state = AWAY;
			}
		}
		break;
		}
	}

	{ // spawn new customers periodically
		customer_spawn_timer -= elapsed;
		if (customer_spawn_timer < 0)
		{
			//std::cout << "A new customer has been spawned!" << std::endl;
			size_t r = rand() % 100;
			// the amount of time until the next customer spawns is governed by the
			// random variable 'rand_time'. 'rand_time' is uniformly distributed
			// between 7 and 17 seconds
			float rand_time = 7 + 10.0f * (r / (float)100);
			customer_spawn_timer = rand_time;
			scene.transforms.emplace_back();
			scene.drawables.emplace_back(&scene.transforms.back());
			Scene::Drawable *new_customer = &scene.drawables.back();

			// choose a random customer base from the list of customer bases
			r = rand() % customer_base.size();
			new_customer->pipeline = customer_base[r]->pipeline;

			// set the location of the customer, init data
			new_customer->transform->position = customer_spawn_point->position;
			//std::string new_customer_name = "Customer" + std::to_string(customers.size() + 1);
			Customer c = Customer(new_customer_name(), new_customer->transform, this->day_index);
			c.order = new_item().second;
			c.init();
			// std::cout << "max wait time:" << c.max_wait_time << std::endl;
			// give the customer a waypoint from one of the open waypoint
			bool has_set_cwaypoint = false;
			for (auto &[waypoint, is_open] : customer_waypoints)
			{
				if (is_open)
				{
					c.waypoint = waypoint;
					is_open = false;
					has_set_cwaypoint = true;
					break;
				}
			}
			if (!has_set_cwaypoint)
			{
				std::cerr << "All of the waypoints are full, could not find waypoint for customer "
							 "As a workaround, this customer's waypoint will be set to the origin"
						  << std::endl;
				c.waypoint = new Scene::Transform();
				c.waypoint->position = glm::vec3(0, 0, 0);
				c.waypoint->rotation = glm::vec3(0, 0, 0);
				c.waypoint->scale = glm::vec3(1, 1, 1);
			}

			customers[c.name] = c;
			assert(c.waypoint != NULL);
		}
	}

	{ // handle customer behaviour depending on state
		for (auto &[name, customer] : customers)
		{
			switch (customer.status)
			{
			case Customer::Status::New:
			{
				// customers fly into their seat in the beginning of 'New' state
				if (customer.t_new < customer.new_animation_time)
				{
					// std::cout << "New" << std::endl;
					customer.t_new += elapsed;
					float t = (customer.new_animation_time - customer.t_new) / customer.new_animation_time;
					assert(customer.transform != NULL);
					assert(customer_spawn_point != NULL);
					assert(customer.waypoint != NULL);
					customer.transform->position = customer_spawn_point->position * t + (customer.waypoint->position * (1.0f - t));
				}
			}
			break;
			// customers wait for their order in 'Wait' state
			case Customer::Status::Wait:
			{
				customer.t_wait += elapsed;
				customer.transform->position = customer.waypoint->position;
				// the customer gets angry if it waits too longs, score gets deducted
				if (customer.t_wait > customer.max_wait_time)
				{
					catch_message = std::string("Customer [") + customer.name + "] has waited too long :( and left!";
					//std::cout << "Customer [" << customer.name << "] has waited too long :(. Customer is leaving..." << std::endl;
					state.score -= std::min(50.0f, (float)this->day_index * 10.0f);
					state.score = std::max(state.score, 0);
					customer.status = Customer::Status::Finished;
					player.bag.clear_item();
					player.cur_order.clear_item();
					player.cur_customer = std::string("");
					order_status = OrderStatus::Empty;
				}
			}
			break;
			// customers fly away in 'Finished' state
			case Customer::Status::Finished:
			{
				customer.t_finished += elapsed;
				float t = (customer.finished_animation_time - customer.t_finished) / customer.finished_animation_time;
				glm::vec3 desired_position = customer_spawn_point->position;
				desired_position.y += 50.0f;
				customer.transform->position = customer_spawn_point->position * (1.0f - t) + (desired_position * t);
				if (customer.t_finished > customer.finished_animation_time)
				{
					customer.transform->position.x = 10000; // move the customer super far away
					customer_waypoints[customer.waypoint] = true;
					customer.status = Customer::Status::Inactive;
				}
			}
			break;
			case Customer::Status::Inactive:
			{
				// do nothing
			}
			}
		}
	}

	space.downs = 0;

	//update visability of cat and human
	player.updateDrawable();
	updateProximity(); //Update nearest action for next control event

	//update color explosion effect
	if (color_explosion_timer < color_explosion_anim_time) {
		color_explosion_timer += elapsed;
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{

	//update camera aspect ratio for drawable:
	player.orbitCamera.camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	glUseProgram(lit_color_texture_program->program);

	//Load light arrays
	//Code modified from https://github.com/15-466/15-466-f19-base6/blob/master/DemoLightingForwardMode.cpp

	uint32_t lightCount = std::min<uint32_t>((uint32_t)scene.lights.size(), lit_color_texture_program->maxLights);

	std::vector<int32_t> light_type;
	light_type.reserve(lightCount);
	std::vector<glm::vec3> light_location;
	light_location.reserve(lightCount);
	std::vector<glm::vec3> light_direction;
	light_direction.reserve(lightCount);
	std::vector<glm::vec3> light_energy;
	light_energy.reserve(lightCount);
	std::vector<float> light_cutoff;
	light_cutoff.reserve(lightCount);

	for (auto const &light : scene.lights)
	{
		glm::mat4 light_to_world = light.transform->make_local_to_world();
		//set up lighting information for this light:
		light_location.emplace_back(glm::vec3(light_to_world[3]));
		light_direction.emplace_back(glm::vec3(-light_to_world[2]));
		light_energy.emplace_back(light.energy);

		if (light.type == Scene::Light::Point)
		{
			light_type.emplace_back(0);
			light_cutoff.emplace_back(1.0f);
		}
		else if (light.type == Scene::Light::Hemisphere)
		{
			light_type.emplace_back(1);
			light_cutoff.emplace_back(1.0f);
		}
		else if (light.type == Scene::Light::Spot)
		{
			light_type.emplace_back(2);
			light_cutoff.emplace_back(std::cos(0.5f * light.spot_fov));
		}
		else if (light.type == Scene::Light::Directional)
		{
			light_type.emplace_back(3);
			light_cutoff.emplace_back(1.0f);
		}

		//skip remaining lights if maximum light count reached:
		if (light_type.size() == lightCount)
			break;
	}

	glUniform1ui(lit_color_texture_program->LIGHT_COUNT_uint, lightCount);

	GL_ERRORS();
	glUniform1f(lit_color_texture_program->LIGHT_COUNT_float, (float)lightCount);

	GL_ERRORS();

	glUniform1iv(lit_color_texture_program->LIGHT_TYPE_int_array, lightCount, light_type.data());
	glUniform3fv(lit_color_texture_program->LIGHT_LOCATION_vec3_array, lightCount, glm::value_ptr(light_location[0]));
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3_array, lightCount, glm::value_ptr(light_direction[0]));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3_array, lightCount, glm::value_ptr(light_energy[0]));
	glUniform1fv(lit_color_texture_program->LIGHT_CUTOFF_float_array, lightCount, light_cutoff.data());

	// set uniforms for color explosion effect 
	glUniform3fv(lit_color_texture_program->COLOR_EXPLOSION_ORIGIN_vec3, 1, glm::value_ptr(player.transform->position));
	color_explosion_timer_normalized = color_explosion_timer / color_explosion_anim_time;
	glUniform1f(lit_color_texture_program->COLOR_EXPLOSION_T_float, color_explosion_timer_normalized);

	GL_ERRORS();

	glUseProgram(0);

	glClearColor(37/255.0, 25/255.0, 12/255.0, 0/255.0); // brown background color
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*player.orbitCamera.camera);
	GL_ERRORS();

	/*//In case you are wondering if your walkmesh is lining up with your scene, try:
	{
		glDisable(GL_DEPTH_TEST);
		DrawLines lines(player.orbitCamera.camera->make_projection() * glm::mat4(player.orbitCamera.camera->transform->make_world_to_local()));
		for (auto const &tri : boundWalkmesh->triangles) {
			lines.draw(boundWalkmesh->vertices[tri.x], boundWalkmesh->vertices[tri.y], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
			lines.draw(boundWalkmesh->vertices[tri.y], boundWalkmesh->vertices[tri.z], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
			lines.draw(boundWalkmesh->vertices[tri.z], boundWalkmesh->vertices[tri.x], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
		}
	}*/

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		float ofs = 2.0f / drawable_size.y;
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f));

		constexpr float H = 0.09f;

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
		auto draw_item = [&](StarbuckItem item, glm::vec3 pos, float sz = 0.10f) {
			if (item.item_name == "bag")
			{
				draw_text(std::string("Bag : "),
						  glm::vec3(-aspect + pos.x, pos.y, pos.z),
						  sz);
			}
			else
			{
				draw_text(std::string("Order : "),
						  glm::vec3(-aspect + pos.x, pos.y, pos.z),
						  sz);
			}
			float cnt = 0.0f;
			for (auto &ingredients : item.recipe)
			{
				cnt += 1.0f;
				draw_text(ingredients.first,
						  glm::vec3(-aspect + pos.x, pos.y - sz * (cnt + 1.5f), pos.z),
						  sz * 0.75f);
			}
		};
		// draw item
		{
			draw_item(player.cur_order, glm::vec3(0.1f, 0.85f, 0.0f));
			draw_item(player.bag, glm::vec3(0.75f, 0.85f, 0.0f));
		}
		// draw score
		{
			draw_text("Score: " + std::to_string(state.score),
					  glm::vec3(-aspect + 3.0f + 0.1f * H, -0.2f + 1.0f - 0.1f * H, 0.0f));
		}

		// draw goal
		{
			draw_text("Goal: " + std::to_string(state.goal),
					  glm::vec3(-aspect + 3.0f + 0.1f * H, -0.1f + 1.0f - 0.1f * H, 0.0));
		}

		// draw timer
		{
			draw_text("Remain Time: " + std::to_string((int)(state.game_timer + 0.5f)),
					  glm::vec3(-0.05f + 0.1f * H, -0.1f + 1.0f - 0.1f * H, 0.0));
		}
		// draw catch_message
		{
			draw_text(catch_message,
					  glm::vec3(-0.05f + 0.9f * H, -0.1f - 0.85f - 0.1f * H, 0.0f));
		}
		// draw either closest ingredient or the closest customer
		{
			if (state.proximity == Proximity::IngredientProx)
			{
				draw_text(closest_ingredient_name,
						  glm::vec3(-0.05f + 0.9f * H, -0.1f - 0.70f - 0.1f * H, 0.0f));
			}
			else if (state.proximity == Proximity::CustomerProx)
			{
				draw_text(closest_customer_name,
						  glm::vec3(-0.05f + 0.9f * H, -0.1f - 0.70f - 0.1f * H, 0.0f));
			}
		}
		// game state display
		switch (state.playing)
		{
		case ongoing:
		{
			draw_text(order_message,
					  glm::vec3(-aspect + 0.1f * H, -0.85f + 0.1f * H, 0.0f));
			/*draw_text("Mouse motion looks; WASD moves; escape ungrabs mouse",
							glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0));*/
		}
		break;
		case won:
		{
			draw_text("You successfully got through day " + std::to_string(day_index) +"! Press R to continue.",
					  glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0));
		}
		break;
		case lost:
		{
			draw_text("You are fired! Press R to restart.",
					  glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0));
		}
		break;
		case menu:
		{
		}
		break;
		}

		// manager state display
		switch (manager_state)
		{
		case AWAY:
		{
		}
		break;

		case ARRIVING:
		{
			draw_text("The manager is arriving soon!",
					  glm::vec3(-aspect + 0.1f * H, 0.25 + -1.0 + 0.1f * H, 0.0));
		}
		break;

		case HERE:
		{
			draw_text("THE MANAGER IS HERE!",
					  glm::vec3(-aspect + 0.1f * H, 0.25 + -1.0 + 0.1f * H, 0.0));
		}
		break;
		}
	}

	GL_ERRORS();
}
