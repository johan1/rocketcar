#ifndef _WORLD_H_
#define _WORLD_H_

namespace rocketcar {

struct World {
	std::vector<b2Vec2> ground;
	std::vector<b2Vec2> sky;
};

}

#endif
