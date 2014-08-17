#ifndef ROCKETCAR_GAMESTATE_H
#define ROCKETCAR_GAMESTATE_H

#include <string>

namespace rocketcar {

class GameState {
public:
static GameState& getInstance();

void setCarSource(std::string const& carSource) {
	this->carSource = carSource;
}

std::string getCarSource() const {
	return carSource;
}

private:
	std::string carSource;

	GameState() {}
	GameState(GameState const&) = delete;
	GameState& operator=(GameState const&) = delete;
};

}

#endif /* GAMESTATE_H */
