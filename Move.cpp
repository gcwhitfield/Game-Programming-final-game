#include "PlayMode.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#define ERROR_F 0.000001f
#define MAX_SPEED_H 1.5f

//To do: Test!!!

//Move walk mesh code over to move


//Updates transform and velocity of cat
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
		else if (keys.up) {
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
			offsetCamera.y = 0.0f;
			assert(keys.space || abs(offsetCamera.y) < ERROR_F);
		}
		player.iter++;
		assert(keys.space || abs(offsetCamera.y) < ERROR_F);

		//Want vertical increase to always be the same, 1.0f, so don't normalize

		//Find worldspace velocity vector, and update player's velocity with it
		offsetCamera = player.camera->transform->rotation * offsetCamera;
	 
		player.catVelocity += offsetCamera * glm::vec3(player.flapVelocity);
		player.catVelocity += offsetCamera * glm::vec3(player.flapVelocity);
		if (player.catVelocity.x >= MAX_SPEED_H) player.catVelocity.x = MAX_SPEED_H;
		else if (player.catVelocity.x <= MAX_SPEED_H) player.catVelocity.x = -MAX_SPEED_H;
		if (player.catVelocity.y >= MAX_SPEED_H) player.catVelocity.y = MAX_SPEED_H;
		else if (player.catVelocity.y <= MAX_SPEED_H) player.catVelocity.y = -MAX_SPEED_H;
		
		assert(keys.space || abs(offsetCamera.z) < ERROR_F);
	

	//X,Z update
	
		float horizontalInc = 0.5f; //We want horizontal speed to be more than vertical speed, but by how much I would need to test
		player.posDelt = glm::vec3(horizontalInc * elapsed) * glm::vec3(player.catVelocity.x, player.catVelocity.y, 0.0f); //Affects walkmesh pos onl

	

	//y update
		float velocity = player.catVelocity.z;
		if (player.height <= ERROR_F) {
			player.airTime = 0.0f;
			player.height = 0.0f;
			player.catVelocity = glm::vec3(0.f);
			player.posDelt = glm::vec3(0.f);
		}
		else {
			player.airTime += elapsed;
			velocity += gravity * player.airTime;
		}
		player.height += velocity * elapsed; //Height is added to the transform only after the walkmesh position is found
	
}