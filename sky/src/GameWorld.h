#pragma once
#include "AABBCollision.h"
#include <vector>

class GameWorld
{
public:
	static std::vector<AABBCollision*> AABBs;
	static glm::vec3 WorldExtent;
};

