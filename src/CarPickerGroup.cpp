#include "CarPickerGroup.h"
#include "MainMenuGroup.h"

#include <rocket/game2d/scene/ControllerScene.h>
#include <rocket/input/ControllerEvent.h>
#include <rocket/game2d/world/Sprite.h>
#include <rocket/game2d/Director.h>

#include "GameState.h"
#include "JsonLoader.h"

#include "box2d/RubeParser.h"

using namespace rocket;

namespace rocketcar {

static glm::mat4 projectionFunction(glm::vec4 const& viewPort) {
    float sw = viewPort[2];
    float sh = viewPort[3];

	if (sw < sh) {
		float r = sh/sw;
	    return glm::ortho(-1.f, 1.f, -r * 1.f, r * 1.f, -5.0f, 5.0f);
	} else {
		float r = sw/sh;
	    return glm::ortho(-r * 1.f, r * 1.f, -1.f, 1.f, -5.0f, 5.0f);
	}
}

CarPickerGroup::CarPickerGroup() : carIndex(0) {
	initCarScene();
	initControllerScene();
}

void CarPickerGroup::initControllerScene() {
	controllerScene = std::make_shared<ControllerScene>(0, projectionFunction);

	leftButtonSprite = std::make_shared<Sprite>(ImageId(ResourceId("images/left_arrow_normal.png")), 0.2, 0.1);
	leftButtonSpriteObject = controllerScene->add(leftButtonSprite, false);
	leftButtonSpriteObject->setPosition(glm::vec3(-0.2, -0.8, 0.0));
	auto leftButton = controllerScene->addButton(button_id::LEFT, leftButtonSpriteObject);
	leftButton->setDimension(util::newDimension(0.2, 0.1, 2.0));

	rightButtonSprite = std::make_shared<Sprite>(ImageId(ResourceId("images/right_arrow_normal.png")), 0.2, 0.1);
	rightButtonSpriteObject = controllerScene->add(rightButtonSprite, false);
	rightButtonSpriteObject->setPosition(glm::vec3(0.2, -0.8, 0.0));
	auto rightButton = controllerScene->addButton(button_id::RIGHT, rightButtonSpriteObject);
	rightButton->setDimension(util::newDimension(0.2, 0.1, 2.0));

	auto goButtonSprite = std::make_shared<Sprite>(ImageId(ResourceId("images/go_button.png")), 0.2, 0.2);
	goButtonSpriteObject = controllerScene->add(goButtonSprite, false);
	goButtonSpriteObject->setPosition(glm::vec3(0.0, -0.8, 0.0));
	auto goButton = controllerScene->addButton(button_id::ACTION, goButtonSpriteObject);
	goButton->setDimension(util::newDimension(0.2, 0.2, 2.0));

	controllerScene->registerHandler(HandlerBuilder<ControllerEvent>::build( [this] (ControllerEvent const& event) {
		return onControllerEvent(event);
	}));

	controllerScene->addOnProjectionChangedObserver([this] () {
		auto centerBottom = controllerScene->unproject( glm::vec3(0, -1, 0));
		auto leftButtonPos = centerBottom + glm::vec3(-0.2, 0.15, 0.0);
		auto rightButtonPos = centerBottom + glm::vec3(0.2, 0.15, 0.0);
		auto goButtonPos = centerBottom + glm::vec3(0.0, 0.15, 0.0);

		leftButtonSpriteObject->setPosition(leftButtonPos);
		rightButtonSpriteObject->setPosition(rightButtonPos);
		goButtonSpriteObject->setPosition(goButtonPos);
	});

	this->addScene(controllerScene);
}

void CarPickerGroup::initCarScene() {
	// Load from file.
	load();

	// Initiating box2d scene.
	carScene = std::make_shared<Box2dScene>();
	carScene->getCamera().setScale(glm::vec3(25.0f, 25.0f, 1.0f));

	auto pickerBg = std::make_shared<Sprite>(ImageId(ResourceId("images/car_picker_bg.png")), 25.0, 25.0);
	auto bgObject = carScene->add(pickerBg, false);
	bgObject->move(glm::vec3(0, 0, -1));

	// Create the bounding box.
	b2BodyDef groundDef;
	groundDef.type = b2_staticBody;

	const float s = 12.0f;
	const float w = 1.0f;

	b2FixtureDef groundFixtureDef;
	groundFixtureDef.density = 1.0f;
	b2PolygonShape boxShape;

	// Bottom
	boxShape.SetAsBox(s/2.0, w/2.0f);
	groundFixtureDef.shape = &boxShape;
	groundDef.position = b2Vec2(0, -(s/2.0+w/2.0f));
	auto bottom = carScene->getBox2dWorld().CreateBody(&groundDef);
	bottom->CreateFixture(&groundFixtureDef);

	// Top
	groundFixtureDef.shape = &boxShape;
	groundDef.position = b2Vec2(0, (s/2.0+w/2.0f));
	auto top = carScene->getBox2dWorld().CreateBody(&groundDef);
	top->CreateFixture(&groundFixtureDef);

	// Left
	boxShape.SetAsBox(w/2.0f, s/2.0f);
	groundFixtureDef.shape = &boxShape;
	groundDef.position = b2Vec2(-(s/2.0+w/2.0f), 0);
	auto left = carScene->getBox2dWorld().CreateBody(&groundDef);
	left->CreateFixture(&groundFixtureDef);

	// Right
	groundFixtureDef.shape = &boxShape;
	groundDef.position = b2Vec2((s/2.0+w/2.0f), 0);
	auto right = carScene->getBox2dWorld().CreateBody(&groundDef);
	right->CreateFixture(&groundFixtureDef);

	selectCar();
	this->addScene(carScene);
}

void CarPickerGroup::switchToNextCar() {
	// TODO: Destroy previous
	carIndex = (carIndex + 1) % carSources.size();

	selectCar();
}

void CarPickerGroup::switchToPreviousCar() {
	if (carIndex == 0) {
		carIndex = carSources.size() - 1;
	} else {
		carIndex = (carIndex-1) % carSources.size();
	}

	selectCar();
}

void CarPickerGroup::selectCar() {
	for (auto body : carBox2dData.bodiesInLoadOrder) {
		carScene->removeAttachedObjects(body);
		carScene->getBox2dWorld().DestroyBody(body);
	}
	GameState::getInstance().setCarSource(carSources[carIndex]);

	LOGD("Select car " << carSources[carIndex]);
	RubeParser rubeParser;
	auto doc = loadJson(ResourceId(carSources[carIndex]));
	auto car1BuildData = rubeParser.loadRubeDocument(doc);

	carBox2dData = carScene->loadBox2dData(car1BuildData, b2Vec2(0.0, 0), false);
}

bool CarPickerGroup::onControllerEvent(rocket::input::ControllerEvent const& event) {
	if (event.getButtonId() == button_id::LEFT) {
		if (event.getValue() == 0) { // Release
			leftButtonSprite->updateImage(ImageId(ResourceId("images/left_arrow_normal.png")));
		} else {
			leftButtonSprite->updateImage(ImageId(ResourceId("images/left_arrow_pressed.png")));
			switchToPreviousCar();
		}
		return true;
	} else if (event.getButtonId() == button_id::RIGHT) {
		if (event.getValue() == 0) { // Release
			rightButtonSprite->updateImage(ImageId(ResourceId("images/right_arrow_normal.png")));
		} else {
			rightButtonSprite->updateImage(ImageId(ResourceId("images/right_arrow_pressed.png")));
			switchToNextCar();
		}
		return true;
	} else if (event.getButtonId() == button_id::ACTION) {
		Director::getDirector().removeSceneGroup(this);
		Director::getDirector().addSceneGroup(std::make_shared<MainMenuGroup>());
		return true;
	} else {
		return false;
	}
}

void CarPickerGroup::load() {
	auto jo = loadJson(ResourceId("car_picker.json"));
	auto cars = jo["available_cars"];
	for (std::size_t i = 0; i < cars.size(); ++i) {
		carSources.push_back(cars[i].asString());
	}
}

}
