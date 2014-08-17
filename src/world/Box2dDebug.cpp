#include "Box2dDebug.h"

#include <rocket/game2d/world/Camera.h>
#include <rocket/Log.h>

#include <Box2D/Box2D.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <rocket/util/Geometry.h>
#include <rocket/glutils/GLUtils.h>

#include <glm/gtc/matrix_inverse.hpp>

// This class can be refactored, so that we don't implement the same functionality twice.
using namespace rocket::glutils;
using namespace rocket::game2d;
using namespace rocket::graphics;
using namespace rocket::util;

namespace rocketcar {

static glm::vec4 b2Vec2ToGlmVec4(b2Vec2 const& vec) {
	glm::vec4 glmVec(0.0f);

	glmVec[0] = vec.x;
	glmVec[1] = vec.y;
	glmVec[2] = 0.0f;
	glmVec[3] = 1.0f;

	return glmVec;
}

static RGBAColor b2ColorToRGBAColor(b2Color const& color) {
	RGBAColor rgbaColor;
	rgbaColor.r = color.r;
	rgbaColor.g = color.g;
	rgbaColor.b = color.b;
	rgbaColor.a = 1.0f;

	return rgbaColor;
}

bool Box2dDebug::verticesInView(const b2Vec2* vertices, int32 vertexCount) {
	Point p1 = createPoint(vertices[0].x, vertices[0].y);
	Point p2 = createPoint(vertices[0].x, vertices[0].y);
	for (int i = 1; i < vertexCount; ++i) {
		p1.x = std::min(p1.x, vertices[i].x);
		p1.y = std::min(p1.y, vertices[i].y);
		p2.x = std::max(p2.x, vertices[i].x);
		p2.y = std::max(p2.y, vertices[i].y);
	}
	AABox pbox(p1, p2);

	AABox vbox = AABox(
			createPoint(gameView[0], gameView[2]), createPoint(gameView[1], gameView[3]));

	if (vbox.intersects(pbox)) {
		++drawCount;
//		LOGD("PASS!");
		return true;
	} else {
		++ignoreCount;
//		LOGD("FAIL!");
		return false;
	}
}

bool Box2dDebug::circleInView(const b2Vec2& center, float radius) {
	b2Vec2 arr[2];
	arr[0] = b2Vec2(center.x - radius, center.y - radius);
	arr[1] = b2Vec2(center.x + radius, center.y + radius);

	return verticesInView(arr, 2);
}

void Box2dDebug::preparePolygon(const b2Vec2* vertices, int32 vertexCount) {
	glmVertices.clear();
	for (int i = 0; i < vertexCount; ++i) {
		glmVertices.push_back(b2Vec2ToGlmVec4(vertices[i]));
	}
}

/// Draw a closed polygon provided in CCW order.
void Box2dDebug::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	if (!verticesInView(vertices, vertexCount)) {
		return;
	}
	preparePolygon(vertices, vertexCount);

	RGBAColor rgbaColor = b2ColorToRGBAColor(color);
	canvas->drawPolygon(glmVertices, rgbaColor);
}

/// Draw a solid closed polygon provided in CCW order.
void Box2dDebug::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	if (!verticesInView(vertices, vertexCount)) {
		return;
	}

	preparePolygon(vertices, vertexCount);

	RGBAColor rgbaColor = b2ColorToRGBAColor(color);
	rgbaColor.a = alpha;
	canvas->fillPolygon(glmVertices, rgbaColor);
	rgbaColor.a = 1.0f;
	canvas->drawPolygon(glmVertices, rgbaColor);
}


void Box2dDebug::prepareCircle(const b2Vec2& center, float32 radius, int segments) {
	glmVertices.clear();
	b2Vec2 point;
	glmVertices.push_back(b2Vec2ToGlmVec4(center));
	for (int i = 0; i <= segments; ++i) {
		float angle = static_cast<float>(i) * (2.0f*M_PI)/static_cast<float>(segments);
 
		point.x = center.x + radius * std::sin(angle);
		point.y = center.y + radius * std::cos(angle);
		glmVertices.push_back(b2Vec2ToGlmVec4(point));
	}
}

/// Draw a circle.
void Box2dDebug::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {
	if (!circleInView(center, radius)) {
		return;
	}
	RGBAColor rgbaColor;
	rgbaColor.r = color.r;
	rgbaColor.g = color.g;
	rgbaColor.b = color.b;
	rgbaColor.a = 1.0f; // 1.0f;

	prepareCircle(center, radius, 20);

	canvas->drawPolygon(glmVertices, rgbaColor);
}

/// Draw a solid circle.
void Box2dDebug::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
	if (!circleInView(center, radius)) {
		return;
	}
	prepareCircle(center, radius, 20);

	RGBAColor rgbaColor = b2ColorToRGBAColor(color);
	rgbaColor.a = alpha; // 1.0f;
	canvas->fillPolygon(glmVertices, rgbaColor);
	rgbaColor.a = 1.0f;
	canvas->drawPolygon(glmVertices, rgbaColor);
	DrawSegment(center, center + radius * axis, color);
}

/// Draw a line segment.
void Box2dDebug::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
	b2Vec2 arr[2];
	arr[0] = p1;
	arr[1] = p2;
	if (!verticesInView(arr, 2)) {
		return;
	}

	glmVertices.clear();
	glmVertices.push_back(b2Vec2ToGlmVec4(p1));
	glmVertices.push_back(b2Vec2ToGlmVec4(p2));

	RGBAColor rgbaColor = b2ColorToRGBAColor(color);
	canvas->drawSegment(glmVertices[0], glmVertices[1], rgbaColor);
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void Box2dDebug::DrawTransform(const b2Transform& xf) {
	b2Vec2 p1 = xf.p;
	b2Vec2 p2;
	float k_axisScale = 0.4f;

	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	DrawSegment(p1, p2, b2Color(1, 0, 0));

	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	DrawSegment(p1, p2, b2Color(0, 1, 0));
}

void Box2dDebug::renderImpl(rocket::graphics::Canvas &canvas) {
	this->canvas = &canvas;
	
	drawCount = 0;
	ignoreCount = 0;

	auto& mvpMatrix = canvas.getMvpMatrix();
	auto mvpInverse = glm::inverse(mvpMatrix); 

	auto lowerLeft = mvpInverse * glm::vec4(-1, -1, 0, 1);
	auto upperRight = mvpInverse * glm::vec4(1, 1, 0, 1);

	gameView = glm::vec4(lowerLeft.x, upperRight.x, lowerLeft.y, upperRight.y);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false);

	// Well I guess we'll never ever have more than one world anyhow...
	// debug drawing really should mutable.
	world->SetDebugDraw(this); 
	world->DrawDebugData();

	glDisable(GL_BLEND);
	glDepthMask(true);
}

}
