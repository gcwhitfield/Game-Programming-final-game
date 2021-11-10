#include "PlayMode.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#define ERROR_F 0.000001f
#define MAX_SPEED_H 1.5f
#define MAX_HEIGHT 15.f

//To do: Resolve issues with bounded walkmesh, test control speed for flight

//Move walk mesh code over to move

void PlayMode::Player::updateDrawable() {
	if (playerStatus == Cat || playerStatus == toHuman) {
		cat->shouldDraw = true;
		human->shouldDraw = false;
	}
	else{
		cat->shouldDraw = false;
		human->shouldDraw = true;
	}
}

//Updates transform and velocity of cat
//All code is in CAMERA space
void PlayMode::updateCat(PlayMode::Keys keys, float elapsed, float gravity) {

	//Get horizontal keys as an enum just to make the following code cleaner 
	enum Horizontal {
		horiL, horiR, horiN //horiN adds no horizontal offset if both or neither A and D are pressed
	};
	int hori;
	glm::vec3 offsetCamera = glm::vec3(0.0f, 1.0f, 0.0f);
	if (keys.left && !keys.right) {
		hori = horiL;
	}
	else if (keys.right && !keys.left) {
		hori = horiR;
	}
	else {
		hori = horiN;
	}
	//Get direction and add to horizontal velocity vector
	

		//Case on the player's inputted direction, and create the corresponding upward vector from the camera's perspective
		if (!(keys.up != keys.down)) { //If both up and down are pressed, treat as if neither are pressed
			switch(hori) {
			case(horiL):
				offsetCamera = glm::vec3(-1.0f, 1.0f, 0.0f);
				break;
			case(horiR):
				offsetCamera = glm::vec3(1.0f, 1.0f, 0.0f);
				break;
			default:
				break;
			}
		}
		else if (!keys.up) {
			switch (hori) {
			case(horiL):
				offsetCamera = glm::vec3(-1.0f, 1.0f, -1.0f);
				break;
			case(horiR):
				offsetCamera = glm::vec3(1.0f, 1.0f, -1.0f);
				break;
			default:
				offsetCamera = glm::vec3(0.0f, 1.0f, -1.0f);
				break;
			}

		}
		else {
			switch (hori) {
			case(horiL):
				offsetCamera = glm::vec3(-1.0f, 1.0f, 1.0f);
				break;
			case(horiR):
				offsetCamera = glm::vec3(1.0f, 1.0f, 1.0f);
				break;
			default:
				offsetCamera = glm::vec3(0.0f, 1.0f, 1.0f);
				break;
			}

		}

		if (!keys.space) {
			offsetCamera = glm::vec3(0.0f);
			assert(keys.space || abs(offsetCamera.y) < ERROR_F);
		}
		assert(keys.space || abs(offsetCamera.y) < ERROR_F);

		float vertInc = 1.5f;
		offsetCamera.y *= vertInc;

		//Want vertical increase to always be the same, 1.0f, so don't normalize

		//Find worldspace velocity vector, and update player's velocity with it

	 
		player.catVelocity += offsetCamera * glm::vec3(player.flapVelocity);
		if (player.catVelocity.x >= MAX_SPEED_H) player.catVelocity.x = MAX_SPEED_H;
		else if (player.catVelocity.x <= -MAX_SPEED_H) player.catVelocity.x = -MAX_SPEED_H;
		if (player.catVelocity.z >= MAX_SPEED_H) player.catVelocity.z = MAX_SPEED_H;
		else if (player.catVelocity.z <= -MAX_SPEED_H) player.catVelocity.z = -MAX_SPEED_H;

		  
		float horizontalVelocity = 2.5; //  1.5f; 
		player.posDelt = glm::vec2(horizontalVelocity * elapsed) * glm::vec2(player.catVelocity.x, player.catVelocity.z); //Affects walkmesh pos onl

	

	//y update
		float velocity = player.catVelocity.y;
		if ((player.height <= ERROR_F && !player.grounded) || (player.grounded && !keys.space)) {
			player.airTime = 0.0f;
			player.height = 0.0f;
			if (player.lastCollision)
				player.height = objectHeight;
			player.catVelocity = glm::vec3(0.f);
			player.posDelt = glm::vec3(0.f);
			player.grounded = true;
		}
		else {
			player.grounded = false;
			player.airTime += elapsed;
			velocity += gravity * player.airTime;
		}
		player.height += velocity * elapsed; //Height is added to the transform only after the walkmesh position is found
		if (player.height <= ERROR_F) player.height = 0.0f;
		if (player.height >= MAX_HEIGHT) {
			player.height = MAX_HEIGHT;
			player.catVelocity.y = 0.0f;
			player.airTime = 0.0f;
		}
	
}

