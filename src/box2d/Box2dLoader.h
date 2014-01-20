#ifndef _ROCKET_BOX2D_LOADER_H_
#define _ROCKET_BOX2D_LOADER_H_

#include "CommonStructs.h"

namespace rocket { namespace box2d {

struct LoadData {
	std::unordered_map<std::string, b2Body*> bodies; // Joints refer to bodies by name.
	std::vector<b2Body*> bodiesInLoadOrder; // Images refer to bodies in load order.
	std::unordered_map<std::string, b2Body*> joints; // It's nice to have joints.
	std::unordered_map<std::string, b2Fixture*> fixtures; // It's nice to have joints.
};

LoadData loadBox2dData(b2World &world, Box2dParseData const& box2dData, b2Vec2 const& offset, bool modifyWorldProperties);

}};

#endif
