#include "JsonLoader.h"

#include <rocket/util/Log.h>


using namespace rocket::game2d;

namespace rocketcar {

json::value loadJson(rocket::resource::ResourceId const& resourceId) {
	auto resources = Director::getDirector().getResources();
	auto is = resources.openResource(resourceId);
	try {
		return json::parse(*is);
	} catch (std::exception &e) {
		LOGE(boost::format("Failed to parse json resource=%s, error=%s") %
				resourceId % e.what());
		throw;
	}
}

}
