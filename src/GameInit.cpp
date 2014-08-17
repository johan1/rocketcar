#include "GameInit.h"

#include "LevelGroup.h"

#include <rocket/game2d/Director.h>

// #include <rocket/game2d/world/Menu.h>

// #include <rocket/game2d/renderer/RendererManager.h>
// #include <rocket/game2d/renderer/SpriteRenderer.h>
// #include <rocket/game2d/renderer/TextRenderer.h>
// #include "world/Box2dRenderer.h"

#include "MainMenuGroup.h"
#include "CarPickerGroup.h"

using namespace rocketcar;
using namespace rocket::resource;
using namespace rocket::game2d;

namespace rocket {

void gameInit() {
/*
	RendererManager::getInstance().setRenderer<Sprite>(std::unique_ptr<Renderer<Sprite>>(new SpriteRenderer()));
	RendererManager::getInstance().setRenderer<Text>(std::unique_ptr<Renderer<Text>>(new TextRenderer()));
	RendererManager::getInstance().setRenderer<b2World>(std::unique_ptr<Renderer<b2World>>(new Box2dRenderer()));
*/

	Director::getDirector().addSceneGroup(std::make_shared<CarPickerGroup>());
//	Director::getDirector().addSceneGroup(std::make_shared<MainMenuGroup>());
//	Director::getDirector().addSceneGroup(std::make_shared<LevelGroup>());

/*
	auto menu = std::make_shared<Menu>(ResourceId("KhmerOS.ttf"));
	menu->addItem("Level 1", [](){});
	menu->addItem("Level 2", [](){});
	menu->addItem("Level 3", [](){});
	menu->addItem("Level 4", [](){});

	menu->addItem("Level 5", [menu]() {
		Director::getDirector().removeScene(menu);
		Director::getDirector().addSceneGroup(std::make_shared<LevelGroup>());
	});

	menu->setSelectedIndex(2);
	Director::getDirector().addScene(menu);
	
	//Director::getDirector().addSceneGroup(std::make_shared<LevelGroup>());
*/
}

}
