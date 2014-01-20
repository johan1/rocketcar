#ifndef _ROCKET_BOX_H_
#define _ROCKET_BOX_H_

//#include <rocket/resource/image/ImageId.h>
#include <rocket/game2d/world/RenderObject.h>
//#include <memory>
//#include <Box2D/Box2D.h>

class Box {
public:
//	Box(b2Body* body, std::shared_ptr<rocket::game2d::Sprite> const& sprite);
//	Box() : duration(1.0f, std::shared_ptr<rocket::game2d::Sprite> const& sprite) :
//			sprite(sprite) {}
	Box(float duration, rocket::game2d::RenderObject *ro) :
			duration(duration), ro(ro) {}

	float getDuration() const { return duration; }
	rocket::game2d::RenderObject* getRenderObject() const { return ro; }

private:
	float duration;
	rocket::game2d::RenderObject* ro;
};

#endif
