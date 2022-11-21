#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "ModelTriangle.h"

struct RayTriangleIntersection {
	glm::vec3 intersectionPoint;
	glm::vec2 textureIntersection;
	float distanceFromCamera;
	ModelTriangle intersectedTriangle;
	size_t triangleIndex;
	float u;
	float v;
	float w;

	RayTriangleIntersection();
	RayTriangleIntersection(const glm::vec3 &point, float distance, const ModelTriangle &triangle, size_t index, float uVal, float vVal);
	RayTriangleIntersection(const glm::vec3 &point, const glm::vec2 &texturePoint, float distance, const ModelTriangle &triangle, size_t index, float uVal, float vVal);
	friend std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection);
};
