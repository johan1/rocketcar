#include "Box2dLoader.h"

namespace rocket { namespace box2d {

LoadData loadBox2dData(b2World &world, Box2dParseData const& parseData, b2Vec2 const& offset, bool modifyWorldProperties) {
	LoadData loadData;

	if (modifyWorldProperties) {
		world.SetGravity( parseData.worldData.gravity );
		world.SetAllowSleeping( parseData.worldData.allowSleep );
		world.SetAutoClearForces( parseData.worldData.autoClearForces);
		world.SetWarmStarting( parseData.worldData.warmStarting);
		world.SetContinuousPhysics( parseData.worldData.continuousPhysics);
		world.SetSubStepping(parseData.worldData.subStepping); 
	}

	for (auto& bodyData : parseData.bodies) {
		auto body = world.CreateBody(&bodyData.bodyDef);
		for (auto& fixtureData : bodyData.fixtureDatas) {
			auto fixture = body->CreateFixture(&fixtureData.fixtureDef);
			loadData.fixtures[fixtureData.name] = fixture;
		}

		loadData.bodies[bodyData.name] = body;
		loadData.bodiesInLoadOrder.push_back(body);
	}

	for (auto& jointData : parseData.joints) {
		jointData.jointDef->bodyA = loadData.bodies[jointData.bodyA];
		jointData.jointDef->bodyB = loadData.bodies[jointData.bodyB];
		world.CreateJoint(jointData.jointDef.get());
	}

	// TODO: We should look to create joints ańd bodies in correct location to begin with...
	for (auto& body : loadData.bodiesInLoadOrder) {
		body->SetTransform(body->GetPosition() + offset, body->GetAngle());
	}

	return loadData;
}

}}
