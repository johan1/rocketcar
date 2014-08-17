#include "RubeParser.h"

#include <Box2D/Box2D.h>

#include <algorithm>
#include <cmath>

static const bool DO_LOG = false;

namespace rocket { namespace box2d {

std::string RubeParser::valueTypeToString(Json::Value const& value) {
	if (value.isBool()) return "bool";
	else if (value.isString()) return "string";
	else if (value.isNumeric()) return "number";
	else if (value.isObject()) return "object";
	else if (value.isArray()) return "array";
	else if (value.isNull()) return "null";
	else throw std::runtime_error("WTF!! RubeParser::valueTypeToString didn't find value type...");
}

Json::Value RubeParser::lookupValue(Json::Value const& value, std::string const& key, Json::ValueType const& valueType) {
	if(DO_LOG) LOGD("lookupValue: " << key);

	if (!value.isMember(key)) {
		return Json::Value(valueType);
	}

	auto ret = value[key];
	// Check if correct type.
	if (!ret.isConvertibleTo(valueType)) {
		std::string msg = (boost::format("Unexpected type (%s) for key (%s)")
				% valueTypeToString(ret) % key).str();
		throw RubeParserException(RubeParserException::Type::UnexpectedValue, 
				msg);
	}

	return ret;
}

double RubeParser::getNumber(Json::Value const& value, std::string const& key) {
	auto ret = lookupValue(value, key, Json::ValueType::realValue);
	return ret.asDouble();
}

int RubeParser::getInt(Json::Value const& value, std::string const& key) {
	return lround(getNumber(value, key));
}

bool RubeParser::getBool(Json::Value const& value, std::string const& key) {
	auto ret = lookupValue(value, key, Json::ValueType::booleanValue);
	return ret.asBool();
}

std::string RubeParser::getString(Json::Value const& value, std::string const& key) {
	auto ret = lookupValue(value, key, Json::ValueType::stringValue);
	return ret.asString();
}

b2Vec2 RubeParser::getb2Vec2(Json::Value const& root, std::string const& key) {
	if (root.isMember(key) && root[key].isNumeric()) {
		double coord = root[key].asDouble();
		return b2Vec2(coord, coord);
	} else {
		auto vecObject = lookupValue(root, key, Json::ValueType::objectValue);
		float x = getNumber(vecObject, "x");
		float y = getNumber(vecObject, "y");
		return b2Vec2(x, y);
	}
}

std::vector<b2Vec2> RubeParser::getb2Vec2s(Json::Value const& root, std::string const& key) {
	std::vector<b2Vec2> b2Vec2s;

	auto vectors = lookupValue(root, key, Json::ValueType::objectValue);

	auto xValues = lookupValue(vectors, "x", Json::ValueType::arrayValue);
	for (size_t i = 0; i < xValues.size(); ++i) {
		b2Vec2s.push_back(b2Vec2(xValues[i].asDouble(), 0.0f));
	}
	auto yValues = lookupValue(vectors, "y", Json::ValueType::arrayValue);
	for (size_t i = 0; i < yValues.size(); ++i) {
		b2Vec2s[i].y = yValues[i].asDouble();
	}

	return b2Vec2s;
}

void RubeParser::loadCircleShape(b2CircleShape &circleShape, Json::Value const& circleRoot) {
	if(DO_LOG) LOGD("Loading circle shape");

	circleShape.m_p = getb2Vec2(circleRoot, "center");
	circleShape.m_radius = getNumber(circleRoot, "radius");
}

void RubeParser::loadPolygonShape(b2PolygonShape &polygonShape, Json::Value const& polygonRoot) {
	if (DO_LOG) if(DO_LOG) LOGD("Loading polygon shape");

	auto vertices = getb2Vec2s(polygonRoot, "vertices");
	polygonShape.Set(&(vertices[0]), vertices.size());
}

void RubeParser::loadChainShape(b2ChainShape &chainShape, Json::Value const& chainRoot) {
	if (DO_LOG) LOGD("Loading chain shape");

	auto vertices = getb2Vec2s(chainRoot, "vertices");
	
	if (chainRoot.isMember("hasNextVertex") || chainRoot.isMember("hasPrevVertex") ||
			chainRoot.isMember("nextVertex") || chainRoot.isMember("prevVertex")) {
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

FixtureParseData RubeParser::loadBox2dFixture(Json::Value const& fixtureRoot) {
	if (DO_LOG) LOGD("Loading fixture");

	FixtureParseData fixtureData;
	fixtureData.name = getString(fixtureRoot, "name");

	fixtureData.fixtureDef.friction = getNumber(fixtureRoot, "friction");
	fixtureData.fixtureDef.restitution = getNumber(fixtureRoot, "restitution");
	fixtureData.fixtureDef.density = getNumber(fixtureRoot, "density");
	fixtureData.fixtureDef.isSensor = getBool(fixtureRoot, "sensor");


	if (fixtureRoot.isMember("filter-categoryBits")) {
		fixtureData.fixtureDef.filter.categoryBits = getInt(fixtureRoot, "filter-categoryBits");
	} else {
		fixtureData.fixtureDef.filter.categoryBits = 1;
	}
	if (fixtureRoot.isMember("filter-maskBits")) 	{
		fixtureData.fixtureDef.filter.categoryBits = getInt(fixtureRoot, "filter-maskBits");
	} else {
		fixtureData.fixtureDef.filter.categoryBits = 0xffff;
	}
	fixtureData.fixtureDef.filter.groupIndex = getInt(fixtureRoot, "filter-groupIndex");

	if (fixtureRoot.isMember("circle")) {
		b2CircleShape *circleShape = new b2CircleShape;
		fixtureData.shape = std::unique_ptr<b2Shape>(circleShape);
		loadCircleShape(*circleShape, fixtureRoot["circle"]);
	} else if (fixtureRoot.isMember("polygon")) {
		b2PolygonShape *polygonShape = new b2PolygonShape;
		fixtureData.shape = std::unique_ptr<b2Shape>(polygonShape);
		loadPolygonShape(*polygonShape, fixtureRoot["polygon"]);
	} else if (fixtureRoot.isMember("chain")) {
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

BodyParseData RubeParser::loadBox2dBody(Json::Value const& bodyRoot) {
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

	auto fixtures = lookupValue(bodyRoot, "fixture", Json::ValueType::arrayValue);
	for (size_t i = 0; i < fixtures.size(); ++i) {
		bodyData.fixtureDatas.push_back(loadBox2dFixture(fixtures[i]));
	}

	return bodyData;
}

JointParseData RubeParser::loadBox2dJoint(Json::Value const& jointRoot, Box2dParseData &box2dData)  {
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

void RubeParser::loadCustomProperty(std::unordered_map<std::string, boost::any> &map, Json::Value const& customProperty) {
	std::string name = getString(customProperty, "name");
	
	if (customProperty.isMember("int")) {
		map[name] = getInt(customProperty, "int");
	} else if (customProperty.isMember("float")) {
		map[name] = getNumber(customProperty, "float");
	} else if (customProperty.isMember("string")) {
		map[name] = getString(customProperty, "string");
	} else if (customProperty.isMember("vec2")) {
		map[name] = getb2Vec2(customProperty, "vec2");
	} else if (customProperty.isMember("bool")) {
		map[name] = getBool(customProperty, "bool");
	} else {
		throw RubeParserException(RubeParserException::Type::UnsupportedType,
				"Unnexptected type");
	}
}

ImageParseData RubeParser::loadBox2dImage(Json::Value const& imageRoot) {
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

	if (imageRoot.isMember("colorTint")) {
		auto tintArr = lookupValue(imageRoot, "colorTint", Json::ValueType::arrayValue);
		for (size_t i = 0; i < tintArr.size() && i < 4; ++i) {
			imageData.colorTint[i] = static_cast<unsigned char>(tintArr[i].asInt());
		}
	} else {
		imageData.colorTint[0] = 255;
		imageData.colorTint[1] = 255;
		imageData.colorTint[2] = 255;
		imageData.colorTint[3] = 255;
	}

	auto customProperties = lookupValue(imageRoot, "customProperties", Json::ValueType::arrayValue);
	for (size_t i = 0; i < customProperties.size(); ++i) {
		loadCustomProperty(imageData.customProperties, customProperties[i]);
	}

	return imageData;
}

Box2dParseData RubeParser::loadRubeDocument(Json::Value const& documentRoot) {
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

	auto bodies = lookupValue(documentRoot, "body", Json::ValueType::arrayValue);
	for (size_t i = 0; i < bodies.size(); ++i) {
		parseData.bodies.push_back(loadBox2dBody(bodies[i]));
	}

	auto joints = lookupValue(documentRoot, "joint", Json::ValueType::arrayValue);
	for (size_t i = 0; i < joints.size(); ++i) {
		parseData.joints.push_back(loadBox2dJoint(joints[i], parseData));
	}

	auto images = lookupValue(documentRoot, "image", Json::ValueType::arrayValue);
	for (size_t i = 0; i < images.size(); ++i) {
		parseData.images.push_back(loadBox2dImage(images[i]));
	}

	auto customProperties = lookupValue(documentRoot, "customProperties", Json::ValueType::arrayValue);
	for (size_t i = 0; i < customProperties.size(); ++i) {
		loadCustomProperty(parseData.customProperties, customProperties[i]);
	}

	return parseData;
}

}}
