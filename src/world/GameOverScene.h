#ifndef ROCKET_CAR_GAMEOVERSCENE_H
#define ROCKET_CAR_GAMEOVERSCENE_H 

#include <rocket/game2d/world/Scene.h>

namespace rocketcar {

class GameOverScene : public rocket::game2d::Scene {
public:
	GameOverScene();
	virtual ~GameOverScene() {}

private:
	rocket::game2d::RenderObject* spinningWheel;
	int ticksPerRoundabout;

	virtual void updateImpl();

};

}

#endif /* GAMEOVERSCENE_H */
