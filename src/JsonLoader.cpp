#include "JsonLoader.h"

#include <rocket/Log.h>

using namespace rocket::game2d;

namespace rocketcar {

Json::Value loadJson(rocket::resource::ResourceId const& resourceId) {
	auto resources = Director::getDirector().getResources();
	auto is = resources.openResource(resourceId);
	try {
		Json::Value value;
		Json::Reader reader;
		reader.parse(*is, value);
		return value;
	} catch (std::exception &e) {
		LOGE("Failed to parse json resource=" << resourceId <<", error=" << e.what());
		throw;
	}
}

}
