#include "PlayMode.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#define ERROR_F 0.000001f
#define MAX_SPEED_H 1.5f
#define MAX_HEIGHT 15.f

//To do: Test!!!

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