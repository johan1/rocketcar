#include "LevelLoader.h"

#include "BackgroundAtlas.h"
#include <rocket/resource/ResourceId.h>
#include <json/json.h>

// TODO: Below includes should be removed when deprecating the world generator.
#include "JsonLoader.h"
#include "world/Ground.h"
#include "WorldGenerator.h"
#include "world/Box2dDebug.h"

namespace rocketcar {

using namespace rocket;

LevelLoader::LevelLoader() {}

LevelLoader& LevelLoader::getInstance() {
	static LevelLoader loader;
	return loader;
}

LevelData LevelLoader::loadLevel(Box2dScene& box2dScene, ParallaxScene& bgScene) {
	ResourceId level("level1.json");
	
	// TODO: We should check theme and load appropriate atlas, if no theme let's enable box2d rendering
	auto b2RenderObject = box2dScene.add(std::make_shared<Box2dDebug>(&box2dScene.getBox2dWorld()), false);
	b2RenderObject->move(glm::vec3(0,0,-1));
	return load(box2dScene, level);
}

LevelData LevelLoader::load(Box2dScene &scene, ResourceId const& levelResource) {
    Json::Value root = loadJson(levelResource);

	// Create border
	{
		float height = root["height"].asDouble();
		float width = root["width"].asDouble();

		std::array<b2Vec2, 5> limits;
		limits[0] = b2Vec2{0, -height/2.0f};
		limits[1] = b2Vec2{width, -height/2.0f};
		limits[2] = b2Vec2{width, height/2.0f};
		limits[3] = b2Vec2{0, height/2.0f};
		limits[4] = limits[0];

		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody; 
		bodyDef.position = b2Vec2(0, 0);
		b2Body *body = scene.getBox2dWorld().CreateBody(&bodyDef);

		b2FixtureDef fixtureDef;
		fixtureDef.density = 1.0f;
		b2ChainShape chain;
		chain.CreateChain(limits.data(), limits.size());
		fixtureDef.shape = &chain;

		body->CreateFixture(&fixtureDef);
	}

	LevelData ld;
	auto &tilesArr = root["tiles"];
	for (auto &tile : tilesArr) {
		if (tile["name"].asString().find("ground") == 0) {
			addGround(ld, scene, tile);
		}
	}

	return ld;
}

// TODO Maybe we should factor out the ground loaders.
void LevelLoader::addGround(LevelData &ld, Box2dScene &scene, Json::Value const& groundValue) {
	auto name = groundValue["name"].asString();
	auto x = groundValue["x"].asDouble();
	auto y = groundValue["y"].asDouble();

	b2BodyDef bodyDef;
	bodyDef.position = b2Vec2(x, y);
	bodyDef.type = b2_staticBody;

	b2FixtureDef groundFixtureDef;
	groundFixtureDef.density = 1.0;
	b2PolygonShape shape;
	b2Vec2 vecs[4];

	if (name == "ground00") {
		vecs[0].Set(0, 0);
		vecs[1].Set(1, 0);
		vecs[2].Set(1, 1);
		vecs[3].Set(0, 1);
		shape.Set(vecs, 4);
	} else if (name == "ground11+") {
		vecs[0].Set(0, 0);
		vecs[1].Set(1, 0);
		vecs[2].Set(1, 1);
		shape.Set(vecs, 3);
	} else if (name == "ground11-") {
		vecs[0].Set(0, 0);
		vecs[1].Set(1, 0);
		vecs[2].Set(0, 1);
		shape.Set(vecs, 3);
	} else if (name == "ground12+") {
		vecs[0].Set(0, 0);
		vecs[1].Set(1, 0);
		vecs[2].Set(1, 2);
		shape.Set(vecs, 3);
	} else if (name == "ground12-") {
		vecs[0].Set(0, 0);
		vecs[1].Set(1, 0);
		vecs[2].Set(0, 2);
		shape.Set(vecs, 3);
	} else if (name == "ground21+") {
		vecs[0].Set(0, 0);
		vecs[1].Set(2, 0);
		vecs[2].Set(2, 1);
		shape.Set(vecs, 3);
	} else if (name == "ground21-") {
		vecs[0].Set(0, 0);
		vecs[1].Set(2, 0);
		vecs[2].Set(0, 1);
		shape.Set(vecs, 3);
	}
	groundFixtureDef.shape = &shape;

	LOGD("Adding box at (" << bodyDef.position.x << ", " << bodyDef.position.y << ")");
	auto body = scene.getBox2dWorld().CreateBody(&bodyDef);
	auto fixture = body->CreateFixture(&groundFixtureDef);
	ld.groundFixtures.push_back(fixture);
}

LevelData LevelLoader::deprecatedGenerator(Box2dScene& box2dScene) {
	WorldGenerator levelGenerator;
	auto groundData = levelGenerator.generateTerrain(box2dScene.getBox2dWorld());

	LevelData ld;
	ld.groundFixtures.push_back(groundData.fixture);

	auto b2Vecs = groundData.vecs;
	auto boxMetas = levelGenerator.generateBoxes(
			box2dScene.getBox2dWorld(), b2Vecs[1].x, b2Vecs[b2Vecs.size()-1].x, 12.0f, 20.0f, 0.5f, 5.0f, 200);
	for (auto boxMeta : boxMetas) {
		// TODO We shoul lookup image id from theme atlas
		auto sprite = std::make_shared<Sprite>(ImageId(ResourceId("images/box.png")), boxMeta.size, boxMeta.size);
		auto ro = box2dScene.attachToBox2dBody(boxMeta.body, sprite);
		ld.boxes[boxMeta.body] = std::unique_ptr<Box>(new Box(2.0f, ro));
	}

	return ld;
	// TODO: We need the ground fixtures to be able to detect roof collisions.
	// groundFixtures.push_back(groundData.fixture);
}

}
