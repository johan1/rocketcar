#ifndef _ROCKET_CONTACT_REPEATER_H_
#define _ROCKET_CONTACT_REPEATER_H_

namespace rocket { namespace box2d {

struct BeginContactEvent {
	b2Contact contact;
};

struct EndContactEvent {
	b2Contact contact;
};

struct PreSolveEvent {
	b2Contact contact;
	b2Manifold oldManifold;
};

struct PostSolveEvent {
	b2Contact contact;
	b2ContactImpulse impulse;
};

class ContactRepeater : public rocket::util::Repeater<ContactRepeater,
		BeginContactEvent, EndContactEvent, PreSolveEvent, PostSolveEvent>, public b2ContactListener {
public:
	virtual void BeginContact(b2Contact* contact) {
		post(BeginContactEvent {*contact} );
	}

	virtual void EndContact(b2Contact* contact) {
		post(EndContactEvent {*contact} );
	}

	virtual void PreSolve(b2Contact* contact, b2Manifold const* oldManifold) {
		post(PreSolveEvent(*contact, *oldManifold);
	}

	virtual void PostSolve(b2Contact* contact, b2ContactImpulse const* impulse) {
		post(PostSolveEvent(*contact, *impulse);
	}

	// Bring Repeater::repeat into scope
	using rocket::input::InputRepeater<ContactRepeater>::type::repeat;

	template <typename Event>
	void repeat(Event const& event) {
		
	}
};

}}

#endif
