#include "EnergyBar.h"

using namespace rocket::graphics;

namespace rocketcar {

void EnergyBar::renderImpl(Canvas &canvas) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	vertices[0] = glm::vec4(-width/2.0f, -height/2.0f, 0, 1);
	vertices[1] = glm::vec4(width/2.0f, -height/2.0f, 0, 1);
	vertices[2] = glm::vec4(width/2.0f, height/2.0f, 0, 1);
	vertices[3] = glm::vec4(-width/2.0f, height/2.0f, 0, 1);

	canvas.fillPolygon(vertices, style.bgColor);
	canvas.drawPolygon(vertices, style.bgFrameColor);

	float distance = width/static_cast<float>(maxValue); // Distance between center of two neighbouring blocks.

	float blockWidth = distance * 0.8f;
	float blockHeight = height * 0.8f;

	glm::vec2 p(-width/2.0f + distance/2.0f + 0.1f * distance, 0);

	for (uint32_t i = 0; i < value; ++i) {
		vertices[0] = glm::vec4(p.x - blockWidth/2.0f, p.y - blockHeight/2.0f, 0, 1);
		vertices[1] = glm::vec4(p.x + blockWidth/2.0f, p.y - blockHeight/2.0f, 0, 1);
		vertices[2] = glm::vec4(p.x + blockWidth/2.0f, p.y + blockHeight/2.0f, 0, 1);
		vertices[3] = glm::vec4(p.x - blockWidth/2.0f, p.y + blockHeight/2.0f, 0, 1);

		canvas.fillPolygon(vertices, style.fgColor);
		canvas.drawPolygon(vertices, style.fgFrameColor);

		p.x += distance; // or rather (maxValue+1)/width
	}

	glDisable(GL_BLEND);
}

}
