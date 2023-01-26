#pragma once

#include <iostream>
#include <vector>
#include "glm/glm.hpp"

class OpenVDBReader
{
public:
	bool Read(const std::string& path, std::vector<float>& data, glm::ivec3& dimensions);
};