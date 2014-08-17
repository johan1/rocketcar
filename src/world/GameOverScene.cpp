#include "GameOverScene.h"

#include <rocket/Types.h>
#include <rocket/game2d/world/Sprite.h>
#include <rocket/util/Transformation.h>

#include <cmath>

namespace rocketcar {

using namespace rocket;
using namespace rocket::game2d;
using namespace rocket::resource;
using namespace rocket::resource::image;

using namespace boost::chrono;

static std::function<glm::mat4(glm::vec4 const&)> createProjectionFunction() {
    return [](glm::vec4 const& viewPort) -> glm::mat4 {
        float sw = viewPort[2];
        float sh = viewPort[3];

		if (sw < sh) {
			float r = sh/sw;
	        return glm::ortho(-0.5f, 0.5f, -r * 0.5f, r * 0.5f, -5.0f, 5.0f);
		} else {
			float r = sw/sh;
	        return glm::ortho(-r * 0.5f, r * 0.5f, -0.5f, 0.5f, -5.0f, 5.0f);
		}
    };
}

GameOverScene::GameOverScene() : Scene(createProjectionFunction()), spinningWheel(nullptr),
		ticksPerRoundabout(duration_cast<ticks>(seconds(10)).count()) {
	auto spinningWheelSprite = std::make_shared<Sprite>(
			ImageId(ResourceId("images/spinning wheel.png")), 1.0f, 0.8f);

	spinningWheel = add(spinningWheelSprite, false);
	spinningWheel->move(glm::vec3(0, 0, -1));

	schedule([this] () {
		auto rotation = 2.0f*M_PI/static_cast<float>(ticksPerRoundabout);
		spinningWheel->rotate(rotation, util::Z_AXIS);
		return ticks(1);
	});
	// Set spinning properties
}

void GameOverScene::updateImpl() {
}

}
