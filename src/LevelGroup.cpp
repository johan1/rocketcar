#include "LevelGroup.h"
#include "world/Box2dScene.h"

#include <rocket/Algorithm.h>
#include <rocket/Log.h>
#include <rocket/Application.h>
#include <rocket/game2d/Director.h>
#include <rocket/game2d/world/Scene.h>
#include <rocket/game2d/world/Sprite.h>

#include <rocket/input/ControllerEvent.h>
#include <rocket/game2d/world/Text.h>
#include <rocket/game2d/scene/ControllerScene.h>
#include <rocket/util/Geometry.h>

#include "GameState.h"
#include "CarPickerGroup.h"

#include "BackgroundAtlas.h"
#include "ParallaxScene.h"

#include "LevelLoader.h"

#include <rocket/game2d/world/Sprite.h>
#include <rocket/game2d/world/ParticleEmitter.h>
#include <rocket/game2d/world/Uniform2dParticleGenerator.h>
#include <rocket/transitions/CircleFadeOut.h>
#include "JsonLoader.h"
#include "box2d/RubeParser.h"
#include "box2d/Box2dLoader.h"

#include "world/Box.h"

using namespace rocket;
using namespace rocket::game2d;
using namespace rocket::input;
using namespace rocket::resource;
using namespace rocket::resource::image;
using namespace rocket::util;
using namespace rocket::glutils;
 
using namespace rocket::box2d;

