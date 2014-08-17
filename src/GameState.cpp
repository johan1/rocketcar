#include "GameState.h"

namespace rocketcar {

GameState& GameState::getInstance() {
	static GameState instance;
	return instance;
}

}
