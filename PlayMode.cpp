#include "PlayMode.hpp"

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
//generate a new_item from the item list randomly
std::pair<std::string, StarbuckItem> new_item()
{
	std::random_device r;
	std::default_random_engine e1(r());
	size_t sz = RotationList.size() - 1;
	std::uniform_int_distribution<int> uniform_dist(0, static_cast<int>(sz));
	int choice = uniform_dist(e1);
	auto it = RotationList.begin();
	while(choice --)it++;
	return *it;
}
//debugging printing information for recipe
std::ostream &operator<<(std::ostream &os, const glm::vec3 &pos)
{
	return os << '(' << pos.x << ' ' << pos.y << ' ' << pos.z << ')';
}
std::ostream &operator<<(std::ostream &os, const StarbuckItem &item){
	for(auto &ingredient : item.recipe){
		os << "|" << ingredient.first << "|" << std::endl;
	}
	return os; 
}

bool collide(Scene::Transform * trans_a, Scene::Transform * trans_b, float radius = 6.0f){
	auto a_pos = trans_a -> position;
	auto b_pos = trans_b -> position;
	// printf("%f\n", distance2(a_pos, b_pos));
	return distance2(a_pos, b_pos) < radius;
}

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
Load<WalkMeshes> phonebank_walkmeshes(LoadTagDefault, []() -> WalkMeshes const * {
	WalkMeshes *ret = new WalkMeshes(data_path("starbucks.w"));
	walkmesh = &ret->lookup("WalkMesh");
	return ret;
});

// cite: https://www.fesliyanstudios.com/royalty-free-sound-effects-download/footsteps-31
Load<Sound::Sample> manager_footstep_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("manager_footstep.wav"));
});

PlayMode::PlayMode() : scene(*starbucks_scene)
{
	//create a player transform:
	scene.transforms.emplace_back();
	player.transform = &scene.transforms.back();

	//create a player camera attached to a child of the player transform:
	scene.transforms.emplace_back();
	scene.cameras.emplace_back(&scene.transforms.back());
	player.camera = &scene.cameras.back();
	player.camera->fovy = glm::radians(60.0f);
	player.camera->near = 0.01f;
	player.camera->transform->parent = player.transform;

	//player's eyes are 1.8 units above the ground:
	player.camera->transform->position = glm::vec3(0.0f, 0.0f, 1.8f);

	//rotate camera facing direction (-z) to player facing direction (+y):
	player.camera->transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//start player walking at nearest walk point:
	player.at = walkmesh->nearest_walk_point(player.transform->position);

	for (auto &d : scene.drawables)
	{
		std::string &str = d.transform->name;
		if (str == "Manager")
		{
			manager = &d;
			manager_here_pos = d.transform->position;
		} 
		//store ingredients information and location
		if(ingredients.find(str) != ingredients.end()){
			ingredient_transforms[str] = d.transform;
		}
		//store customers ingredients information and location
		if(str != "CustomerBase" && str.length() >= 8 && str.substr(0,8) == "Customer"){
			auto Cu = Customer(str, d.transform);
			Cu.order = new_item().second;
			customers[str] = Cu;
		}
		// add the "CustomerWaypoint" transforms from the starbucks.blend scene into a 
		// vector
		if (str.find("CustomerWaypoint") < str.size()) {
			customer_open_waypoints.emplace_back();
			customer_open_waypoints.back()->position = d.transform->position;
		}
		// set the "CustomerBase" and "CustomerSpawnPoint" transforms to their corresponding 
		// transforms in the starbucks.blend scene
		if (str == "CustomerBase") {
			customer_base = &d;
		}
		if (str == "CustomerSpawnPoint") {
			customer_spawn_point = d.transform;
		}
		// the player starts at the location of the "Player" object in the Blender scene
		if (str == "Player") {
			std::cout << "Player transform has been found in the blender scene" << std::endl;
			player.transform->position = d.transform->position;
			d.transform->position.y = 100000;
		}
	}

	assert(customer_open_waypoints.size() > 0);
	assert(customer_occupied_waypoints.size() == 0);
	assert(customer_base != NULL);
	assert(manager != NULL);

	// initialize timer
	game_state.game_timer = game_state.day_period_time;
	// TODO: mechanism of setting revenue goal
	game_state.goal = 100;

	//Orders
	player.bag.item_name = "bag";
}

