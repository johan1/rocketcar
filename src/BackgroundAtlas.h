#ifndef _ROCKET_BACKGROUND_ATLAS_H_
#define _ROCKET_BACKGROUND_ATLAS_H_

#include <rocket/game2d/world/Sprite.h>
#include <rocket/resource/ResourceId.h>

#include <memory>
#include <random>

namespace rocketcar {

class BackgroundAtlas {
public:
	BackgroundAtlas(rocket::resource::ResourceId const& atlasId);

	std::shared_ptr<rocket::game2d::Sprite> getDirtPiece();
	std::shared_ptr<rocket::game2d::Sprite> getGroundPiece();
	std::shared_ptr<rocket::game2d::Sprite> getGroundSkyPiece();
	std::shared_ptr<rocket::game2d::Sprite> getSkyPiece();
	std::shared_ptr<rocket::game2d::Sprite> getSkySpacePiece();
	std::shared_ptr<rocket::game2d::Sprite> getSpacePiece();

private:
	rocket::resource::ResourceId atlasId;

	std::shared_ptr<rocket::game2d::Sprite> getPiece(uint32_t index);

	std::vector<uint32_t> dirtIndexes;
	std::vector<uint32_t> groundIndexes;
	std::vector<uint32_t> groundSkyIndexes;
	std::vector<uint32_t> skyIndexes;
	std::vector<uint32_t> skySpaceIndexes;
	std::vector<uint32_t> spaceIndexes;

	std::default_random_engine random_engine;
};

}

#endif
