#ifndef _WORLD_GENERATOR_H_
#define _WORLD_GENERATOR_H_

#include <Box2D/Box2D.h>

#include <ctime>
#include <functional>
#include <random>
#include <vector>

namespace rocketcar {

struct TerrainParameters {
	uint32_t steps; // valid
	int32_t resolution;
	float stepLength; // valid
	double terrainVariance; // (0-1) // valid.
	size_t seed;

	//! Should return the probability given current point that terrain height should incresase.
	std::function<double(b2Vec2 const&)> heightFunction;

	TerrainParameters() : steps(2000), resolution(20), stepLength(2.5), terrainVariance(0.9) {
		seed = std::time(nullptr);
		heightFunction = [](b2Vec2 const& point) -> double {
			const float maxHeight = 10;
			return (maxHeight-point.y)/(2*maxHeight);
		};
	}
};

struct BoxMetaData {
	b2Body* body;
	float size;
};

struct GroundMetaData {
	std::vector<b2Vec2> vecs;
	b2Fixture* fixture;
};

class WorldGenerator {
public:
	// requirements resolution > 0, steps > 0, stepSize > 0
	GroundMetaData generateTerrain(b2World &world);

	// requirements resolution > 0, steps > 0, stepSize > 0
//	std::vector<b2Vec2> generateTerrain(b2World &world, TerrainParameters const& params);
	GroundMetaData generateTerrain(b2World &world, TerrainParameters const& params);

	std::vector<BoxMetaData> generateBoxes(b2World &world, float x1, float x2, float y1, float y2, float s1, float s2, uint32_t number);

private:
	int32_t generateNextAngle(b2Vec2 const& currentPoint, int32_t currentAngle, TerrainParameters const& params);
	std::default_random_engine random_engine;
};

}

#endif
