#include "Ground.h"
#include <rocket/util/Log.h>

using namespace rocket::glutils;
using namespace rocket::graphics;

namespace rocketcar {

Ground::Ground(std::vector<b2Vec2> const& b2Vecs, float coordRelation, rocket::resource::image::ImageId const& bgImage) : 
		b2Vecs(b2Vecs), coordRelation(coordRelation), bgImage(bgImage) {
/*
	auto v_min = std::min_element(b2Vecs.begin(), b2Vecs.end(), [] (b2Vec2 const& v1, b2Vec2 const& v2) {
		return v1.y < v2.y;
	});

//	lower_y = v_min->y - 1.0f;
*/
}

// TODO: Assumes anchored in 0,0,0 with no rotation... We should fix this... Later...
void Ground::renderImpl(Canvas &canvas) {
	auto& aabb = canvas.getAABB();

	auto lower_y = aabb.p1.y;

	// Find index of first ground piece to draw..
	// Note: Assuming ground object is anchored in 0,0 and not rotated...

	uint32_t vecSize = b2Vecs.size();

	uint32_t i = 0;
	for (; i < (vecSize-1) && b2Vecs[i+1].x < aabb.getP1().x; ++i);

	auto& textureManager = canvas.getTextureManager();
	auto& texture = textureManager.getTexture(bgImage.getResourceId());
//	texture.setParameters(GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texture.setParameters(GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
//	TextureData(bgImage);

	// Let's draw...
	for (; i < vecSize - 1 && b2Vecs[i].x < aabb.getP2().x; ++i) {
		if (b2Vecs[i+1].x <= b2Vecs[i].x) {
			continue;
		}

		vertices.clear();

		vertices.push_back( Vertex6d(b2Vecs[i].x, std::min(lower_y, b2Vecs[i].y), coordRelation * b2Vecs[i].x, coordRelation * lower_y) );
		vertices.push_back( Vertex6d(b2Vecs[i+1].x, std::min(lower_y, b2Vecs[i+1].y), coordRelation * b2Vecs[i+1].x, coordRelation * lower_y) );
		vertices.push_back( Vertex6d(b2Vecs[i+1].x, b2Vecs[i+1].y, coordRelation * b2Vecs[i+1].x, coordRelation * b2Vecs[i+1].y) );
		vertices.push_back( Vertex6d(b2Vecs[i].x, b2Vecs[i].y, coordRelation * b2Vecs[i].x, coordRelation * b2Vecs[i].y) );

//		td.wrapS = GL_MIRRORED_REPEAT;
		// td.wrapT = GL_MIRRORED_REPEAT;

		canvas.drawTexture(vertices, texture, false);
	}
}

}
