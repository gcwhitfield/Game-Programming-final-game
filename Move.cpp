#include "PlayMode.hpp"
#define ERROR_F 0.000001f

//Updates transform and velocity of cat
PlayMode::Player updateCat(PlayMode::Player player, bool space, float elapsed, float gravity) {
	//Get direction and add to horizontal velocity vector
	if(space){
		player.catVelocity += player.curDir * player.flapVelocity;
	}

	//X,Z update
	player.transform->positon += elapsed * glm::vec3(player.catVelocity.x, 0.0f, player.catVelocity.z);

	//y update
	{
		if (player.height <= ERROR_F) {
			player.airTime = 0.0f;
		}
		else {
			player.airTime += elapsed;
			float velocity = player.catVelocity.y + gravity * player.airTime;
			player.height += velocity * elapsed;
		}
	}
}


glm::vec3 updateDir(PlayMode::player) {

}