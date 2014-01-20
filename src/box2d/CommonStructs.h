#ifndef _ROCKET_COMMON_BOX2D_STRUCTS_H_
#define _ROCKET_COMMON_BOX2D_STRUCTS_H_

#include <Box2D/Box2D.h>

#include <boost/any.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rocket { namespace box2d {

struct ImageParseData {
	std::string name;
	float opacity;
	int renderOrder;
	float scale;
	float angle;
	int bodyId; // -1 means no body
	b2Vec2 center;
	std::vector<b2Vec2> corners;
	std::string file;
	int filter;
	bool flip;
	unsigned char colorTint[4];
	std::unordered_map<std::string, boost::any> customProperties;
};

struct FixtureParseData {
	std::string name;
	b2FixtureDef fixtureDef;
	std::unique_ptr<b2Shape> shape;
};

struct BodyParseData {
	struct MassData {
		double mass;
		b2Vec2 center;
		double I;
	};
	// b2Body *body;

	std::string name;
	b2BodyDef bodyDef;
	std::vector<FixtureParseData> fixtureDatas;
	MassData massData;
};

struct JointParseData {
//	b2Joint *joint;
	std::unique_ptr<b2JointDef> jointDef;
	std::string name;
	std::string bodyA;
	std::string bodyB;
	std::string type;
};

struct WorldParseData {
	b2Vec2 gravity;
	bool allowSleep;
	bool autoClearForces;
	bool warmStarting;
	bool continuousPhysics;
	bool subStepping;
};

// Rename to RubeData
struct Box2dParseData {
	int positionIterations;
	int velocityIterations;
	int stepsPerSecond;

	WorldParseData worldData;

	std::vector<BodyParseData> bodies;
	std::vector<JointParseData> joints;
	std::vector<ImageParseData> images;
	std::unordered_map<std::string, boost::any> customProperties;

	template <typename T>
	T getCustomProperty (std::string const& name, T const& defaultValue) {
		if (customProperties.find(name) != customProperties.end()) {
			return boost::any_cast<T>(customProperties[name]);
		} else {
			return defaultValue;
		}
	}

	bool hasCustomProperty(std::string const& name) {
		return customProperties.find(name) != customProperties.end();
	}
};

}}

#endif
