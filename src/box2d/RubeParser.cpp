#include "RubeParser.h"

#include <cppjson/json.h>
#include <Box2D/Box2D.h>

// #include <type_traits>
#include <algorithm>
#include <cmath>

static const bool DO_LOG = false;

namespace rocket { namespace box2d {

using ValueTypeChecker = bool (*)(json::value const&);

json::value RubeParser::getDefaultValue(ValueTypeChecker checker) {
	if(DO_LOG) LOGD("getDefaultValue!");
	if (checker == json::is_string) {
		return json::value("");
	} else if (checker == json::is_number) {
		return json::value(0);
	} else if (checker == json::is_bool) {
		return json::value(false);
	} else if (checker == json::is_object) {
		return json::value(json::object());
	} else if (checker == json::is_array) {
		return json::value(json::array());
	} else if (checker == json::is_null) {
		return json::value(); // NULL
	} else {
	
		throw RubeParserException(RubeParserException::Type::UnsupportedType, "Invalid type checker");
	}
}

std::string RubeParser::valueTypeToString(json::value const& value) {
	if (json::is_bool(value)) return "bool";
	else if (json::is_string(value)) return "string";
	else if (json::is_number(value)) return "number";
	else if (json::is_object(value)) return "object";
	else if (json::is_array(value)) return "array";
	else if (json::is_null(value)) return "null";
	else throw std::runtime_error("WTF!! RubeParser::valueTypeToString didn't find value type...");
}

json::value RubeParser::lookupValue(json::value const& value, std::string const& key, ValueTypeChecker checker) {
	if(DO_LOG) LOGD(boost::format("lookupValue: %s") % key); 
	if (!json::has_key(value, key)) {
		return getDefaultValue(checker);
	}

	auto ret = value[key];
	// Check if correct type.
	if (!checker(ret)) {
		std::string msg = (boost::format("Unexpected type (%s) for key (%s)")
				% valueTypeToString(ret) % key).str();
		throw RubeParserException(RubeParserException::Type::UnexpectedValue, 
				msg);
	}

	return ret;
}

double RubeParser::getNumber(json::value const& value, std::string const& key) {
	auto ret = lookupValue(value, key, json::is_number);
	return json::to_number(ret);
}

int RubeParser::getInt(json::value const& value, std::string const& key) {
	return lround(getNumber(value, key));
}

bool RubeParser::getBool(json::value const& value, std::string const& key) {
	auto ret = lookupValue(value, key, json::is_bool);
	return json::to_bool(ret);
}

std::string RubeParser::getString(json::value const& value, std::string const& key) {
	auto ret = lookupValue(value, key, json::is_string);
	return json::to_string(ret);
}

b2Vec2 RubeParser::getb2Vec2(json::value const& root, std::string const& key) {
	if (json::has_key(root, key) && json::is_number(root[key])) {
		double coord = json::to_number(root[key]);
		return b2Vec2(coord, coord);
	} else {
		auto vecObject = lookupValue(root, key, json::is_object);
		float x = getNumber(vecObject, "x");
		float y = getNumber(vecObject, "y");
		return b2Vec2(x, y);
	}
}

std::vector<b2Vec2> RubeParser::getb2Vec2s(json::value const& root, std::string const& key) {
	std::vector<b2Vec2> b2Vec2s;

	auto vectors = lookupValue(root, key, json::is_object);

	auto xValues = lookupValue(vectors, "x", json::is_array);
	for (size_t i = 0; i < json::size(xValues); ++i) {
		b2Vec2s.push_back(b2Vec2(json::to_number(xValues[i]), 0.0f));
	}
	auto yValues = lookupValue(vectors, "y", json::is_array);
	for (size_t i = 0; i < json::size(yValues); ++i) {
		b2Vec2s[i].y = json::to_number(yValues[i]);
	}

	return b2Vec2s;
}

void RubeParser::loadCircleShape(b2CircleShape &circleShape, json::value const& circleRoot) {
	if(DO_LOG) LOGD("Loading circle shape");

	circleShape.m_p = getb2Vec2(circleRoot, "center");
	circleShape.m_radius = getNumber(circleRoot, "radius");
}

void RubeParser::loadPolygonShape(b2PolygonShape &polygonShape, json::value const& polygonRoot) {
	if (DO_LOG) if(DO_LOG) LOGD("Loading polygon shape");

	auto vertices = getb2Vec2s(polygonRoot, "vertices");
	polygonShape.Set(&(vertices[0]), vertices.size());
}

void RubeParser::loadChainShape(b2ChainShape &chainShape, json::value const& chainRoot) {
	if (DO_LOG) LOGD("Loading chain shape");

	auto vertices = getb2Vec2s(chainRoot, "vertices");
	
	if (json::has_key(chainRoot, "hasNextVertex") || json::has_key(chainRoot, "hasPrevVertex") ||
			json::has_key(chainRoot, "nextVertex") || json::has_key(chainRoot, "prevVertex")) {
		chainShape.CreateLoop(&(vertices[0]), vertices.size());

		// TODO: I'm not convinced that this is intentional, however protocol allows specification of these
		// parameters, this let's apply and see what happens...
		chainShape.m_hasNextVertex = getBool(chainRoot, "hasNextVertex");
		chainShape.m_hasPrevVertex = getBool(chainRoot, "hasPrevVertex");
		chainShape.m_nextVertex = getb2Vec2(chainRoot, "nextVertex");
		chainShape.m_prevVertex = getb2Vec2(chainRoot, "prevVertex");
	} else {
		chainShape.CreateChain(&(vertices[0]), vertices.size());
	}
}

FixtureParseData RubeParser::loadBox2dFixture(json::value const& fixtureRoot) {
	if (DO_LOG) LOGD("Loading fixture");

	FixtureParseData fixtureData;
	fixtureData.name = getString(fixtureRoot, "name");

	fixtureData.fixtureDef.friction = getNumber(fixtureRoot, "friction");
	fixtureData.fixtureDef.restitution = getNumber(fixtureRoot, "restitution");
	fixtureData.fixtureDef.density = getNumber(fixtureRoot, "density");
	fixtureData.fixtureDef.isSensor = getBool(fixtureRoot, "sensor");

	if (json::has_key(fixtureRoot, "filter-categoryBits")) {
		fixtureData.fixtureDef.filter.categoryBits = getInt(fixtureRoot, "filter-categoryBits");
	} else {
		fixtureData.fixtureDef.filter.categoryBits = 1;
	}
	if (json::has_key(fixtureRoot, "filter-maskBits")) 	{
		fixtureData.fixtureDef.filter.categoryBits = getInt(fixtureRoot, "filter-maskBits");
	} else {
		fixtureData.fixtureDef.filter.categoryBits = 0xffff;
	}
	fixtureData.fixtureDef.filter.groupIndex = getInt(fixtureRoot, "filter-groupIndex");

	if (json::has_key(fixtureRoot, "circle")) {
		b2CircleShape *circleShape = new b2CircleShape;
		fixtureData.shape = std::unique_ptr<b2Shape>(circleShape);
		loadCircleShape(*circleShape, fixtureRoot["circle"]);
	} else if (json::has_key(fixtureRoot, "polygon")) {
		b2PolygonShape *polygonShape = new b2PolygonShape;
		fixtureData.shape = std::unique_ptr<b2Shape>(polygonShape);
		loadPolygonShape(*polygonShape, fixtureRoot["polygon"]);
	} else if (json::has_key(fixtureRoot, "chain")) {
		b2ChainShape *chainShape = new b2ChainShape;
		fixtureData.shape = std::unique_ptr<b2Shape>(chainShape);
		loadChainShape(*chainShape, fixtureRoot["chain"]);
	} else {
		throw std::runtime_error("Unsupported fixture type");
	}
	fixtureData.fixtureDef.shape = fixtureData.shape.get();

	// TODO Map fixture name name to bodyData
	// TODO Implement support for custom properties.

	return fixtureData;
}

BodyParseData RubeParser::loadBox2dBody(json::value const& bodyRoot) {
	if(DO_LOG) LOGD("Loading body");

	BodyParseData bodyData;

	bodyData.bodyDef.type = static_cast<b2BodyType>(getInt(bodyRoot, "type"));
	bodyData.bodyDef.position = getb2Vec2(bodyRoot, "position");
	bodyData.bodyDef.angle = getNumber(bodyRoot, "angle");
	bodyData.bodyDef.linearVelocity = getb2Vec2(bodyRoot, "linearVelocity");
	bodyData.bodyDef.angularVelocity = getNumber(bodyRoot, "angularVelocity");
	bodyData.bodyDef.linearDamping = getNumber(bodyRoot, "linearDamping");
	bodyData.bodyDef.angularDamping = getNumber(bodyRoot, "angularDamping");
	bodyData.bodyDef.awake = getBool(bodyRoot, "awake");
	bodyData.bodyDef.bullet = getBool(bodyRoot, "bullet");
	bodyData.bodyDef.fixedRotation = getBool(bodyRoot, "fixedRotation");	

	bodyData.name = getString(bodyRoot, "name");
	bodyData.massData.mass = getNumber(bodyRoot, "massData-mass");
	bodyData.massData.center = getb2Vec2(bodyRoot, "massData-center");
	bodyData.massData.I = getNumber(bodyRoot, "massData-I");

	auto fixtures = lookupValue(bodyRoot, "fixture", json::is_array);
	for (size_t i = 0; i < json::size(fixtures); ++i) {
		bodyData.fixtureDatas.push_back(loadBox2dFixture(fixtures[i]));
	}

	return bodyData;
}

JointParseData RubeParser::loadBox2dJoint(json::value const& jointRoot, Box2dParseData &box2dData)  {
	if(DO_LOG) LOGD("Loading joint");

	JointParseData jointData;
	jointData.name = getString(jointRoot, "name");
	jointData.type = getString(jointRoot, "type"); // Do we want to expose type as string? Why not typeid?

	jointData.bodyA = box2dData.bodies[getInt(jointRoot, "bodyA")].name;
	jointData.bodyB = box2dData.bodies[getInt(jointRoot, "bodyB")].name;
	
	if (jointData.type == "revolute") {
		auto revJointDef = createBox2dJointDef<b2RevoluteJointDef>(jointRoot);
		revJointDef->enableLimit = getBool(jointRoot, "enableLimit");
		revJointDef->enableMotor = getBool(jointRoot, "enableMotor");
		// revJointDef->jointSpeed = getNumber(jointRoot, "jointSpeed"); // TODO: What should joint speed do?
		revJointDef->lowerAngle = getNumber(jointRoot, "lowerLimit");
		revJointDef->maxMotorTorque = getNumber(jointRoot, "maxMotorTorque");
		revJointDef->motorSpeed = getNumber(jointRoot, "motorSpeed");
		revJointDef->referenceAngle = getNumber(jointRoot, "refAngle");
		revJointDef->upperAngle = getNumber(jointRoot, "upperLimit");
		jointData.jointDef = std::move(revJointDef);

	} else if (jointData.type == "distance") {
		auto distanceJointDef = createBox2dJointDef<b2DistanceJointDef>(jointRoot);
		distanceJointDef->dampingRatio = getNumber(jointRoot, "dampingRatio");
		distanceJointDef->frequencyHz = getNumber(jointRoot, "frequency");
		distanceJointDef->length = getNumber(jointRoot, "length");
		jointData.jointDef = std::move(distanceJointDef);

	} else if (jointData.type == "prismatic") {
		auto prismaticJointDef = createBox2dJointDef<b2PrismaticJointDef>(jointRoot);
		prismaticJointDef->enableLimit = getBool(jointRoot, "enableLimit");
		prismaticJointDef->enableMotor = getBool(jointRoot, "enableMotor");
		prismaticJointDef->localAxisA = getb2Vec2(jointRoot, "localAxisA");
		prismaticJointDef->lowerTranslation = getNumber(jointRoot, "lowerLimit");
		prismaticJointDef->maxMotorForce = getNumber(jointRoot, "maxMotorForce");
		prismaticJointDef->motorSpeed = getNumber(jointRoot, "motorSpeed");
		prismaticJointDef->referenceAngle = getNumber(jointRoot, "referenceAngle");
		prismaticJointDef->upperTranslation = getNumber(jointRoot, "upperLimit");
		jointData.jointDef = std::move(prismaticJointDef); // Do we need move here or is it implicit?

	} else if (jointData.type == "wheel") {
		auto wheelJointDef = createBox2dJointDef<b2WheelJointDef>(jointRoot);
		wheelJointDef->enableMotor = getBool(jointRoot, "enableMotor");
		wheelJointDef->localAxisA = getb2Vec2(jointRoot, "localAxisA");
		wheelJointDef->maxMotorTorque = getNumber(jointRoot, "maxMotorTorque");
		wheelJointDef->motorSpeed = getNumber(jointRoot, "motorSpeed");
		wheelJointDef->dampingRatio = getNumber(jointRoot, "springDampingRatio");
		wheelJointDef->frequencyHz = getNumber(jointRoot, "springFrequency");
		jointData.jointDef = std::move(wheelJointDef); // Do we need move here or is it implicit?

	} else if (jointData.type == "rope") {
		auto ropeJointDef = createBox2dJointDef<b2RopeJointDef>(jointRoot);
		ropeJointDef->maxLength = getNumber(jointRoot, "maxLength");
		jointData.jointDef = std::move(ropeJointDef);

	} else if (jointData.type == "motor") {
		// Motor joints differs a bit...
		std::unique_ptr<b2MotorJointDef> motorJointDef(new b2MotorJointDef);

		// TODO: We can NOT set any bodies until they are created. We need to resolve this!!!
//		motorJointDef->bodyA = parseData.bodies[getInt(jointRoot, "bodyA")].body;
//		motorJointDef->bodyB = parseData.bodies[getInt(jointRoot, "bodyB")].body;

		motorJointDef->collideConnected = getBool(jointRoot, "collideConnected");
		motorJointDef->linearOffset = getb2Vec2(jointRoot, "linearOffset");
		motorJointDef->maxForce = getNumber(jointRoot, "maxForce");
		motorJointDef->maxTorque = getNumber(jointRoot, "maxTorque");
		motorJointDef->correctionFactor = getNumber(jointRoot, "correctionFactor");
		jointData.jointDef = std::move(motorJointDef);

	} else if (jointData.type == "weld") {
		auto weldJointDef = createBox2dJointDef<b2WeldJointDef>(jointRoot);
		weldJointDef->referenceAngle = getNumber(jointRoot, "refAngle");
		weldJointDef->dampingRatio = getNumber(jointRoot, "dampingRatio");
		weldJointDef->frequencyHz = getNumber(jointRoot, "frequency");
		jointData.jointDef = std::move(weldJointDef);

	} else if (jointData.type == "friction") {
		auto frictionJointDef = createBox2dJointDef<b2FrictionJointDef>(jointRoot);
		frictionJointDef->maxForce = getNumber(jointRoot, "maxForce");
		frictionJointDef->maxTorque = getNumber(jointRoot, "maxTorque");
		jointData.jointDef = std::move(frictionJointDef);

	} else {
		throw RubeParserException(RubeParserException::Type::UnknowJointType,
				"Unrecognised joint type");
	}

	return jointData;
}

void RubeParser::loadCustomProperty(std::unordered_map<std::string, boost::any> &map, json::value const& customProperty) {
	std::string name = getString(customProperty, "name");
	
	if (json::has_key(customProperty, "int")) {
		map[name] = getInt(customProperty, "int");
	} else if (json::has_key(customProperty, "float")) {
		map[name] = getNumber(customProperty, "float");
	} else if (json::has_key(customProperty, "string")) {
		map[name] = getString(customProperty, "string");
	} else if (json::has_key(customProperty, "vec2")) {
		map[name] = getb2Vec2(customProperty, "vec2");
	} else if (json::has_key(customProperty, "bool")) {
		map[name] = getBool(customProperty, "bool");
	} else {
		throw RubeParserException(RubeParserException::Type::UnsupportedType,
				"Unnexptected type");
	}
}

ImageParseData RubeParser::loadBox2dImage(json::value const& imageRoot) {
	ImageParseData imageData;
	imageData.name = getString(imageRoot, "name");
	imageData.opacity = getNumber(imageRoot, "opacity");
	imageData.renderOrder = getInt(imageRoot, "renderOrder");
	imageData.scale = getNumber(imageRoot, "scale");
	imageData.bodyId = getInt(imageRoot, "body");
	imageData.angle = getNumber(imageRoot, "angle");
	imageData.center = getb2Vec2(imageRoot, "center");
	imageData.corners = getb2Vec2s(imageRoot, "corners");
	imageData.file = getString(imageRoot, "file");
	imageData.filter = getInt(imageRoot, "filter");
	imageData.flip = getBool(imageRoot, "flip");

	if (json::has_key(imageRoot, "colorTint")) {
		auto tintArr = lookupValue(imageRoot, "colorTint", json::is_array);
		for (size_t i = 0; i < json::size(tintArr) && i < 4; ++i) {
			imageData.colorTint[i] = static_cast<unsigned char>(json::to_number(tintArr[i]));
		}
	} else {
		imageData.colorTint[0] = 255;
		imageData.colorTint[1] = 255;
		imageData.colorTint[2] = 255;
		imageData.colorTint[3] = 255;
	}

	auto customProperties = lookupValue(imageRoot, "customProperties", json::is_array);
	for (size_t i = 0; i < json::size(customProperties); ++i) {
		loadCustomProperty(imageData.customProperties, customProperties[i]);
	}

	return imageData;
}

Box2dParseData RubeParser::loadRubeDocument(json::value const& documentRoot) {
	Box2dParseData parseData;

    parseData.positionIterations = getInt(documentRoot, "positionIterations");
    parseData.velocityIterations = getInt(documentRoot, "velocityIterations"); 
    parseData.stepsPerSecond = getInt(documentRoot, "stepsPerSecond"); 

	parseData.worldData.gravity = getb2Vec2(documentRoot, "gravity");
	parseData.worldData.allowSleep = getBool(documentRoot, "allowSleep");
	parseData.worldData.autoClearForces = getBool(documentRoot, "autoClearForces");
	parseData.worldData.warmStarting = getBool(documentRoot, "warmStarting");
	parseData.worldData.continuousPhysics = getBool(documentRoot, "continuousPhysics");
	parseData.worldData.subStepping = getBool(documentRoot, "subStepping");
/*	
	if (modifyWorldProperties) {
		world.SetGravity( getb2Vec2(documentRoot, "gravity") );
		world.SetAllowSleeping( getBool(documentRoot, "allowSleep"));
		world.SetAutoClearForces( getBool(documentRoot, "autoClearForces"));
		world.SetWarmStarting( getBool(documentRoot, "warmStarting"));
		world.SetContinuousPhysics( getBool(documentRoot, "continuousPhysics"));
		world.SetSubStepping( getBool(documentRoot, "subStepping"));
	}
*/

	auto bodies = lookupValue(documentRoot, "body", json::is_array);
	for (size_t i = 0; i < json::size(bodies); ++i) {
		parseData.bodies.push_back(loadBox2dBody(bodies[i]));
//		loadBox2dBody(world, bodies[i], parseData);
	}

	auto joints = lookupValue(documentRoot, "joint", json::is_array);
	for (size_t i = 0; i < json::size(joints); ++i) {
		parseData.joints.push_back(loadBox2dJoint(joints[i], parseData));
	}

	auto images = lookupValue(documentRoot, "image", json::is_array);
	for (size_t i = 0; i < json::size(images); ++i) {
		parseData.images.push_back(loadBox2dImage(images[i]));
	}

	auto customProperties = lookupValue(documentRoot, "customProperties", json::is_array);
	for (size_t i = 0; i < json::size(customProperties); ++i) {
		loadCustomProperty(parseData.customProperties, customProperties[i]);
	}

/*
	for (auto& bodyData : parseData.bodies) {
		bodyData.body->SetTransform(bodyData.body->GetPosition() + offset, 0);
	}
*/
	return parseData;
}

/*
// Utility functions...
BodyParseData* lookupBody(std::string const &name, Box2dParseData &box2dData) {
	for (auto &bodyData : box2dData.bodies) {
		if (bodyData.name == name) {
			return &bodyData;
		}
	}
	
	return nullptr;
}
*/

}}
