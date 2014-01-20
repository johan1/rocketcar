#ifndef _ROCKET_PARALLAX_SCENE_H_
#define _ROCKET_PARALLAX_SCENE_H_

#include <memory>
#include <rocket/game2d/world/Scene.h>

namespace rocketcar {
class ParallaxScene : public rocket::game2d::Scene {
public:
	ParallaxScene(rocket::game2d::SceneObject const* camera);

private:
	rocket::game2d::SceneObject const* followCamera;
//	std::shared_ptr<rocket::game2d::Node> followCamera;
	virtual void updateImpl();
};

}
#endif
