#include "PlayMode.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#define ERROR_F 0.000001f
#define MAX_SPEED_H 2.5f;

//To do: Test!!!

//Move walk mesh code over to move



//Updates transform and velocity of cat
PlayMode::Player updateCat(PlayMode::Player *player, PlayMode::Keys keys, float elapsed, float gravity) {

	//Get horizontal keys as an enum just to make the following code cleaner 
	enum Horizontal {
		horiL, horiR, horiN; //horiN adds no horizontal offset if both or neither A and D are pressed
	};
	int hori;
	glm::vec3 offsetCamera = glm::vec3(0.0f, 1.0f, 0.0f);
	if (keys.left && !keys.right && player->flapVelocity.x <= MAX_SPEED_H) {
		hori = horiL;
	}
	else if (keys.right && !keys.left && player->flapVelocity.x >= -MAX_SPEED_H) {
		hori = horiR;
	}
	else (keys.left && keys.right) {
		hori = horiN;
	}
	//Get direction and add to horizontal velocity vector
	{

		//Case on the player's inputted direction, and create the corresponding upward vector from the camera's perspective
		if (!(keys.up == keys.down)) { //If both up and down are pressed, treat as if neither are pressed
			switch(hori) {
			case(horiL):
				offsetCamera = glm::vec3(-1.0f, 1.0f, 0.0f);
				break;
			case(horiR):
				offsetCamera = glm::vec3(1.0f, 1.0f, 0.0f);
				break;
			default:
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
		}

		//Want vertical increase to always be the same, 1.0f, so don't normalize

		//Find worldspace velocity vector, and update player's velocity with it
		player->curDir = player->camera->transform->rotation * offsetCamera;
		std::swap(player->curDir.y, player->curDir.z); //Swap to convert to a worldspace vector
		player->catVelocity += player->curDir * player->flapVelocity;
	}

	//X,Z update
	{
		float horizontalInc = 3.f; //We want horizontal speed to be more than vertical speed, but by how much I would need to test
		player->transform->positon += glm::vec3(horizontalInc*elapsed) *  glm::vec3(player->catVelocity.x, 0.0f, player->catVelocity.z); //Affects walkmesh pos only
	}

	//y update
	{
		float velocity = player->catVelocity.y
		if (player->height <= ERROR_F) {
			player->airTime = 0.0f;
			player->height = 0.0f;
		}
		else {
			player->airTime += elapsed;
			velocity += gravity * player->airTime;
		}
		player->height += velocity * elapsed; //Height is added to the transform only after the walkmesh position is found
	}
}