namespace rocketcar {

LevelGroup::LevelGroup() {
	// float a1, float a2, float r, float s1, float s2, glutils::RGBAColor c1, glutils::RGBAColor c2, float duration
	rocketParticleGenerator = std::make_shared<Uniform2dParticleGenerator>( 7 * M_PI/6.0f, 11 * M_PI / 6.0f, 3.0f, 0.2, 0.8, 10, 40,
			RGBAColor(0xff0000ff), RGBAColor(0xffce0080));
	explosionParticleGenerator = std::make_shared<Uniform2dParticleGenerator>(0, 2 * M_PI, 3.0f, 0.2, 0.8, 10, 40,
			RGBAColor(0xff0000ff), RGBAColor(0x40202080));

	std::shared_ptr<ControllerScene> controllerScene = std::make_shared<ControllerScene>(1);
	auto button1 = controllerScene->addButton(button_id::BUTTON1);
	button1->setPosition(glm::vec3(-0.5, 0, 0));
	button1->setDimension(glm::vec3(1, 1, 1));

	auto button2 = controllerScene->addButton(button_id::BUTTON2);
	button2->setPosition(glm::vec3(0.5, 0, 0));
	button2->setDimension(glm::vec3(1, 1, 1));

	addScene(controllerScene);

	postRenderer = std::make_shared<CircleFadeOut>();
	postRenderer->setRadius(0.0f, 0.0f);

	box2dScene = std::make_shared<Box2dScene>();
	box2dScene->setPostRenderer(postRenderer);
	bgScene = std::make_shared<ParallaxScene>(&box2dScene->getCamera());
	bgScene->setPostRenderer(postRenderer);

	// Loading rocket car data from file.
	RubeParser rubeParser;
	auto doc = loadJson(ResourceId(GameState::getInstance().getCarSource()));
	rocketCarBuildData = rubeParser.loadRubeDocument(doc);

	loadRocketCar(true);
	loadLevel();

	box2dScene->registerHandler(HandlerBuilder<ControllerEvent>::build( [this] (ControllerEvent const& event) {
		return onControllerEvent(event);
	}));

	box2dScene->setPreSolveCallback([this] (b2Contact* contact, b2Manifold const* oldManifold) {
		onPreSolveContact(contact, oldManifold);
	});

	box2dScene->setPostSolveCallback([this] (b2Contact* contact, const b2ContactImpulse* impulse) {
		onPostSolveContact(contact, impulse);
	});

	box2dScene->setBeginContactCallback([this] (b2Contact* contact) {
		onContactBegin(contact);
	});

	box2dScene->setEndContactCallback([this] (b2Contact* contact) {
		onContactEnd(contact);
	});

	addScene(bgScene);
	addScene(box2dScene);

	loadHud();
}

void LevelGroup::loadHud() {
	// Finally create and add HUD Scene
	hudScene = std::make_shared<Scene>(gameProjectionFunction());
	hudScene->getCamera().setScale(glm::vec3(25.0f, 25.0f, 1.0f));
	hud.energyBar = std::make_shared<EnergyBar>(10.0f, 1.0f);
	hud.energyBar->setValues(50, 50);
	energyBarNode = hudScene->add(hud.energyBar, false);
	energyBarNode->setPosition(glm::vec3(7.0f, 0, 0));

	hudScene->addOnProjectionChangedObserver([this] () { // Well this is not OK. or is it?, nope
		auto topRightCoord = hudScene->unproject(glm::vec3(1.0f, 1.0f, 0.0f));
		energyBarNode->setPosition(glm::vec3(topRightCoord.x - 6.0f, topRightCoord.y - 1.5f, 0));
	});
	addScene(hudScene);
}

void LevelGroup::loadLevel() {
	levelData = LevelLoader::getInstance().loadLevel(
			ResourceId(GameState::getInstance().getCurrentLevel()),
					*box2dScene, *bgScene);

	// Boxes...
//	auto b2Vecs = groundData.vecs;
//	auto boxMetas = levelGenerator.generateBoxes(
//			box2dScene->getBox2dWorld(), b2Vecs[1].x, b2Vecs[b2Vecs.size()-1].x, 12.0f, 20.0f, 0.5f, 5.0f, 200);
//	for (auto boxMeta : boxMetas) {
//		auto sprite = std::make_shared<Sprite>(ImageId(ResourceId("images/box.png")), boxMeta.size, boxMeta.size);
//		auto ro = box2dScene->attachToBox2dBody(boxMeta.body, sprite);
//
//		boxes[boxMeta.body] = std::unique_ptr<Box>(new Box(2.0f, ro));
//	}
}

void LevelGroup::loadRocketCar(bool updateWorldProperties) {
	auto loadData = box2dScene->loadBox2dData(rocketCarBuildData, b2Vec2(5.0, 0), updateWorldProperties);
	if (updateWorldProperties) {
		double viewWidth = rocketCarBuildData.getCustomProperty<double>("viewport_width", 25.0);
		box2dScene->getCamera().setScale(glm::vec3(viewWidth, viewWidth, 1.0));
		bgScene->getCamera().setScale(glm::vec3(viewWidth, viewWidth, 1.0));

		auto leftX = rocketCarBuildData.getCustomProperty<double>("actor_view_lower_x", -8.0);
		auto rightX = rocketCarBuildData.getCustomProperty<double>("actor_view_upper_x", 8.0);
		auto lowerY = rocketCarBuildData.getCustomProperty<double>("actor_view_lower_y", -8.0);
		auto upperY = rocketCarBuildData.getCustomProperty<double>("actor_view_upper_y", 8.0);

		box2dScene->setCameraDistanceToActor(leftX, rightX, lowerY, upperY);
		box2dScene->setCameraSpeed(25.0f*5/(60.0f)); // Five screen per second...
	}
	
	rocketCar.chassi = loadData.bodies[
			rocketCarBuildData.getCustomProperty<std::string>("actor", "actor")];
	rocketCar.wheel1 = loadData.bodies[
			rocketCarBuildData.getCustomProperty<std::string>("wheel1", "wheel1")];
	rocketCar.wheel2 = loadData.bodies[
			rocketCarBuildData.getCustomProperty<std::string>("wheel2", "wheel2")];
	rocketCar.roofFixture = loadData.fixtures[
			rocketCarBuildData.getCustomProperty<std::string>("roof_fixture", "roof_fixture")];

	// Copy fixtures from load data to rocketCar.fixtures
	rocketCar.fixtures.clear();
	transform(loadData.fixtures, std::back_inserter(rocketCar.fixtures), getter<1>{});

	rocketCar.maxRocketFuel = rocketCarBuildData.getCustomProperty<int>("max_fuel", 180);
	rocketCar.rocketFuel = rocketCar.maxRocketFuel;
	rocketCar.setCrashed(false);

	rocketCar.rocketEmitter = std::make_shared<ParticleEmitter>(rocketParticleGenerator, ResourceId("images/smoke.png"), 500);
	box2dScene->attachToBox2dBody(rocketCar.chassi, rocketCar.rocketEmitter);
	box2dScene->setActor(rocketCar.chassi);

	fadeIn();
}

void LevelGroup::onLoaded() {
	auto& audioPlayer = Director::getDirector().getAudioPlayer();
	bgMusic = audioPlayer.load(ResourceId("audio/caprice2.ogg"));
	bgMusicId = audioPlayer.play(bgMusic, true); // Repeat
}

void LevelGroup::onUnloaded() {
	auto& audioPlayer = Director::getDirector().getAudioPlayer();
	audioPlayer.stop(bgMusicId);
	audioPlayer.free(bgMusic);
}

void LevelGroup::solveRoofCollision(b2Contact const* contact) {
	b2Fixture const* other;
	if (contact->GetFixtureA() == rocketCar.roofFixture) {
		other = contact->GetFixtureB();
	} else if (contact->GetFixtureB() == rocketCar.roofFixture) {
		other = contact->GetFixtureA();
	}

	auto it = find(levelData.groundFixtures, other);
	if (it != levelData.groundFixtures.end()) {
		destroyCar();
	}
}

void LevelGroup::destroyCar() {
	if (rocketCar.hasCrashed()) {
		return;
	}

	rocketCar.setCrashed(true);
	auto explosionEmitter = std::make_shared<ParticleEmitter>(explosionParticleGenerator, ResourceId("images/smoke.png"), 500);

	auto explosionObject = box2dScene->attachToBox2dBody(rocketCar.chassi, explosionEmitter);
	explosionObject->move(glm::vec3(0, 0, 5));
	box2dScene->schedule([this, explosionObject]() {
		box2dScene->remove(explosionObject);
		return ticks::zero();
	}, seconds(5));

	explosionEmitter->start();

	box2dScene->scheduleAfterPhysicsUpdate([this]() {
		// TODO: Start explosion.
		auto& world = box2dScene->getBox2dWorld();
		auto joint = world.GetJointList();
		while (joint) {
			auto nextJoint = joint->GetNext();
			if (joint->GetBodyA() == rocketCar.chassi || joint->GetBodyB() == rocketCar.chassi) {
				world.DestroyJoint(joint);
			}
			joint = nextJoint;
		}

		box2dScene->schedule([this] () {
			// We should schedule this for later...
			loadRocketCar(false);
			updateHud();
			return ticks::zero();
		}, seconds(3));

		box2dScene->schedule([this] () {
			fadeOut();
			return ticks::zero();
		}, seconds(2));
	});
}

void LevelGroup::solveLevelBoundCollision(b2Contact const* contact) {
	b2Fixture const *otherFixture;
	if (contact->GetFixtureA() == levelData.borderFixture) {
		otherFixture = contact->GetFixtureB();
	} else if (contact->GetFixtureB() == levelData.borderFixture) {
		otherFixture = contact->GetFixtureA();
	} else {
		return;
	}

	if (find(rocketCar.fixtures, otherFixture) != rocketCar.fixtures.end()) {
		destroyCar();
	}
}

void LevelGroup::fadeOut() {
	auto steps = boost::chrono::duration_cast<ticks>(milliseconds(500)).count();
	auto stepSize = 1.4f/static_cast<float>(steps);

	box2dScene->setPostRenderer(postRenderer);
	bgScene->setPostRenderer(postRenderer);

	box2dScene->schedule([this, steps, stepSize] () mutable {
		auto outerRadius = stepSize * static_cast<float>(steps);
		auto innerRadius = std::max(0.0f, outerRadius - 0.3f);
		postRenderer->setRadius(innerRadius, outerRadius);

		--steps;
		if (steps >= 0) {
			return ticks(1);
		} else {
			return ticks::zero();
		}
	});
}

void LevelGroup::fadeIn() {
	auto steps = boost::chrono::duration_cast<ticks>(milliseconds(1000)).count();
	auto stepSize = 1.4f/static_cast<float>(steps);

	box2dScene->setPostRenderer(postRenderer);
	bgScene->setPostRenderer(postRenderer);

	box2dScene->schedule([this, steps, stepSize] () mutable {
		auto outerRadius = 1.4f - stepSize * static_cast<float>(steps);
		auto innerRadius = std::max(0.0f, outerRadius -0.3f);
		postRenderer->setRadius(innerRadius, outerRadius);

		--steps;
		if (steps > 0) {
			return ticks(1);
		} else {
			box2dScene->clearPostRenderer();
			bgScene->clearPostRenderer();
			
			return ticks::zero();
		}
	});
}

void LevelGroup::onPreSolveContact(b2Contact *contact, b2Manifold const*) {
	solveRoofCollision(contact);
	solveLevelBoundCollision(contact);
}

void LevelGroup::solveBoxCollision(b2Body *body, float impulse2) {
	if (levelData.boxes.find(body) != levelData.boxes.end()) {
		auto limit = levelData.boxes[body]->getDuration() * body->GetMass() * 10.0f;
		if (impulse2 > limit * limit) {
			auto explosionEmitter = std::make_shared<ParticleEmitter>(explosionParticleGenerator, ResourceId("images/smoke.png"), 500);
			auto bodyPos = body->GetPosition();
			auto explosionObject = box2dScene->add(explosionEmitter, true); //   attachToBox2dBody(body, explosionEmitter);
			explosionObject->setPosition(glm::vec3(bodyPos.x, bodyPos.y, 0));
			explosionEmitter->start();

			// Let's remove box and body in the next frame
			auto renderObject = levelData.boxes[body]->getRenderObject();
			levelData.boxes.erase(body);
			box2dScene->scheduleAfterPhysicsUpdate([this, body, renderObject]() {
//				LOGD(boost::format("BOOM %s") % body);
				box2dScene->removeAttachedObject(renderObject);
				box2dScene->getBox2dWorld().DestroyBody(body);
			});

			// Let's remove explosion after 5 seconds...
			box2dScene->schedule([this, explosionObject]() {
				box2dScene->remove(explosionObject);
				return ticks::zero();
			}, seconds(5));
		}
	}
}

void LevelGroup::onPostSolveContact(b2Contact* contact, const b2ContactImpulse* impulse) {
	float impulse2 = impulse->normalImpulses[0] * impulse->normalImpulses[0] +
			impulse->normalImpulses[1] * impulse->normalImpulses[1];

	auto bodyA = contact->GetFixtureA()->GetBody();
	auto bodyB = contact->GetFixtureB()->GetBody();
	solveBoxCollision(bodyA, impulse2);
	solveBoxCollision(bodyB, impulse2);
}

void LevelGroup::onContactBegin(b2Contact* contact) {
//	LOGD("Contact begin!!!");
	if (!rocketCar.hasCrashed()) {
		solveExitContact(contact);
	}
}

void LevelGroup::onContactEnd(b2Contact* contact) {
	// Empty for now...
}

void LevelGroup::solveExitContact(b2Contact const* contact) {
	b2Fixture const* exitFixture;
	b2Fixture const* otherFixture;
	if (contains_element(levelData.exitFixtures, contact->GetFixtureA())) {
		exitFixture = contact->GetFixtureA();
		otherFixture = contact->GetFixtureB();
	} else if (contains_element(levelData.exitFixtures, contact->GetFixtureB())) {
		exitFixture = contact->GetFixtureB();
		otherFixture = contact->GetFixtureA();
	} else {
		return; // No interesting sensor event.
	}

	if (contains_element(rocketCar.fixtures, otherFixture)) {
		levelComplete = true;
		if (!box2dScene->isPhysicsPaused()) {
			box2dScene->pausePhysics();
			Application::getApplication().schedule([this]() {
				// TODO: Director needs a replace scene group
				// possible with a transition.
				GameState::getInstance().nextLevel();
				Director::getDirector().removeSceneGroup(this);
				Director::getDirector().addSceneGroup(
						std::make_shared<LevelGroup>());
				return ticks::zero();
			}, seconds(5));
		}
	}
}

bool LevelGroup::onControllerEvent(rocket::input::ControllerEvent const& ce) {
	if (rocketCar.hasCrashed()) {
		return true;
	}

	if (ce.getButtonId() == button_id::BUTTON1) {
		if (ce.getValue() > 0) {
			rocketCar.applyGas = true;
		} else {
			rocketCar.applyGas = false;
		}
	} else if (ce.getButtonId() == button_id::BUTTON2) {
		if (ce.getValue() > 0) {
			rocketCar.applyRocket = true;
		} else {
			rocketCar.applyRocket = false;
		}
	} else if (ce.getButtonId() == button_id::DEBUG_RELOAD && ce.getValue() != 0) {
		LOGD("Reloading...");
		Director::getDirector().removeSceneGroup(this);
		Director::getDirector().addSceneGroup(std::make_shared<LevelGroup>());
	} else if (ce.getButtonId() == button_id::BUTTON_BACK && ce.getValue() != 0) {
		Director::getDirector().removeSceneGroup(this);
		Director::getDirector().addSceneGroup(std::make_shared<CarPickerGroup>());
	}

	return true;
}

void LevelGroup::updateHud() {
	hud.energyBar->setValues((50*rocketCar.rocketFuel)/rocketCar.maxRocketFuel, 50);
}

void LevelGroup::onUpdate() {
	// Update gas logic
	if (rocketCar.applyGas) {
		float actorMass = rocketCar.chassi->GetMass();
		float impulse = actorMass;

		rocketCar.wheel1->ApplyAngularImpulse(-impulse, true);
		rocketCar.wheel2->ApplyAngularImpulse(-impulse, true);
	}

	if (rocketCar.rocketFuel <= 0) {
		rocketCar.applyRocket = false;
	}

	// Update rocket logic.
	if (rocketCar.applyRocket) {
		float actorMass = rocketCar.chassi->GetMass();
		float angle = rocketCar.chassi->GetAngle();
		b2Vec2 force(-std::sin(angle), std::cos(angle));
		force *= actorMass * 30;
		rocketCar.chassi->ApplyForceToCenter(force, true);

		--rocketCar.rocketFuel;
		updateHud();

		if (!rocketCar.rocketEmitter->isStarted()) {
			rocketCar.rocketEmitter->start();
		}
	} else {
		if (rocketCar.rocketEmitter->isStarted()) {
			rocketCar.rocketEmitter->stop();
		}
	}
}

}
