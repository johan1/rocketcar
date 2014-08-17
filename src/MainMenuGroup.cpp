#include "MainMenuGroup.h"
#include "LevelGroup.h"

#include "world/GameOverScene.h"

#include <rocket/game2d/world/Menu.h>
#include <rocket/game2d/Director.h>

using namespace rocketcar;
using namespace rocket::resource;
using namespace rocket::game2d;

MainMenuGroup::MainMenuGroup() {
	auto goScene = std::make_shared<GameOverScene>();

	auto menu = std::make_shared<Menu>(ResourceId("KhmerOS.ttf"));
	menu->addItem("Inferno", [](){});
	menu->addItem("King of the hills", [](){});
	menu->addItem("Boxes, so many boxes", [](){});
	menu->addItem("Rocket, go!", [](){});

	menu->addItem("Test level", [this, menu]() {
		Director::getDirector().removeSceneGroup(this);
		Director::getDirector().addSceneGroup(std::make_shared<LevelGroup>());
	});

	addScene(goScene);
	addScene(menu);
}

void MainMenuGroup::onLoaded() {
	auto& audioPlayer = Director::getDirector().getAudioPlayer();
	bgMusic = audioPlayer.load(ResourceId("audio/main_menu.ogg"));
	bgMusicId = audioPlayer.play(bgMusic, true); // Repeat
}

void MainMenuGroup::onUnloaded() {
	auto& audioPlayer = Director::getDirector().getAudioPlayer();
	audioPlayer.stop(bgMusicId);
	audioPlayer.free(bgMusic);
}
