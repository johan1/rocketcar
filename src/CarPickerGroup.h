#ifndef _ROCKET_CARPICKERGROUP_H_
#define _ROCKET_CARPICKERGROUP_H_

#include <rocket/game2d/SceneGroup.h>
#include <rocket/game2d/world/Sprite.h>
#include <rocket/game2d/world/RenderObject.h>
#include <rocket/game2d/scene/ControllerScene.h>

#include "world/Box2dScene.h"

namespace rocketcar {

class CarPickerGroup : public rocket::SceneGroup {
public:
	CarPickerGroup();

private:
	std::shared_ptr<rocket::ControllerScene> controllerScene;
	rocket::RenderObject* leftButtonSpriteObject;
	rocket::RenderObject* rightButtonSpriteObject;
	rocket::RenderObject* goButtonSpriteObject;

	std::shared_ptr<rocket::Sprite> leftButtonSprite;
	std::shared_ptr<rocket::Sprite> rightButtonSprite;

	std::shared_ptr<Box2dScene> carScene;

	std::vector<std::string> carSources;

	rocket::box2d::LoadData carBox2dData;

	int carIndex;

	void load();
	void selectCar();

	void switchToNextCar();
	void switchToPreviousCar();

	void initControllerScene();
	void initCarScene();
	bool onControllerEvent(rocket::input::ControllerEvent const& event);
};

}

#endif
