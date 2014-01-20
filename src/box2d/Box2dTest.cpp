#include "Box2dTest.h"

#include <Box2D/Box2D.h>

//! TODO: This class should be obsolete

using namespace rocket::util;

namespace rocket { namespace box2d {

bool isShapeOverlappingBody(b2Shape const& shape, b2Transform const&xf, b2Body const& body) {
	auto fixture = body.GetFixtureList();
	while (fixture != nullptr) {
		if(b2TestOverlap(&shape, 0, fixture->GetShape(), 0, xf, body.GetTransform())) {
			return true;
		}
		fixture = fixture->GetNext();
	}

	return false;
}

b2Body *findOverlappingBody(glm::vec3 const& worldCoord, float radius, b2World &world) {
	b2CircleShape area;
	area.m_p.Set(worldCoord.x, worldCoord.y);
	area.m_radius = radius;

	auto body = world.GetBodyList();

	b2Transform xf;
	xf.SetIdentity();
	while (body != nullptr) {
		if (isShapeOverlappingBody(area, xf, *body)) {
			return body;
		}
		body = body->GetNext();
	}

	return nullptr;
}

b2Body* findStaticBody(b2World& world) {
	auto body = world.GetBodyList();
	while (body != nullptr) {
		if (body->GetType() == b2BodyType::b2_staticBody) {
			return body;
		}
		body = body->GetNext();
	}

	return nullptr;
}

AABox calculateBoundingBox(b2Vec2 const *vertices, int count) {
	Point lowerLeft = createPoint(vertices[0].x, vertices[0].y);
	Point upperRight = createPoint(vertices[0].x, vertices[0].y);

	for (int i = 1; i < count; ++i) {
		lowerLeft.x = std::min(vertices[i].x, lowerLeft.x);
		lowerLeft.y = std::min(vertices[i].y, lowerLeft.y);

		upperRight.x = std::max(vertices[i].x, upperRight.x);
		upperRight.y = std::max(vertices[i].y, upperRight.y);
	}

	return AABox(lowerLeft, upperRight);
}

}}
