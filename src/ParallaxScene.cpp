#include "ParallaxScene.h"
#include "world/Box2dScene.h"

namespace rocketcar {

ParallaxScene::ParallaxScene(rocket::game2d::SceneObject const* camera) : 
		rocket::game2d::Scene(gameProjectionFunction()), followCamera(camera) {}

void ParallaxScene::updateImpl() {
	auto &pos = followCamera->getPosition();
	glm::vec3 newPos = pos;
	newPos.x = pos.x*0.5f;
	newPos.y = pos.y*0.5f;

	getCamera().setPosition(newPos);
}

}
