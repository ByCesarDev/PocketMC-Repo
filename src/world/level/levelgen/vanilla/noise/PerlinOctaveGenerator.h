#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__PerlinOctaveGenerator_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__PerlinOctaveGenerator_H__

#include <vector>
#include "PerlinNoise.h"

namespace VanillaGenerator {

class PerlinOctaveGenerator
{
public:
	std::vector<PerlinNoise> octaves;
	int sizeX;
	int sizeY;
	int sizeZ;
	double scaleX;
	double scaleY;
	double scaleZ;

	PerlinOctaveGenerator();
	PerlinOctaveGenerator(Random& rand, int octavesCount, int sizeX, int sizeY, int sizeZ);

	std::vector<double> getFractalBrownianMotion(double x, double y, double z, double lacunarity, double persistence);
};

} // namespace VanillaGenerator

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__PerlinOctaveGenerator_H__*/
