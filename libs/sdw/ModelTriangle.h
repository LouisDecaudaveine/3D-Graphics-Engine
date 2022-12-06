#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"

struct ModelTriangle {
	std::array<glm::vec3, 3> vertices{};
	std::array<glm::vec2,3> verticesLighting{};
	std::array<glm::vec3, 3> verticesNormals{};
	std::array<TexturePoint, 3> texturePoints{};
	Colour colour{};
	glm::vec3 normal{};
	bool hasTexture {};
	bool isReflective {};
	std::string textureMap{};
	std::string bumpsMap {};

	ModelTriangle();
	ModelTriangle(
		const glm::vec3 &v0, 
		const glm::vec3 &v1, 
		const glm::vec3 &v2, 
		Colour trigColour);

	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
