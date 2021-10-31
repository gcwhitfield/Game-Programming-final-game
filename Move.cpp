#include "PlayMode.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#define ERROR_F 0.000001f
#define MAX_SPEED_H 0.5f

//To do: Test!!!

//Move walk mesh code over to move


//Updates transform and velocity of cat
void PlayMode::updateCat(PlayMode::Keys keys, float elapsed, float gravity) {
	std::cout << "keys: u d l r s " << keys.up << " " << keys.down << " " << keys.left << " " << keys.right << " " << keys.space << std::endl;

	//Get horizontal keys as an enum just to make the following code cleaner 
	enum Horizontal {
		horiL, horiR, horiN //horiN adds no horizontal offset if both or neither A and D are pressed
	};
	int hori;
	glm::vec3 offsetCamera = glm::vec3(0.0f, 1.0f, 0.0f);
	if (keys.left && !keys.right && player.catVelocity.x <= MAX_SPEED_H) {
		hori = horiL;
	}
	else if (keys.right && !keys.left && player.catVelocity.x >= -MAX_SPEED_H) {
		hori = horiR;
	}
	else {
		hori = horiN;
	}

	std::cout << hori << "hori\n";
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
			std::cout << "here\n";
			offsetCamera.y = 0.0f;
		}
		assert(keys.space || offsetCamera.y == 0.0f);

		//Want vertical increase to always be the same, 1.0f, so don't normalize

		//std::cout << " and offset camera " << offsetCamera.x << " " << offsetCamera.y << " " << offsetCamera.z << std::endl;
		//std::cout << " and vat vel " << player.catVelocity.x << " " << player.catVelocity.y << " " << player.catVelocity.z << std::endl;

		//Find worldspace velocity vector, and update player's velocity with it
		offsetCamera = player.camera->transform->rotation * offsetCamera;
		std::swap(offsetCamera.y, offsetCamera.z); //Swap to convert to a worldspace vector
		player.catVelocity += offsetCamera * glm::vec3(player.flapVelocity);
		assert(keys.space || offsetCamera.z == 0.0f);
	

	//X,Z update
	
		float horizontalInc = 3.f; //We want horizontal speed to be more than vertical speed, but by how much I would need to test
		player.posDelt = glm::vec3(horizontalInc * elapsed) * glm::vec3(player.catVelocity.x, player.catVelocity.y, 0.0f); //Affects walkmesh pos onl

	

	//y update
		float velocity = player.catVelocity.z;
		if (player.height <= ERROR_F) {
			player.airTime = 0.0f;
			player.height = 0.0f;
			player.catVelocity.z = 0.0f;
		}
		else {
			player.airTime += elapsed;
			velocity += gravity * player.airTime;
		}
		player.height += velocity * elapsed; //Height is added to the transform only after the walkmesh position is found
		std::cout << "plyaer height " << player.height << std::endl;
	
}