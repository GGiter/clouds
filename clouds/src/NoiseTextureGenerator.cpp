#include "NoiseTextureGenerator.h"
#include "TileableVolumeNoise/TileableVolumeNoise.h"
#include "glm/glm.hpp"

float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

float smoothstep(float edge0, float edge1, float x) {
		float t = std::min(std::max((x - edge0) / (edge1 - edge0), 0.0f), 1.0f);
    return t * t * (3.0 - 2.0 * t);
}

void NoiseTextureGenerator::GeneratePerlinTexture2D(int width, int height, std::vector<float>& data)
{
	data.resize(width * height * 4 + 4);
	const glm::vec3 normFactPerlin = glm::vec3(1.0f / float(width));
	int index = 0;
	for (int s=0; s<width; s++)
	{
		for (int t = 0; t<height; t++)
		{
			for (int r = 0; r<1; r++)
			{
				glm::vec3 coord = glm::vec3(s, t, r) * normFactPerlin;

				const int octaveCount = 1;
				const float frequency = 8;
				float noise0 = Tileable3dNoise::PerlinNoise(coord, frequency, octaveCount);
				float noise1 = Tileable3dNoise::PerlinNoise(coord, frequency * 2, octaveCount);
				float noise2 = Tileable3dNoise::PerlinNoise(coord, frequency * 4, octaveCount);

				int addr = r*width*height + t*height + s;
				data[index] = noise0;
				data[index + 1] = noise1;
				data[index + 2] = noise2;
				data[index + 3] = 1.0f;
				index += 4;
			}
		}
	}
}

void NoiseTextureGenerator::GenerateCurlTexture2D(int width, int height, std::vector<float>& data)
{
	data.resize(width * height * 4 + 4);
	for(int i = 0, index = 0; i < width * height; i += 3, index += 12)
	{
		glm::vec3 pos = glm::vec3(float((i/3)%128)/128.0f, float(((i/3)/128)%128)/128.0f, float((i/3)/(128*128))/128.0f);
		glm::vec3 noise = Tileable3dNoise::CurlNoise(pos);

		data[index] = remap(noise.x, -1.0f, 1.0f, 0.0f, 1.0f);
		data[index + 1] = data[index];
		data[index + 2] = data[index];
		data[index + 3] = 1.0f;

		data[index + 4] = remap(noise.y, -1.0f, 1.0f, 0.0f, 1.0f);
		data[index + 5] = data[index + 4];
		data[index + 6] = data[index + 4];
		data[index + 7] = 1.0f;

		data[index + 8] = remap(noise.z, -1.0f, 1.0f, 0.0f, 1.0f);
		data[index + 9] = data[index + 8];
		data[index + 10] = data[index + 8];
		data[index + 11] = 1.0f;
	}
}

void NoiseTextureGenerator::GenerateWorleyTexture3D(int width, int height, int depth, std::vector<float>& data)
{
	data.resize(width * height * depth * 4 + 4);

	const float frequenceMul[6] = { 2.0f,8.0f,14.0f };

	const glm::vec3 normFactWorley = glm::vec3(1.0f / float(width));
	int index = 0;
	for (int s = 0; s<width; s++)
	{
		for (int t = 0; t<height; t++)
		{
			for (int r = 0; r<depth; r++)
			{
				glm::vec3 coord = glm::vec3(s, t, r) * normFactWorley;

				const float cellCount = 3;
				float noise0 = 1.0 - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[0]);
				float noise1 = 1.0 - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[1]);
				float noise2 = 1.0 - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[2]);
				data[index] = noise0;
				data[index + 1] = noise1;
				data[index + 2] = noise2;
				data[index + 3] = 1.0f;
				index += 4;
			}
		}
	}
}

void NoiseTextureGenerator::GeneratePerlinWorleyTexture3D(int width, int height, int depth, std::vector<float>& data)
{
	data.resize(width * height * depth * 4 + 4);
	const float frequenceMul[6] = { 2.0f,8.0f,14.0f,20.0f,26.0f,32.0f };
	int index = 0;
	for (int s = 0; s<width; s++)
	{
		const glm::vec3 normFact = glm::vec3(1.0f / float(width));
		for (int t = 0; t<height; t++)
		{
			for (int r = 0; r<depth; r++)
			{
				glm::vec3 coord = glm::vec3(s, t, r) * normFact;

				// Perlin FBM noise
				const int octaveCount = 3;
				const float frequency = 8.0f;
				float perlinNoise = Tileable3dNoise::PerlinNoise(coord, frequency, octaveCount);

				float PerlinWorleyNoise = 0.0f;
				{
					const float cellCount = 4;
					const float worleyNoise0 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[0]));
					const float worleyNoise1 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[1]));
					const float worleyNoise2 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[2]));
					const float worleyNoise3 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[3]));
					const float worleyNoise4 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[4]));
					const float worleyNoise5 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * frequenceMul[5]));	// half the frequency of texel, we should not go further (with cellCount = 32 and texture size = 64)

					float worleyFBM = worleyNoise0*0.625f + worleyNoise1*0.25f + worleyNoise2*0.125f;

					// Perlin Worley is based on description in GPU Pro 7: Real Time Volumetric Cloudscapes.
					// However it is not clear the text and the image are matching: images does not seem to match what the result  from the description in text would give.
					// Also there are a lot of fudge factor in the code, e.g. *0.2, so it is really up to you to fine the formula you like.
					//PerlinWorleyNoise = remap(worleyFBM, 0.0, 1.0, 0.0, perlinNoise);	// Matches better what figure 4.7 (not the following up text description p.101). Maps worley between newMin as 0 and 
					PerlinWorleyNoise = remap(perlinNoise, 0.0f, 1.0f, worleyFBM, 1.0f);	// mapping perlin noise in between worley as minimum and 1.0 as maximum (as described in text of p.101 of GPU Pro 7) 
				}

				const float cellCount = 4;
				float worleyNoise0 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 1));
				float worleyNoise1 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 2));
				float worleyNoise2 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 4));
				float worleyNoise3 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 8));
				float worleyNoise4 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 16));
				//float worleyNoise5 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 32));	//cellCount=2 -> half the frequency of texel, we should not go further (with cellCount = 32 and texture size = 64)

				// Three frequency of Worley FBM noise
				float worleyFBM0 = worleyNoise1*0.625f + worleyNoise2*0.25f + worleyNoise3*0.125f;
				float worleyFBM1 = worleyNoise2*0.625f + worleyNoise3*0.25f + worleyNoise4*0.125f;
				//float worleyFBM2 = worleyNoise3*0.625f + worleyNoise4*0.25f + worleyNoise5*0.125f;
				float worleyFBM2 = worleyNoise3*0.75f + worleyNoise4*0.25f; // cellCount=4 -> worleyNoise5 is just noise due to sampling frequency=texel frequency. So only take into account 2 frequencies for FBM

				float value = 0.0;
				{
					// pack the channels for direct usage in shader
					float lowFreqFBM = worleyFBM0*0.625f + worleyFBM1*0.25f + worleyFBM2*0.125f;
					float baseCloud = PerlinWorleyNoise;
					value = remap(baseCloud, -(1.0f - lowFreqFBM), 1.0f, 0.0f, 1.0f);
					// Saturate
					value = std::fminf(value, 1.0f);
					value = std::fmaxf(value, 0.0f);
				}
				data[index] = PerlinWorleyNoise;
				data[index + 1] = worleyFBM0;
				data[index + 2] = worleyFBM1;
				data[index + 3] = worleyFBM2;
				index += 4;
			}
		}
	}
}