//Update player status from cat to human or viceversa, instant for second, drops object to ground for first
void PlayMode::transition(float elapsed, float gravity) {
	if (player.playerStatus == toHuman) {
		if (player.height <= ERROR_F) {
			player.playerStatus = Human;
		}
		else {
			player.fallTime += elapsed;
			player.height += gravity * player.fallTime * elapsed;
			if (player.height <= ERROR_F) player.height = 0.0f;
			player.transform->position.z = player.height;
			
		}
	}
	else {
		player.playerStatus = Cat;
	}
}

//Decide whether the cat has hit a collision, and if so, how
void PlayMode::decidePos(glm::vec3 inBounds, glm::vec3 at) {
	auto posDif = [this](glm::vec3 inBounds, glm::vec3 at) {
		return glm::length(inBounds - at);
	};
	if (posDif(inBounds,at) <= ERROR_F) { //If positions are close, assume no object is at player
		player.transform->position.x = at.x;
		player.transform->position.y = at.y;
		player.lastCollision = false;
		if (player.height >= ERROR_F)
			player.grounded = false;
	}
	else { //Player is over the collision bounds
		if (player.lastCollision) { //If the player collided last frame, don't push off
			if (player.height <= objectHeight + ERROR_F) { //If "in" object, treat as if grounded on object
				player.height = objectHeight;
				player.catVelocity = glm::vec3(0.0f);
				player.grounded = true;
				player.airTime = 0.0f;
			}
			else
				player.grounded = false;
			player.transform->position.x = inBounds.x;
			player.transform->position.y = inBounds.y;
			player.lastCollision = true;
		}
		else { //Last frame, the player hadn't collided yet
			if (player.height >= ERROR_F)
				player.grounded = false;
			if (player.height >= objectHeight) { //Allow over object, and treat frame as collision
				player.lastCollision = true;
				player.transform->position.x = inBounds.x;
				player.transform->position.y = inBounds.y;
			}
			else { //Block player from moving further
				player.lastCollision = false;
				player.transform->position.x = at.x;
				player.transform->position.y = at.y;
			}
		}
	}
	player.transform->position.z = 0.0f;

}

void PlayMode::getBoundedPos(glm::vec2 move, WalkMesh const* boundWalkmesh, WalkMesh const* walkmesh) {
	
	//Get bounded walkmesh position for cat, then use to get new position for cat

	glm::vec3 remain = player.transform->make_local_to_world() * glm::vec4(move.x, move.y, 0.0f, 0.0f);

	//Get position on bounded walkmesh, exactly as done for walkmesh ebfore
	for (uint32_t iter = 0; iter < 10; ++iter)
	{
		if (remain == glm::vec3(0.0f))
			break;
		WalkPoint end;
		float time;
		boundWalkmesh->walk_in_triangle(player.outOfBounds, remain, &end, &time);
		player.outOfBounds = end;
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
		if (boundWalkmesh->cross_edge(player.outOfBounds, &end, &rotation))
		{
			//stepped to a new triangle:
			player.outOfBounds = end;
			//rotate step to follow surface:
			remain = rotation * remain;
		}
		else {
			//ran into a wall, bounce / slide along it:
			glm::vec3 const& a = boundWalkmesh->vertices[player.outOfBounds.indices.x];
			glm::vec3 const& b = boundWalkmesh->vertices[player.outOfBounds.indices.y];
			glm::vec3 const& c = boundWalkmesh->vertices[player.outOfBounds.indices.z];
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
			else {
				//if it's just pointing along the edge, bend slightly away from wall:
				remain += 0.01f * d * in;
			}
		}
	}

	if (remain != glm::vec3(0.0f)) {
		std::cout << "NOTE: code used full iteration budget for walking." << std::endl;
	}

	//Get worldspace posiitions on both walkmeshes
	glm::vec3 at = walkmesh->to_world_point(player.at);
	assert(boundWalkmesh != NULL);
	glm::vec3 inBound = boundWalkmesh->to_world_point(player.outOfBounds);

	//Updatte proper cat position
	decidePos(inBound, at);
	player.outOfBounds = boundWalkmesh->nearest_walk_point(player.transform->position);
	player.at = walkmesh->nearest_walk_point(player.transform->position);
}