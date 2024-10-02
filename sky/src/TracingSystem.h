#pragma once
#include "AABBCollision.h";

class TracingSystem
{
public:
	static bool DoesAABBIntersect(AABBCollision* A, AABBCollision* B);
	static bool DoesAABBOverlap(AABBCollision* A, AABBCollision* B);
	static glm::vec3 GetIntersectionPoint(AABBCollision* A, AABBCollision* B);
};

