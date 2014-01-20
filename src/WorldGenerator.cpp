#include "WorldGenerator.h"

#include <ctime>
#include <random>
#include <vector>

#include <rocket/util/Log.h>

namespace rocketcar {

static constexpr float angleInRadians(int32_t angle, int32_t resolution) {
	return angle * (M_PI/(resolution*2.0f));
};

/*
 * Valid angles lies within [-pi/2, pi/2], this interval is divided in, N+1 steps, where N := (resolution*2).
 * For i in [0, N], angle(0) = -pi/2, angle(N/2) = 0, angle(N) = pi/2.
 */

/*
 * previous is the previous angle index.
 * N is the 
 */
int32_t WorldGenerator::generateNextAngle(b2Vec2 const& currentPoint, int32_t currentAngle,
		TerrainParameters const& params) {
	std::bernoulli_distribution stepUp(params.heightFunction(currentPoint)); // Bernoulli distrubution to sample if angle should increase or decrease.
	std::bernoulli_distribution changeAngle(params.terrainVariance);

	if (stepUp(random_engine)) {
		if (currentAngle != params.resolution && changeAngle(random_engine)) {
			++currentAngle;
		}
	} else {
		if (currentAngle != -params.resolution && changeAngle(random_engine)) {
			--currentAngle;
		}
	}

	return currentAngle;
}

GroundMetaData WorldGenerator::generateTerrain(b2World &world) {
	return generateTerrain(world, TerrainParameters()); 
}

GroundMetaData WorldGenerator::generateTerrain(b2World &world, TerrainParameters const& params) {
	GroundMetaData groundData;

	random_engine.seed(params.seed);

//	std::vector<b2Vec2> groundPoints(params.steps);
	groundData.vecs.assign(params.steps, b2Vec2(0, 0));
	std::vector<b2Vec2> skyPoints(4);

	groundData.vecs[0] = b2Vec2(0, 0);
	// auto N = params.resolution * 2;
	// auto angle = N/2;
	int32_t angle = 0;

	skyPoints[0] = groundData.vecs[0];
	skyPoints[1].x = groundData.vecs[0].x;

	float height = skyPoints[0].y;
	for (uint32_t i = 1; i < params.steps; ++i) {
		angle = generateNextAngle(groundData.vecs[i-1], angle, params);

		groundData.vecs[i].x = groundData.vecs[i-1].x + std::cos(angleInRadians(angle, params.resolution)) * params.stepLength;
		groundData.vecs[i].y = groundData.vecs[i-1].y + std::sin(angleInRadians(angle, params.resolution)) * params.stepLength;
		
		height = std::max(groundData.vecs[i].y, height);
	}
	skyPoints[1].y = height + 100.0f;
	skyPoints[2].x = groundData.vecs[params.steps-1].x;
	skyPoints[2].y = height + 100.0f;
	skyPoints[3] = groundData.vecs[params.steps-1];

	// Create ground fixture
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody; 
		bodyDef.position = b2Vec2(0, 0);
		b2Body *body = world.CreateBody(&bodyDef);

		b2FixtureDef fixtureDef;
		fixtureDef.density = 1.0f;
		b2ChainShape chain;
		chain.CreateChain(&groundData.vecs[0], groundData.vecs.size());
		fixtureDef.shape = &chain;

		groundData.fixture = body->CreateFixture(&fixtureDef);
	}

	// Create sky limits, it should not be possible to fall off...
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody; 
		bodyDef.position = b2Vec2(0, 0);
		b2Body *body = world.CreateBody(&bodyDef);

		b2FixtureDef fixtureDef;
		fixtureDef.density = 1.0f;
		b2ChainShape chain;
		chain.CreateChain(&skyPoints[0], skyPoints.size());
		fixtureDef.shape = &chain;

		body->CreateFixture(&fixtureDef);
	}

	return groundData;
}

std::vector<BoxMetaData> WorldGenerator::generateBoxes(b2World &world, float x1, float x2, float y1, float y2, float s1, float s2, uint32_t number) {
	std::vector<BoxMetaData> boxes(number);

	// Generate boxes...
	std::uniform_real_distribution<float> xdist(x1, x2);
	std::uniform_real_distribution<float> ydist(y1, y2);
	std::uniform_real_distribution<float> sdist(s1, s2);

	b2BodyDef boxDef;
	boxDef.type = b2_dynamicBody;

	b2FixtureDef boxFixtureDef;
	boxFixtureDef.density = 1.0f;
	b2PolygonShape boxShape;

	for (uint32_t i = 0; i < number; ++i) {
		auto s = sdist(random_engine);
		boxShape.SetAsBox(s/2.0f, s/2.0f);
		boxFixtureDef.shape = &boxShape;

		boxDef.position = b2Vec2(xdist(random_engine), ydist(random_engine));
		auto body = world.CreateBody(&boxDef);
		body->CreateFixture(&boxFixtureDef);

		boxes[i] = { body, s };
	}

	return boxes;
}

}
