#include "Box2dScene.h"
#include "Box2dDebug.h"
#include "../box2d/Box2dTest.h"
#include "../box2d/RubeParser.h"

#include <rocket/game2d/Director.h>

#include <rocket/util/Log.h>
#include <rocket/game2d/world/Sprite.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Box2D/Box2D.h>

#include <iterator>

#include <cppjson/json.h>

#include <cmath>

static const float DEBUG_Z_ORDER 	= -99.9f;
//static const float SPRITE_Z_ORDER 	= 0.0f;

using namespace rocket::game2d;
using namespace rocket::input;
using namespace rocket::resource;
using namespace rocket::resource::image;
using namespace rocket::util;
using namespace rocket::glutils;
using namespace rocket::box2d;

namespace rocketcar {
std::function<glm::mat4(glm::vec4 const&)> gameProjectionFunction() {
	return [](glm::vec4 const& viewPort) -> glm::mat4 {
		float sw = viewPort[2];
		float sh = viewPort[3];

		// Scaling is performed on the view matrix.
		float ww = 1.0f; 

		// Height is varying depending on screen size.
		// ww = C * sw => C = ww/sw => wh = C * sh = ww / sw * sh
		float wh = ww/sw * sh;
		LOGD(boost::format("screen dimensions={%f, %f}, PROJECTION HEIGHT %f") % sw % sh % wh);

		// Let's support depths in [-100, 100] to allow using multiple layer using integers.
		return glm::ortho(viewPort[0] - ww/2.0f, viewPort[0] + ww/2.0f,
				viewPort[1] - wh/2.0f, viewPort[1] + wh/2.0f, -100.0f, 100.0f);
	};
}

Box2dScene::Box2dScene() : 
		Scene(gameProjectionFunction()), box2dWorld(b2Vec2(0, -10.0f)), actor(nullptr), 
			cameraDistanceToActor(-8, 4, -4, 4), cameraSpeed(1.0f) {

	setZRenderOrder(true);
	box2dWorld.SetContactListener(this);

	registerHandler(HandlerBuilder<PointerEvent>::build(
			[=](PointerEvent const& pe) -> bool {
		auto const& coord = pe.getCoordinate();
		glm::vec3 ndc(coord.x, coord.y, 0.0f);
		auto worldCoord = unproject(ndc);

		if (pe.getActionType() == PointerEvent::ActionType::PRESSED) {
			auto body = findOverlappingBody(worldCoord, 0.5f, box2dWorld, [](b2Body const& body) {
				return body.GetType() == b2BodyType::b2_dynamicBody;
			});

			if (body != nullptr) {
				// Let's create a mouse joint
				b2MouseJointDef mouseJointDef;
				mouseJointDef.bodyA = findStaticBody(box2dWorld);
				mouseJointDef.bodyB = body;
				mouseJointDef.target.Set(worldCoord.x, worldCoord.y);
				mouseJointDef.collideConnected = true;
				mouseJointDef.maxForce = 300.0 * body->GetMass();

				mouseJoints[pe.getPointerId()] = static_cast<b2MouseJoint*>(
						box2dWorld.CreateJoint(&mouseJointDef));
				return true;
			}
		} else if (pe.getActionType() == PointerEvent::ActionType::MOVED) {
			if (mouseJoints.find(pe.getPointerId()) != mouseJoints.end()) {
				mouseJoints[pe.getPointerId()]->SetTarget(b2Vec2(worldCoord.x, worldCoord.y));
				return true;
			}
		} else { // Cancelled or released
			if (mouseJoints.find(pe.getPointerId()) != mouseJoints.end()) {
				box2dWorld.DestroyJoint(mouseJoints[pe.getPointerId()]);
				mouseJoints.erase(pe.getPointerId());
				return true;
			}
		}

		return false;
	}));
}

LoadData Box2dScene::loadBox2dData(Box2dParseData const& box2dData, b2Vec2 const& offset, bool modifyWorldProperties) {
	auto loadData = rocket::box2d::loadBox2dData(box2dWorld, box2dData, offset, modifyWorldProperties);

	for (auto& imageData : box2dData.images) {
		LOGD(boost::format("Attaching %s") % imageData.file);

		b2Body *body = nullptr; // parseData.bodies[imageData.bodyId].body;
		if (imageData.bodyId >= 0 && imageData.bodyId < static_cast<int>(loadData.bodiesInLoadOrder.size())) {
			body = loadData.bodiesInLoadOrder[imageData.bodyId];
		} else if (static_cast<int>(loadData.bodiesInLoadOrder.size()) <= imageData.bodyId) {
			throw std::runtime_error("Invalid body id in parseData");
		} 

		// Adding image
		// width and height is determined through bottomleft and topright corners
		float width = imageData.corners[2].x-imageData.corners[0].x;
		float height = imageData.corners[2].y-imageData.corners[0].y;
		LOGD(boost::format("Loading sprite %s size=(%f, %f)") % imageData.name % width % height);
		std::shared_ptr<Sprite> sprite = std::make_shared<Sprite>(
				ImageId(ResourceId(imageData.file)), width, height);

		if (body) {
			auto spriteObject = attachToBox2dBody(body, sprite);
			spriteObject->setPosition( glm::vec3(
					imageData.center.x, imageData.center.y, imageData.renderOrder) );
			spriteObject->setRotation(imageData.angle, Z_AXIS);
		} else {
			auto spriteObject = add(sprite, true);
			spriteObject->setPosition(glm::vec3(imageData.center.x, imageData.center.y, imageData.renderOrder));
			spriteObject->setRotation(imageData.angle, Z_AXIS);
		}
	}

	return loadData;
}

void Box2dScene::updateImpl() {
	// Update physics.
	box2dWorld.Step(1.0f/60.0f, 6, 2);

	// Post world update actions...
	for (auto& task : afterPhysicsTasks) {
		task();
	}
	afterPhysicsTasks.clear();
	

	if (actor != nullptr) {
		updateCameraPosition();
	}

	for (auto& p : attachedObjects) {
		auto attachInfo = p.second;

		// If body is not sleeping we should update bodyobject position.
		if (attachInfo.body->IsAwake()) {
			auto bodyObject = attachInfo.emptyObject;
			auto oldPos = bodyObject->getPosition();

			auto bodyPos = attachInfo.body->GetPosition();
			bodyObject->setPosition(glm::vec3(bodyPos.x, bodyPos.y, 0.0f));
			bodyObject->setRotation(attachInfo.body->GetAngle(), Z_AXIS);
		}
	}
}

void Box2dScene::updateCameraPosition() {
	// Validate actor
	auto body = box2dWorld.GetBodyList();
	while (body != nullptr && body != actor) {
		body = body->GetNext();
	}

	if (body == nullptr) {
		LOGW("Invalid actor ignoring camera update!");
		actor = nullptr;
		return;
	}

	auto actorPos = actor->GetPosition();
	auto cameraPos = getCamera().getPosition();
	auto actorDistx = actorPos.x - cameraPos.x;
	auto actorDisty = actorPos.y - cameraPos.y;

//	LOGD(boost::format("cameraSpeed %f cameraDistanceToActor[0]=%f") % cameraSpeed % cameraDistanceToActor[0]);
//	float dx;
	if (actorDistx < cameraDistanceToActor[0]) {
		auto dx = std::max(actorDistx - cameraDistanceToActor[0], -cameraSpeed); // (dx < 0)
		cameraPos.x += dx; // i.e. cameraPos.x = actorPos.x - cameraDistanceToActor[0].
	} else if (actorDistx > cameraDistanceToActor[1]) {
		if (cameraSpeed < actorDistx - cameraDistanceToActor[1]) {
			LOGE("We need to go faster...");
		}
		auto dx = std::min(actorDistx - cameraDistanceToActor[1], cameraSpeed); // (dx > 0)
		cameraPos.x += dx; // i.e. cameraPos.x = actorPos.x - cameraDistanceToActor[1];
	}

	if (actorDisty < cameraDistanceToActor[2]) {
		auto dy = actorDisty - cameraDistanceToActor[2]; // (dy < 0)
		cameraPos.y += dy; // i.e. cameraPos.y = actorPos.y - cameraDistanceToActor[2].
	} else if (actorDisty > cameraDistanceToActor[3]) {
		auto dy = actorDisty - cameraDistanceToActor[3]; // (dy > 0)
		cameraPos.y += dy; // i.e. cameraPos.y = actorPos.y - cameraDistanceToActor[3];
	}
	getCamera().setPosition(cameraPos);

/*
	float dx = actorPos.x - cameraPos.x;
	if (dx < cameraDistanceToActor[0]) { // dx < -8
		cameraPos.x = actorPos.x - cameraDistanceToActor[0];
	} else if (dx > cameraDistanceToActor[1]) {
		cameraPos.x = actorPos.x - cameraDistanceToActor[1];
	}


	float dy = actorPos.y - cameraPos.y;
	if (dy < cameraDistanceToActor[2]) {
		cameraPos.y = actorPos.y - cameraDistanceToActor[2];
	} else if (dy > cameraDistanceToActor[3]) {
		cameraPos.y = actorPos.y - cameraDistanceToActor[3];
	}
	getCamera()->setPosition(cameraPos);
*/
}

rocket::game2d::RenderObject* Box2dScene::attachToBox2dBody(b2Body* body, std::shared_ptr<rocket::game2d::Renderable> const& renderable) {
	// Let's create a body node, to reflect the movements of the box2d body.
	auto bodyPos = body->GetPosition();
	auto emptyObject = add(nullptr, true);
	emptyObject->setPosition(glm::vec3(bodyPos.x, bodyPos.y, 0));
	emptyObject->setRotation(body->GetAngle(), rocket::game2d::Z_AXIS);

	// Creating the attached object
	auto attachedObject = add(renderable, true, emptyObject);

	// Creating attachement descriptions and add to map.
	BodyAttachDesc attachDesc;
	attachDesc.body = body;
	attachDesc.emptyObject = emptyObject;
	attachDesc.attachedObject = attachedObject;
	attachedObjects[attachedObject] = attachDesc;

	return attachedObject;
}

void Box2dScene::removeAttachedObject(rocket::game2d::RenderObject* ro) {
	if (attachedObjects.find(ro) != attachedObjects.end()) {
		remove(attachedObjects[ro].emptyObject);
		remove(ro);
		attachedObjects.erase(ro);
	} else {
		ROCKET_ASSERT_TRUE(false);
	}
}

}
