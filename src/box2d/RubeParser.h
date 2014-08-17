#ifndef _ROCKET_RUBE_PARSER_H_
#define _ROCKET_RUBE_PARSER_H_

/**
 * Implements loading of box2d according to https://www.iforce2d.net/rube/json-structure
 */
#include <Box2D/Box2D.h>
#include <rocket/Log.h>
#include <json/json.h>

#include "CommonStructs.h"

namespace rocket { namespace box2d {

class RubeParserException : public std::runtime_error {
public:
	enum class Type {
		UnexpectedValue, // If incorrect value
		UnsupportedType, // If incorrect type
		UnavailableName,  // If name is unavailable
		UnknowJointType
	};

	RubeParserException(Type type, std::string const& what) : std::runtime_error(what), type(type) {
		LOGD("RubeParserException (" << static_cast<int>(type) << "): " << what);
	}
	Type getType() const { return type; }

private:
	Type const type;
};

class RubeParser {
public:
	RubeParser() {}

	Box2dParseData loadRubeDocument(Json::Value const& documentRoot);

private:
	std::string valueTypeToString(Json::Value const& value);

	Json::Value lookupValue(Json::Value const& value, std::string const& key, Json::ValueType const& valueType);

	double getNumber(Json::Value const& value, std::string const& key);

	int getInt(Json::Value const& value, std::string const& key);

	bool getBool(Json::Value const& value, std::string const& key);

	std::string getString(Json::Value const& value, std::string const& key);

	b2Vec2 getb2Vec2(Json::Value const& root, std::string const& key);

	std::vector<b2Vec2> getb2Vec2s(Json::Value const& root, std::string const& key);

	void loadCircleShape(b2CircleShape &circleShape, Json::Value const& circleRoot);

	void loadPolygonShape(b2PolygonShape &polygonShape, Json::Value const& polygonRoot);

	void loadChainShape(b2ChainShape &chainShape, Json::Value const& chainRoot);

	FixtureParseData loadBox2dFixture(Json::Value const& fixtureRoot);

	BodyParseData loadBox2dBody(Json::Value const& bodyRoot);

	template <typename JointDef>
	std::unique_ptr<JointDef> createBox2dJointDef(Json::Value const& jointRoot) {
		std::unique_ptr<JointDef> jointDef(new JointDef);

// TODO: We can NOT set any bodies until their created... We need to resolve this.
//		jointDef->bodyA = parseData.bodies[getInt(jointRoot, "bodyA")].body;
//		jointDef->bodyB = parseData.bodies[getInt(jointRoot, "bodyB")].body;
		jointDef->localAnchorA = getb2Vec2(jointRoot, "anchorA");
		jointDef->localAnchorB = getb2Vec2(jointRoot, "anchorB");
		jointDef->collideConnected = getBool(jointRoot, "collideConnected");

		return jointDef;
	}

	JointParseData loadBox2dJoint(Json::Value const& jointRoot, Box2dParseData &box2dData);

	void loadCustomProperty(std::unordered_map<std::string, boost::any> &map, Json::Value const& customProperty);
	ImageParseData loadBox2dImage(Json::Value const& imageRoot);
};

}
// Collapse
using namespace rocket::box2d;

}

#endif
