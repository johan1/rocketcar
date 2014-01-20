#include "BackgroundAtlas.h"
#include <ctime>

using namespace rocket::game2d;
using namespace rocket::resource;
using namespace rocket::resource::image;


namespace rocketcar {

BackgroundAtlas::BackgroundAtlas(rocket::resource::ResourceId const& atlasId) :
		atlasId(atlasId) {
	random_engine.seed(std::time(nullptr));

	dirtIndexes.push_back(0);
	groundIndexes.push_back(1);
	groundIndexes.push_back(5);
	groundIndexes.push_back(9);
	groundSkyIndexes.push_back(2);
	groundSkyIndexes.push_back(6);
	skyIndexes.push_back(3);
	skyIndexes.push_back(7);
	skyIndexes.push_back(11);
	skyIndexes.push_back(15);
	skySpaceIndexes.push_back(10);
	skySpaceIndexes.push_back(14);
	spaceIndexes.push_back(4);
	spaceIndexes.push_back(8);
	spaceIndexes.push_back(12);
	spaceIndexes.push_back(13);
}

std::shared_ptr<Sprite> BackgroundAtlas::getDirtPiece() {
	std::uniform_int_distribution<uint32_t> dist(0, dirtIndexes.size()-1);
	return getPiece(dirtIndexes[dist(random_engine)]);
}

std::shared_ptr<Sprite> BackgroundAtlas::getGroundPiece() {
	std::uniform_int_distribution<uint32_t> dist(0, groundIndexes.size()-1);
	return getPiece(groundIndexes[dist(random_engine)]);
}

std::shared_ptr<Sprite> BackgroundAtlas::getGroundSkyPiece() {
	std::uniform_int_distribution<uint32_t> dist(0, groundSkyIndexes.size()-1);
	return getPiece(groundSkyIndexes[dist(random_engine)]);
}

std::shared_ptr<Sprite> BackgroundAtlas::getSkyPiece() {
	std::uniform_int_distribution<uint32_t> dist(0, skyIndexes.size()-1);
	return getPiece(skyIndexes[dist(random_engine)]);
}

std::shared_ptr<Sprite> BackgroundAtlas::getSkySpacePiece() {
	std::uniform_int_distribution<uint32_t> dist(0, skySpaceIndexes.size()-1);
	return getPiece(skySpaceIndexes[dist(random_engine)]);
}

std::shared_ptr<Sprite> BackgroundAtlas::getSpacePiece() {
	std::uniform_int_distribution<uint32_t> dist(0, spaceIndexes.size()-1);
	return getPiece(spaceIndexes[dist(random_engine)]);
}

std::shared_ptr<rocket::game2d::Sprite> BackgroundAtlas::getPiece(uint32_t index) {
	return std::make_shared<Sprite>(ImageId(atlasId, static_cast<float>(index/4) * 0.25f, static_cast<float>(index%4) * 0.25f, 0.25f, 0.25f), 4.0f, 4.0f);
}

}
