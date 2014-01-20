#ifndef _ROCKETCAR_BOX2D_TEST_H_
#define _ROCKETCAR_BOX2D_TEST_H_

#include <glm/glm.hpp>
#include <Box2D/Box2D.h>
#include <vector>
#include <rocket/util/Geometry.h>
#include <cmath>

namespace rocket { namespace box2d {

bool isShapeOverlappingBody(b2Shape const& shape, b2Transform const&xf, b2Body const& body);

b2Body const* findOverlappingBody(glm::vec3 const& worldCoord, float radius, b2World const& world);

template <typename BodyFilter>
b2Body const* findOverlappingBody(glm::vec3 const& worldCoord, float radius, b2World const& world, BodyFilter &&filter);

// Does this qualify? Used for finding ground, i.e. a static body that exists throughout the lifecycle of the b2world.
b2Body* findStaticBody(b2World &world);

rocket::util::AABox calculateBoundingBox(b2Vec2 const *vertices, int count);

// Template implementations
template <typename BodyFilter>
b2Body* findOverlappingBody(glm::vec3 const& worldCoord, float radius, b2World &world,
		BodyFilter &&filter) {

	std::function<bool(b2Body const&)> filterFunction;
	if (std::is_rvalue_reference<BodyFilter&&>::value) {
		filterFunction = std::move(filter);
	} else {
		filterFunction = filter;
	}

	b2CircleShape area;
	area.m_p.Set(worldCoord.x, worldCoord.y);
	area.m_radius = radius;

	auto body = world.GetBodyList();

	b2Transform xf;
	xf.SetIdentity();
	while (body != nullptr) {
		if (filterFunction(*body) && isShapeOverlappingBody(area, xf, *body)) {
			return body;
		}
		body = body->GetNext();
	}
	return nullptr;
}

}}

#endif
