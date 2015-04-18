#include "GameState.h"

namespace rocketcar {

GameState::GameState() {
	levels.push_back("level1.json");
	levels.push_back("level2.json");
}

GameState& GameState::getInstance() {
	static GameState instance;
	return instance;
}

}
