#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__SimplexOctaveGenerator_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__SimplexOctaveGenerator_H__

#include <vector>
#include "SimplexNoise.h"

namespace VanillaGenerator {

class SimplexOctaveGenerator
{
public:
	std::vector<SimplexNoise> octaves;
	int sizeX;
	int sizeY;
	int sizeZ;
	double scaleX;
	double scaleY;
	double scaleZ;

	SimplexOctaveGenerator();
	SimplexOctaveGenerator(Random& rand, int octavesCount, int sizeX, int sizeY, int sizeZ);

	void setScale(double scale);
	std::vector<double> getFractalBrownianMotion(double x, double y, double z, double lacunarity, double persistence);
};

} // namespace VanillaGenerator

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__SimplexOctaveGenerator_H__*/
