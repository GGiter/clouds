#pragma once

#include <vector>
#include <string>

class TextureSaver
{
public:
	static bool SaveTextureToFile(const std::vector<float>& data, const std::string& path, int width, int depth, int comp);
};

