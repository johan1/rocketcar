#ifndef _ROCKETCAR_BOX2D_DEBUG_H_
#define _ROCKETCAR_BOX2D_DEBUG_H_

#include <Box2D/Common/b2Draw.h>
#include <Box2D/Dynamics/b2World.h>

#include <rocket/graphics/Canvas.h>
#include <rocket/game2d/world/Renderable.h>

#include <glm/glm.hpp>

#include <vector>

namespace rocketcar {

class Box2dDebug : public rocket::Renderable, private b2Draw {
public:
	Box2dDebug(b2World* world) : world(world), alpha(0.25f) {
		SetFlags(e_shapeBit | e_jointBit | e_aabbBit | e_pairBit | e_centerOfMassBit);
	}

private:
	b2World* world;
	rocket::graphics::Canvas *canvas;

	int drawCount;
	int ignoreCount;

	glm::vec4 gameView;

	float alpha;

	std::vector<glm::vec4> glmVertices;

	virtual void renderImpl(rocket::graphics::Canvas &canvas);

	// Fills glmVertices with vertices of a polygon
	bool verticesInView(const b2Vec2* vertices, int32 vertexCount);

	bool circleInView(const b2Vec2& center, float radius);

	// Fills glmVertices with vertices of a polygon
	void preparePolygon(const b2Vec2* vertices, int32 vertexCount);

	/// Draw a closed polygon provided in CCW order.
	virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

	/// Draw a solid closed polygon provided in CCW order.
	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);

	// Fills glmVertices with vertices of a circle
	void prepareCircle(const b2Vec2& center, float32 radius, int segments);

	/// Draw a circle.
	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
	
	/// Draw a solid circle.
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
	
	/// Draw a line segment.
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);

	/// Draw a transform. Choose your own length scale.
	/// @param xf a transform.
	virtual void DrawTransform(const b2Transform& xf);
};

}

#endif
