#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__SimplexNoise_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__SimplexNoise_H__

#include "PerlinNoise.h"

namespace VanillaGenerator {

class SimplexNoise : public PerlinNoise
{
public:
	int permMod12[512];

	SimplexNoise();
	SimplexNoise(Random& rand);

	double simplex2D(double xin, double yin);
	double simplex3D(double xin, double yin, double zin);

	void get2dNoise(std::vector<double>& noise, double x, double z, int sizeX, int sizeZ, double scaleX, double scaleZ, double amplitude);
	void get3dNoise(std::vector<double>& noise, double x, double y, double z, int sizeX, int sizeY, int sizeZ, double scaleX, double scaleY, double scaleZ, double amplitude);
};

} // namespace VanillaGenerator

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__SimplexNoise_H__*/
