#include "PerlinOctaveGenerator.h"

namespace VanillaGenerator {

PerlinOctaveGenerator::PerlinOctaveGenerator()
:	sizeX(1), sizeY(1), sizeZ(1), scaleX(1.0), scaleY(1.0), scaleZ(1.0)
{
}

PerlinOctaveGenerator::PerlinOctaveGenerator(Random& rand, int octavesCount, int sizeX, int sizeY, int sizeZ)
:	sizeX(sizeX), sizeY(sizeY), sizeZ(sizeZ), scaleX(1.0), scaleY(1.0), scaleZ(1.0)
{
	octaves.reserve(octavesCount);
	for (int i = 0; i < octavesCount; ++i) {
		octaves.emplace_back(rand);
	}
}

std::vector<double> PerlinOctaveGenerator::getFractalBrownianMotion(double x, double y, double z, double lacunarity, double persistence)
{
	std::vector<double> noise(sizeX * sizeY * sizeZ, 0.0);

	double freq = 1.0;
	double amp = 1.0;

	for (size_t i = 0; i < octaves.size(); ++i) {
		double dx = x * freq * scaleX;
		double dy = y * freq * scaleY;
		double dz = z * freq * scaleZ;
		octaves[i].getNoise(noise, dx, dy, dz, sizeX, sizeY, sizeZ, scaleX * freq, scaleY * freq, scaleZ * freq, amp);
		freq *= lacunarity;
		amp *= persistence;
	}

	return noise;
}

} // namespace VanillaGenerator
