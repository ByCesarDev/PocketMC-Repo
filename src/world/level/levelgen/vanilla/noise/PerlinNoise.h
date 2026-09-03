#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__PerlinNoise_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__PerlinNoise_H__

#include <vector>
#include "../../../../../util/Random.h"

namespace VanillaGenerator {

class PerlinNoise
{
public:
	double offsetX;
	double offsetY;
	double offsetZ;
	int perm[512];

	PerlinNoise();
	PerlinNoise(Random& rand);

	static int floor(double x);
	static double fade(double x);
	static double lerp(double x, double y, double z);
	static double grad(int hash, double x, double y, double z);

	double noise(double x, double y = 0.0, double z = 0.0);

	void getNoise(std::vector<double>& noise, double x, double y, double z, int sizeX, int sizeY, int sizeZ, double scaleX, double scaleY, double scaleZ, double amplitude);
	void get2dNoise(std::vector<double>& noise, double x, double z, int sizeX, int sizeZ, double scaleX, double scaleZ, double amplitude);
	void get3dNoise(std::vector<double>& noise, double x, double y, double z, int sizeX, int sizeY, int sizeZ, double scaleX, double scaleY, double scaleZ, double amplitude);
};

} // namespace VanillaGenerator

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_NOISE__PerlinNoise_H__*/
