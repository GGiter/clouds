
#include "TileableVolumeNoise.h"

#include "glm\gtc\noise.hpp"
#include <math.h>

// Perlin noise based on GLM http://glm.g-truc.net
// Worley noise based on https://www.shadertoy.com/view/Xl2XRR by Marc-Andre Loyer

float Tileable3dNoise::hash(float n)
{
	return glm::fract(sin(n+1.951f) * 43758.5453f);
}

// hash based 3d value noise
float Tileable3dNoise::noise(const glm::vec3& x)
{
	glm::vec3 p = glm::floor(x);
	glm::vec3 f = glm::fract(x);

	f = f*f*(glm::vec3(3.0f) - glm::vec3(2.0f) * f);
	float n = p.x + p.y*57.0f + 113.0f*p.z;
	return glm::mix(
		glm::mix(
			glm::mix(hash(n + 0.0f), hash(n + 1.0f), f.x),
			glm::mix(hash(n + 57.0f), hash(n + 58.0f), f.x),
			f.y),
		glm::mix(
			glm::mix(hash(n + 113.0f), hash(n + 114.0f), f.x),
			glm::mix(hash(n + 170.0f), hash(n + 171.0f), f.x),
			f.y),
		f.z);
}

float Tileable3dNoise::Cells(const glm::vec3& p, float cellCount)
{
	const glm::vec3 pCell = p * cellCount;
	float d = 1.0e10;
	for (int xo = -1; xo <= 1; xo++)
	{
		for (int yo = -1; yo <= 1; yo++)
		{
			for (int zo = -1; zo <= 1; zo++)
			{
				glm::vec3 tp = glm::floor(pCell) + glm::vec3(xo, yo, zo);

				tp = pCell - tp - noise(glm::mod(tp, cellCount / 1));

				d = glm::min(d, dot(tp, tp));
			}
		}
	}
	d = std::fminf(d, 1.0f);
	d = std::fmaxf(d, 0.0f);
	return d;
}


float Tileable3dNoise::WorleyNoise(const glm::vec3& p, float cellCount)
{
	return Cells(p, cellCount);
}

float Tileable3dNoise::PerlinNoise(const glm::vec3& pIn, float frequency, int octaveCount)
{
	const float octaveFrenquencyFactor = 2;			// noise frequency factor between octave, forced to 2

	// Compute the sum for each octave
	float sum = 0.0f;
	float weightSum = 0.0f;
	float weight = 0.5f;
	for (int oct = 0; oct < octaveCount; oct++)
	{
		// Perlin vec3 is bugged in GLM on the Z axis :(, black stripes are visible
		// So instead we use 4d Perlin and only use xyz...
		//glm::vec3 p(x * freq, y * freq, z * freq);
		//float val = glm::perlin(p, glm::vec3(freq)) *0.5 + 0.5;

		glm::vec4 p = glm::vec4(pIn.x, pIn.y, pIn.z, 0.0f) * glm::vec4(frequency);
		float val = glm::perlin(p, glm::vec4(frequency));

		sum += val * weight;
		weightSum += weight;

		weight *= weight;
		frequency *= octaveFrenquencyFactor;
	}

	float noise = (sum / weightSum) *0.5f + 0.5f;
	noise = std::fminf(noise, 1.0f);
	noise = std::fmaxf(noise, 0.0f);
	return noise;
 }

glm::vec3 Tileable3dNoise::noiseVec3(const glm::vec3& x)
{
  float s  = glm::perlin(glm::vec3( x ));
  float s1 = glm::perlin(glm::vec3( x.y - 19.1 , x.z + 33.4 , x.x + 47.2 ));
  float s2 = glm::perlin(glm::vec3( x.z + 74.2 , x.x - 124.5 , x.y + 99.4 ));
  glm::vec3 c = glm::vec3( s , s1 , s2 );
  return c;

}

glm::vec3 Tileable3dNoise::CurlNoise(const glm::vec3& p)
{	
	const float e = .1;
	glm::vec3 dx( e   , 0.0 , 0.0 );
	glm::vec3 dy( 0.0 , e   , 0.0 );
	glm::vec3 dz( 0.0 , 0.0 , e   );

	glm::vec3 p_x0 = noiseVec3( p - dx );
	glm::vec3 p_x1 = noiseVec3( p + dx );
	glm::vec3 p_y0 = noiseVec3( p - dy );
	glm::vec3 p_y1 = noiseVec3( p + dy );
	glm::vec3 p_z0 = noiseVec3( p - dz );
	glm::vec3 p_z1 = noiseVec3( p + dz );

	float x = p_y1.z - p_y0.z - p_z1.y + p_z0.y;
	float y = p_z1.x - p_z0.x - p_x1.z + p_x0.z;
	float z = p_x1.y - p_x0.y - p_y1.x + p_y0.x;

	const float divisor = 1.0 / ( 2.0 * e );
	return glm::normalize( glm::vec3( x , y , z ) * divisor );
}