PlayMode::~PlayMode()
{
}
//Order Related Function
bool PlayMode::take_order(){
	//printf("freak0, %zu\n", customers.size());
	for(auto &[name, customer] : customers){
		if(collide(customer.transform, player.transform) && // distance close
				customer.status == Customer::Status::New && // customer is new, has not given order yet
				order_status == OrderStatus::Empty) //player does not have order in hand
		{ 
			//take the order
			player.cur_order = customer.order;
			order_status = OrderStatus::Executing;
			customer.status = Customer::Status::Wait;
			order_message = std::string("Taking order ...... : ") + customer.order.item_name + "!";
			return true;
		}
	}
	return false;
}
bool PlayMode::grab_ingredient(){
	for(auto &[name, ingredient_transform]: ingredient_transforms){
		if(collide(ingredient_transform, player.transform) && // distance close
			order_status == OrderStatus::Executing//player has an order in hand
		) 
		{
			player.bag.recipe[name] ++;
		}
	}
	return false;
}
bool PlayMode::serve_order(){
	for(auto &[name, customer] : customers){
		
		if(collide(customer.transform, player.transform) && // distance close
			customer.status == Customer::Status::Wait && //customer is waiting
			customer.order.item_name == player.cur_order.item_name &&//the order match
			player.cur_order < player.bag // actually has all the correct ingredient
		) 
		{
			//serve the order
			player.cur_order = StarbuckItem(); // empty the order
			customer.status = Customer::Status::Finished;
			order_status = OrderStatus::Empty;
			order_message = std::string("Succeeded in serving : ") + customer.order.item_name + "!";
			//
			game_state.score += 50;
			//
			
			return true;
		}
	}
	return false;
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
		else if (evt.key.keysym.sym == SDLK_r)
		{
			// TODO: restart the game
			return true;
		}
		//Order related controls
		else if (evt.key.keysym.sym == SDLK_z) //take the order
		{
			take_order();
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_x) // serve the order
		{
			serve_order();
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_c) // grab ingredient
		{
			grab_ingredient();
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
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN)
	{
		if (SDL_GetRelativeMouseMode() == SDL_FALSE)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEMOTION)
	{
		if (SDL_GetRelativeMouseMode() == SDL_TRUE)
		{
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y));
			glm::vec3 up = walkmesh->to_world_smooth_normal(player.at);
			player.transform->rotation = glm::angleAxis(-motion.x * player.camera->fovy, up) * player.transform->rotation;

			float pitch = glm::pitch(player.camera->transform->rotation);
			pitch += motion.y * player.camera->fovy;
			//camera looks down -z (basically at the player's feet) when pitch is at zero.
			pitch = std::min(pitch, 0.95f * 3.1415926f);
			pitch = std::max(pitch, 0.05f * 3.1415926f);
			player.camera->transform->rotation = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));

			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed)
{

	// TODO: game menu

	// win and lose
	if (game_state.playing == won || game_state.playing == lost)
	{
		return;
	}
	// global timer count down
	game_state.game_timer -= elapsed;
	// win loss condition
	if (game_state.game_timer <= 0.0f)
	{
		if (game_state.score >= game_state.goal)
		{
			game_state.playing = won;
		}
		else
		{
			game_state.playing = lost;
		}
		return;
	}
	else
	{
		if (manager_state == HERE && player.playerStatus == Cat)
		{
			game_state.playing = lost;
			return;
		}
	}

	//player walking:
	{
		//combine inputs into a move:
		constexpr float PlayerSpeed = 3.0f;
		glm::vec2 move = glm::vec2(0.0f);
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
			move = glm::normalize(move) * PlayerSpeed * elapsed;

		//get move in world coordinate system:
		glm::vec3 remain = player.transform->make_local_to_world() * glm::vec4(move.x, move.y, 0.0f, 0.0f);

		//using a for() instead of a while() here so that if walkpoint gets stuck in
		// some awkward case, code will not infinite loop:
		for (uint32_t iter = 0; iter < 10; ++iter)
		{
			if (remain == glm::vec3(0.0f))
				break;
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
			std::cout << "NOTE: code used full iteration budget for walking." << std::endl;
		}

		//update player's position to respect walking:
		player.transform->position = walkmesh->to_world_point(player.at);

		{ //update player's rotation to respect local (smooth) up-vector:

			glm::quat adjust = glm::rotation(
				player.transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f), //current up vector
				walkmesh->to_world_smooth_normal(player.at)				  //smoothed up vector at walk location
			);
			player.transform->rotation = glm::normalize(adjust * player.transform->rotation);
		}

		/*
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
		*/
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
			if (manager_next_appearance_timer < 3)
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
				// set manager_next_appearance_timer to a random time between 5 and 10 seconds
				size_t r = rand() % 100;
				manager_next_appearance_timer = 5 + 5 * (r / (float)100);
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
				manager_stay_timer = 2.0f;
				manager_state = AWAY;
			}
		}
		break;
		}
	}

	{ // spawn new customers periodically
		customer_spawn_timer -= elapsed;
		if (customer_spawn_timer < 0) {
			std::cout << "A new customer has been spawned!" << std::endl;
			size_t r = rand() % 100;
			// the amount of time until the next customer spawns is governed by the 
			// random variable 'rand_time'. 'rand_time' is uniformly distributed 
			// between 7 and 17 seconds
			float rand_time = 7 + 10.0f * (r / (float)100); 
			customer_spawn_timer = rand_time;
			scene.transforms.emplace_back();
			scene.drawables.emplace_back(scene.transforms.back());
			Scene::Drawable new_customer = scene.drawables.back();
			new_customer.pipeline = customer_base->pipeline;
			new_customer.transform->position = customer_spawn_point->position;
			std::string new_customer_name = "Customer" + std::to_string(customers.size() + 1);
			Customer c = Customer(new_customer_name, new_customer.transform->position);
			c.order = new_item().second;
			customers[c.name] = c;
			
		}
	}

	{ // handle customer behaviour depending on state
		for(auto &[name, customer] : customers){
			switch (customer.status) {
				case Customer::Status::New: {
					customer.t_new += elapsed;
					float t = (customer.new_animation_time - customer.t_new) / customer.new_animation_time; 
					customer.transform->position = customer_spawn_point->position * t + (customer.waypoint->position * (1.0f - t));
				} break;

				case Customer::Status::Wait: {
					customer.t_wait += elapsed;

					// the customer gets angry if it waits too longs, score gets deducted
					if (customer.t_wait > customer.max_wait_time) {
						std::cout << "Customer [" << customer.name << "] has waited too long :(. Customer is leaving..." << std::endl;
						game_state.score -= 10; 
						customer.status = Customer::Status::Finished;
					}
				} break;
				
				case Customer::Status::Finished: {
					customer.t_finished += elapsed;
					float t = (customer.finished_animation_time - customer.t_finished) / customer.finished_animation_time;
					glm::vec3 desired_position = customer_spawn_point->position;
					desired_position.y += 50.0f;
					customer.transform->position = customer_spawn_point->position * t + (desired_position * (1.0f - t));
				} break;
			}
		}
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{
	//update camera aspect ratio for drawable:
	player.camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*player.camera);

	

	/* In case you are wondering if your walkmesh is lining up with your scene, try:
	{
		glDisable(GL_DEPTH_TEST);
		DrawLines lines(player.camera->make_projection() * glm::mat4(player.camera->transform->make_world_to_local()));
		for (auto const &tri : walkmesh->triangles) {
			lines.draw(walkmesh->vertices[tri.x], walkmesh->vertices[tri.y], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
			lines.draw(walkmesh->vertices[tri.y], walkmesh->vertices[tri.z], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
			lines.draw(walkmesh->vertices[tri.z], walkmesh->vertices[tri.x], glm::u8vec4(0x88, 0x00, 0xff, 0xff));
		}
	}
	*/

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
		auto draw_text = [&] (std::string str, glm::vec3 pos, float sz = 0.09f){
			lines.draw_text(str,
							glm::vec3(pos.x, pos.y, pos.z),
							glm::vec3(sz, 0.0f, 0.0f), glm::vec3(0.0f, sz, 0.0f),
							glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			lines.draw_text(str,
							glm::vec3(pos.x + ofs, pos.y + ofs, pos.z),
							glm::vec3(sz, 0.0f, 0.0f), glm::vec3(0.0f, sz, 0.0f),
							glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		};
		auto draw_item = [&] (StarbuckItem item, glm::vec3 pos, float sz = 0.10f){
			if(item.item_name == "bag"){
				draw_text(std::string("Bag : "),
							glm::vec3(-aspect + pos.x,pos.y,pos.z),
							sz);
			}
			else{
				draw_text(std::string("Order : "),							
							glm::vec3(-aspect + pos.x,pos.y,pos.z),
							sz);
			}
			float cnt = 0.0f;
			for(auto &ingredients : item.recipe){
				cnt+=1.0f;
				draw_text(ingredients.first,
							glm::vec3(-aspect + pos.x, pos.y - sz * (cnt + 1.5f), pos.z),
							sz*0.75f);
			}
		};
		// draw item
		{
			draw_item(player.cur_order, glm::vec3(0.1f,0.85f,0.0f));
			draw_item(player.bag, glm::vec3(0.75f,0.85f,0.0f));
		}
		// draw score
		{
			draw_text("Score: " + std::to_string(game_state.score), 
						glm::vec3(-aspect + 3.0f + 0.1f * H, -0.2f + 1.0f - 0.1f * H, 0.0f));
		}

		// draw goal
		{
			draw_text("Goal: " + std::to_string(game_state.goal), 
						glm::vec3(-aspect + 3.0f + 0.1f * H, -0.1f + 1.0f - 0.1f * H, 0.0));
		}

		// draw timer
		{
			draw_text("Remain Time: " + std::to_string((int)(game_state.game_timer + 0.5f)),
							glm::vec3(-0.05f + 0.1f * H, -0.1f + 1.0f - 0.1f * H, 0.0));
		}

		// game state display
		switch (game_state.playing)
		{
		case ongoing:
		{
			draw_text(order_message, 
			glm::vec3(-aspect + 0.1f * H, -0.85f + 0.1f * H, 0.0f)
			);
			/*draw_text("Mouse motion looks; WASD moves; escape ungrabs mouse",
							glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0));*/
		}
		break;
		case won:
		{
			draw_text("You successfully went through today!",
							glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0));
		}
		break;
		case lost:
		{
			draw_text("You are fired! Press R to restart",
							glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0));
		}
		break;
		case menu: {

		} break;
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
