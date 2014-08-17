#ifndef _ROCKET_LEVEL_GROUP_H_
#define _ROCKET_LEVEL_GROUP_H_

#include <rocket/game2d/SceneGroup.h>
#include <Box2D/Box2D.h>

#include "box2d/CommonStructs.h"
#include "world/Box2dScene.h"
#include "ParallaxScene.h"
#include "hud/EnergyBar.h"

#include "world/Box.h"

#include <rocket/game2d/world/ParticleEmitter.h>

#include <unordered_map>

#include <rocket/transitions/CircleFadeOut.h>

namespace rocketcar {

class LevelGroup : public rocket::game2d::SceneGroup {
public:
	LevelGroup(); // TODO: We should add some parameters here, car source and level source.

private:
	rocket::box2d::Box2dParseData rocketCarBuildData;

	class RocketCar {
	public:
		b2Body* chassi;
		b2Body* wheel1;
		b2Body* wheel2;
		b2Fixture* roofFixture; // We destroy car if roof collides with ground

		bool applyGas;
		bool applyRocket;

		uint32_t maxRocketFuel;
		uint32_t rocketFuel;

		std::shared_ptr<rocket::game2d::ParticleEmitter> rocketEmitter;

		RocketCar() : chassi(nullptr), wheel1(nullptr), wheel2(nullptr), roofFixture(nullptr),
				applyGas(false), applyRocket(false), maxRocketFuel(180), rocketFuel(180), crashed(false) {}

		bool hasCrashed() const {
			return crashed;
		}
		
		void setCrashed(bool crashed) {
			if (crashed) {
				applyGas = false;
				applyRocket = false;
			}

			this->crashed = crashed;
		}

	private:
		bool crashed;
	};

	std::vector<b2Fixture*> groundFixtures;

	struct HUD {
		std::shared_ptr<EnergyBar> energyBar;
	};

	uint32_t bgMusic;
	uint32_t bgMusicId;

	std::shared_ptr<Box2dScene> box2dScene;
	std::shared_ptr<ParallaxScene> bgScene;
	std::shared_ptr<rocket::game2d::Scene> hudScene;

	rocket::game2d::SceneObject *energyBarNode;

	std::shared_ptr<rocket::game2d::ParticleGenerator> rocketParticleGenerator;
	std::shared_ptr<rocket::game2d::ParticleGenerator> explosionParticleGenerator;

	std::shared_ptr<rocket::CircleFadeOut> postRenderer;

	RocketCar rocketCar;
	HUD hud;

	virtual void onLoaded();
	virtual void onUnloaded();
	virtual void onUpdate();

	void updateHud();

	void loadHud();
	void loadLevel();
	void loadRocketCar(bool updateWorldProperties);

	void solveRoofCollision(b2Fixture* other);
	void solveBoxCollision(b2Body *body, float impulse2);
	void onPreSolveContact(b2Contact *contact, b2Manifold const* oldManifold);
	void onPostSolveContact(b2Contact* contact, const b2ContactImpulse* impulse);

	bool onControllerEvent(rocket::input::ControllerEvent const& event);

	std::unordered_map<b2Body*, std::unique_ptr<Box>> boxes;

	void fadeOut();

	void fadeIn();

};

};

#endif

