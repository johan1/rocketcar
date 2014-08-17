#ifndef _ROCKET_JSON_LOADER_H_
#define _ROCKET_JSON_LOADER_H_

#include <rocket/game2d/Director.h>
#include <rocket/resource/ResourceId.h>
#include <json/json.h>

namespace rocketcar {

Json::Value loadJson(rocket::resource::ResourceId const& resourceId);

};

#endif
