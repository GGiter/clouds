#pragma once

#include <vector>

class NoiseTextureGenerator
{
public:
	void GeneratePerlinTexture2D(int width, int height, std::vector<float>& data);
	void GenerateCurlTexture2D(int width, int height, std::vector<float>& data);
	void GenerateWorleyTexture3D(int width, int height, int depth, std::vector<float>& data);
	void GeneratePerlinWorleyTexture3D(int width, int height, int depth, std::vector<float> &data);
};

