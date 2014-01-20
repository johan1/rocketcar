#ifndef _ROCKET_ENERGY_BAR_H_
#define _ROCKET_ENERGY_BAR_H_

#include <rocket/graphics/Canvas.h>

#include <rocket/game2d/world/Renderable.h>

namespace rocketcar {

struct EnergyBarStyle {
	rocket::glutils::RGBAColor bgColor;
	rocket::glutils::RGBAColor bgFrameColor;
	rocket::glutils::RGBAColor fgColor;
	rocket::glutils::RGBAColor fgFrameColor;

	EnergyBarStyle() : bgColor(0x00004040), bgFrameColor(0x000000ff), fgColor(0x8080ff80), fgFrameColor(0x000000ff) {}
};

class EnergyBar : public rocket::game2d::Renderable {
public:
	EnergyBar(float width, float height) : width(width), height(height), value(0), maxValue(100), vertices(4) {}

	void setValues(uint32_t value, uint32_t maxValue) {
		this->value = value;
		this->maxValue = maxValue;
	}

	void setWidth(float width) { this->width = width; }
	void setHeight(float height) { this->height = height; }
	float getWidth() const { return width; }
	float getHeight() const { return height; }

	void setStyle(EnergyBarStyle const& style) { this->style = style; }

private:
	float width;
	float height;
	uint32_t value;
	uint32_t maxValue;
	EnergyBarStyle style;

	std::vector<glm::vec4> vertices;

	void renderImpl(rocket::graphics::Canvas &canvas);
};

}

#endif
