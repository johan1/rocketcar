#ifndef _ROCKET_RUBE_PARSER_H_
#define _ROCKET_RUBE_PARSER_H_

/**
 * Implements loading of box2d according to https://www.iforce2d.net/rube/json-structure
 */
// #include <stdexcept>
#include <Box2D/Box2D.h>
#include <rocket/util/Log.h>
#include <cppjson/json.h>

// #include <boost/any.hpp>
// #include <unordered_map>

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
		LOGD(boost::format("RubeParserException (%d): %s\n") % static_cast<int>(type) % what);
	}
	Type getType() const { return type; }

private:
	Type const type;
};

class RubeParser {
public:
	RubeParser() {} /* : offset(b2Vec2(0, 0)), modifyWorldProperties(false) {} */

/*
	void setOffset(b2Vec2 const& offset) {
		this->offset = offset;
	}

	void setModifyWorldProperties(bool modifyWorldProperties) {
		this->modifyWorldProperties = modifyWorldProperties;
	}
*/
	Box2dParseData loadRubeDocument(json::value const& documentRoot);

private:
	using ValueTypeChecker = bool (*)(json::value const&);

//	b2Vec2 offset;
//	bool modifyWorldProperties;

	json::value getDefaultValue(ValueTypeChecker checker);

	std::string valueTypeToString(json::value const& value);
	json::value lookupValue(json::value const& value, std::string const& key, ValueTypeChecker checker);

	double getNumber(json::value const& value, std::string const& key);

	int getInt(json::value const& value, std::string const& key);

	bool getBool(json::value const& value, std::string const& key);

	std::string getString(json::value const& value, std::string const& key);

	b2Vec2 getb2Vec2(json::value const& root, std::string const& key);

	std::vector<b2Vec2> getb2Vec2s(json::value const& root, std::string const& key);

	void loadCircleShape(b2CircleShape &circleShape, json::value const& circleRoot);

	void loadPolygonShape(b2PolygonShape &polygonShape, json::value const& polygonRoot);

	void loadChainShape(b2ChainShape &chainShape, json::value const& chainRoot);

	FixtureParseData loadBox2dFixture(json::value const& fixtureRoot);

	BodyParseData loadBox2dBody(json::value const& bodyRoot);

	template <typename JointDef>
	std::unique_ptr<JointDef> createBox2dJointDef(json::value const& jointRoot) {
		std::unique_ptr<JointDef> jointDef(new JointDef);

// TODO: We can NOT set any bodies until their created... We need to resolve this.
//		jointDef->bodyA = parseData.bodies[getInt(jointRoot, "bodyA")].body;
//		jointDef->bodyB = parseData.bodies[getInt(jointRoot, "bodyB")].body;
		jointDef->localAnchorA = getb2Vec2(jointRoot, "anchorA");
		jointDef->localAnchorB = getb2Vec2(jointRoot, "anchorB");
		jointDef->collideConnected = getBool(jointRoot, "collideConnected");

		return jointDef;
	}

	JointParseData loadBox2dJoint(json::value const& jointRoot, Box2dParseData &box2dData);

	void loadCustomProperty(std::unordered_map<std::string, boost::any> &map, json::value const& customProperty);
	ImageParseData loadBox2dImage(json::value const& imageRoot);
};

// TODO This should be a member...
// BodyParseData* lookupBody(std::string const &name, WorldParseData& parseData);

}}

#endif
