#ifndef ROCKET_GROUND_H_
#define ROCKET_GROUND_H_

#include <Box2D/Box2D.h>
#include <rocket/resource/image/ImageId.h>
#include <rocket/glutils/GLUtils.h>
#include <rocket/graphics/Canvas.h>
#include <rocket/game2d/world/Renderable.h>
#include <vector>

namespace rocketcar {

class Ground : public rocket::game2d::Renderable{
public:
	// 
	Ground(std::vector<b2Vec2> const& b2Vecs, float coordRelation, rocket::resource::image::ImageId const& bgImage);

private:
	std::vector<b2Vec2> b2Vecs;

	float coordRelation;
//	float lower_y;
	rocket::resource::image::ImageId bgImage;
	std::vector<rocket::glutils::Vertex6d> vertices;

	virtual void renderImpl(rocket::graphics::Canvas &canvas);
};

}

#endif
