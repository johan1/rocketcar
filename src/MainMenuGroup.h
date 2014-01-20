#ifndef _ROCKET_MAIN_MENU_GROUP_H_
#define _ROCKET_MAIN_MENU_GROUP_H_

#include <rocket/game2d/SceneGroup.h>

namespace rocketcar {

class MainMenuGroup : public rocket::game2d::SceneGroup {
public:
	MainMenuGroup();

private:
	uint32_t bgMusic;
	uint32_t bgMusicId;

	virtual void onLoaded();
	virtual void onUnloaded();
};

}

#endif
