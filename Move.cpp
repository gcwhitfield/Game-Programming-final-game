#include "PlayMode.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#define ERROR_F 0.000001f

//Updates transform and velocity of cat
PlayMode::Player updateCat(PlayMode::Player *player, PlayMode::Keys keys, float elapsed, float gravity) {
	//Get direction and add to horizontal velocity vector
	if(space){
		
		//Get horizontal keys as an enum just to make the following code cleaner 
		enum Horizontal {
			horiL, horiR, horiN; //horiN adds no horizontal offset if both or neither A and D are pressed
		};
		int hori;
		glm::vec3 offsetCamera = glm::vec3(0.0f,1.0f,0.0f);
		if (keys.left && keys.right) {
			hori = horiN;
		}
		else if (keys.left) {
			hori = horiL;
		}
		else {
			hori = horiR;
		}

		//Case on the player's inputted direction, and create the corresponding upward vector from the camera's perspective
		if (!(keys.up == keys.down)) { //If both up and down are pressed, treat as if neither are pressed
			switch(hori) {
			case(horiL):
				offsetCamera = glm::vec3(-1.0f, 1.0f, 0.0f);
				offsetCamera = glm::normalize(offsetCamera);
				break;
			case(horiR):
				offsetCamera = glm::vec3(1.0f, 1.0f, 0.0f);
				offsetCamera = glm::normalize(offsetCamera);
				break;
			default:
			}
		}
		else if (keys.up) {
			switch (hori) {
			case(horiL):
				offsetCamera = glm::vec3(-1.0f, 1.0f, -1.0f);
				offsetCamera = glm::normalize(offsetCamera);
				break;
			case(horiR):
				offsetCamera = glm::vec3(1.0f, 1.0f, -1.0f);
				offsetCamera = glm::normalize(offsetCamera);
				break;
			default:
				offsetCamera = glm::vec3(0.0f, 1.0f, -1.0f);
				offsetCamera = glm::normalize(offsetCamera);
				break;
			}

		}
		else {
			switch (hori) {
			case(horiL):
				offsetCamera = glm::vec3(-1.0f, 1.0f, 1.0f);
				offsetCamera = glm::normalize(offsetCamera);
				break;
			case(horiR):
				offsetCamera = glm::vec3(1.0f, 1.0f, 1.0f);
				offsetCamera = glm::normalize(offsetCamera);
				break;
			default:
				offsetCamera = glm::vec3(0.0f, 1.0f, 1.0f);
				offsetCamera = glm::normalize(offsetCamera);
				break;
			}

		}

		//Find worldspace velocity vector, and update player's velocity with it
		player->curDir = player->camera->transform->rotation * offsetCamera;
		std::swap(player->curDir.y, player->curDir.z); //Swap to convert to a worldspace vector
		player->catVelocity += player->curDir * player->flapVelocity;
	}

	//X,Z update
	player->transform->positon += elapsed * glm::vec3(player->catVelocity.x, 0.0f, player->catVelocity.z); //Affects walkmesh pos only

	//y update
	{
		if (player->height <= ERROR_F) {
			player->airTime = 0.0f;
		}
		else {
			player->airTime += elapsed;
			float velocity = player->catVelocity.y + gravity * player->airTime;
			player->height += velocity * elapsed; //Height is added to the transform only after the walkmesh position is found
		}
	}
}