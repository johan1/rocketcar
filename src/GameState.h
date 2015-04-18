#ifndef ROCKETCAR_GAMESTATE_H
#define ROCKETCAR_GAMESTATE_H

#include <string>
#include <vector>

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

std::string getCurrentLevel() const {
	return levels[levelIndex];
}

void nextLevel() {
	levelIndex = (levelIndex + 1) % levels.size();
}

private:
	std::string carSource;
	std::vector<std::string> levels;
	uint32_t levelIndex = 0;

	GameState();
	GameState(GameState const&) = delete;
	GameState& operator=(GameState const&) = delete;
};

}

#endif /* GAMESTATE_H */
