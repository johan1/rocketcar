#ifndef _ROCKETCAR_BOX2D_SCENE_H_
#define _ROCKETCAR_BOX2D_SCENE_H_

#include <rocket/game2d/world/Camera.h>
#include <rocket/game2d/world/Renderable.h>
#include <rocket/game2d/world/Scene.h>
#include <rocket/game2d/world/Sprite.h>
#include <rocket/util/Assert.h>

#include <rocket/resource/ResourceId.h>

#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/b2World.h>

#include <Box2D/Dynamics/Joints/b2MouseJoint.h>

#include <functional>
#include <map>
#include <memory>

#include <glm/glm.hpp>

#include "../box2d/Box2dLoader.h"

namespace rocketcar {

std::function<glm::mat4(glm::vec4 const&)> gameProjectionFunction();

//! Description on how to attach an object to a body
struct BodyAttachDesc {
	b2Body const* body;
	rocket::game2d::RenderObject *emptyObject;
	rocket::game2d::RenderObject *attachedObject;

	BodyAttachDesc() : body(nullptr) {}
};

class Box2dScene : public rocket::game2d::Scene, private b2ContactListener {
public:
	Box2dScene(); // TODO: Setup projection and box2d world
	virtual ~Box2dScene() {}

	void setActor(b2Body* body);
	b2Body* getActor();

	void pausePhysics() {
		physicsRunning = false;
	}

	void resumePhysics() {
		physicsRunning = true;
	}

	bool isPhysicsPaused() const {
		return !physicsRunning;
	}

	// Constraint camera distance to actor
	void setCameraDistanceToActor(float left, float right, float bottom, float top);

	//! Set camera speed, units per frame.
	void setCameraSpeed(float cameraSpeed) { this->cameraSpeed = cameraSpeed; }

	// Box2d Contact Listener stuff.
	virtual void BeginContact(b2Contact* contact) {
		if (beginContactCallback) {
			beginContactCallback(contact);
		}
	}

	virtual void EndContact(b2Contact* contact) {
		if (endContactCallback) {
			endContactCallback(contact);
		}
	}

	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
		if (preSolveCallback) {
			preSolveCallback(contact, oldManifold);
		}
	}

	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
		if (postSolveCallback) {
			postSolveCallback(contact, impulse);
		}
	}

	void setBeginContactCallback(std::function<void(b2Contact* contact)> const& callback) {
		beginContactCallback = callback;
	}

	void setEndContactCallback(std::function<void(b2Contact* contact)> const& callback) {
		endContactCallback = callback;
	}

	void setPreSolveCallback(std::function<void(b2Contact* contact, const b2Manifold* oldManifold)> const& callback) {
		preSolveCallback = callback;
	}

	void setPostSolveCallback(std::function<void(b2Contact* contact, const b2ContactImpulse* impulse)> const& callback) {
		postSolveCallback = callback;
	}

	b2World& getBox2dWorld() {
		return box2dWorld;
	}

	void removeAttachedObjects(b2Body const* body);

	rocket::game2d::RenderObject* attachToBox2dBody(b2Body* body, std::shared_ptr<rocket::game2d::Renderable> const& renderable);

	void removeAttachedObject(rocket::game2d::RenderObject* ro);

	// Scene loading
	rocket::box2d::LoadData loadBox2dData(rocket::box2d::Box2dParseData const& box2dData, b2Vec2 const& offset, bool modifyWorldProperties);

	void scheduleAfterPhysicsUpdate(std::function<void(void)> const& task) {
		afterPhysicsTasks.push_back(task);
	}

private:
	b2World box2dWorld;

	bool physicsRunning = true;
	std::map<int, b2MouseJoint*> mouseJoints;
	std::vector<std::function<void(void)>> afterPhysicsTasks;

	// Box2d contact callbacks
	std::function<void(b2Contact*)> beginContactCallback;
	std::function<void(b2Contact*)> endContactCallback;
	std::function<void(b2Contact*, b2Manifold const*)> preSolveCallback;
	std::function<void(b2Contact*, b2ContactImpulse const*)> postSolveCallback;

	b2Body *actor;

	std::unordered_map<rocket::game2d::RenderObject*, BodyAttachDesc> attachedObjects;

	glm::vec4 cameraDistanceToActor;
	float cameraSpeed;

	Box2dScene(Box2dScene const&) = delete;
	Box2dScene& operator=(Box2dScene const&) = delete;

	virtual void updateImpl();
	void updateCameraPosition();

	bool isMouseJointDestroyed(b2MouseJoint* mouseJoint);
};

inline
void Box2dScene::setActor(b2Body* body) {
	this->actor = body;
}

inline
b2Body* Box2dScene::getActor() {
	return actor;
}

inline
void Box2dScene::setCameraDistanceToActor(float left, float right, float bottom, float top) {
	cameraDistanceToActor[0] = left;
	cameraDistanceToActor[1] = right;
	cameraDistanceToActor[2] = bottom;
	cameraDistanceToActor[3] = top;
}

}

#endif
