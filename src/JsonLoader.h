#ifndef _ROCKET_JSON_LOADER_H_
#define _ROCKET_JSON_LOADER_H_

#include <rocket/game2d/Director.h>
#include <rocket/resource/ResourceId.h>
#include <cppjson/json.h>

namespace rocketcar {

json::value loadJson(rocket::resource::ResourceId const& resourceId);

};

#endif
