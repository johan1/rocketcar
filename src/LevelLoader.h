#ifndef ROCKET_LEVEL_LOADER_H
#define ROCKET_LEVEL_LOADER_H

#include "ParallaxScene.h"
#include "world/Box2dScene.h"
#include "world/Box.h"

#include <istream>
#include <memory>
#include <unordered_map>
#include <vector>

#include <Box2D/Box2D.h>
#include <rocket/resource/ResourceId.h>

#include <json/json.h>

namespace rocketcar {

// TODO: Rename when we find an appropriate name.
// Currently this contains ground fixtures so that level group is able to determine
// if roof collides with any fixture marked as ground.
struct LevelData {
	std::vector<b2Fixture*> groundFixtures;
	std::vector<b2Fixture*> exitFixtures;
	std::unordered_map<b2Body*, std::unique_ptr<Box>> boxes;
	b2Fixture *borderFixture;
};

class LevelLoader {
public:
	static LevelLoader& getInstance();
	LevelData loadLevel(rocket::ResourceId const& levelResource, Box2dScene& box2dScene, ParallaxScene& bgScene);

private:
	LevelLoader();

	LevelData load(Box2dScene& scene, rocket::ResourceId const& levelResource);
	void addGround(LevelData &ld, Box2dScene& scene, Json::Value const& value);
	void addExit(LevelData &ld, Box2dScene& scene, Json::Value const& value);

	LevelData deprecatedGenerator(Box2dScene& box2dScene);
};

}

#endif
