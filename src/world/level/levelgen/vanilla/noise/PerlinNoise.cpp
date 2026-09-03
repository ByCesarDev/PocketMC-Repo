#include "PerlinNoise.h"
#include <cmath>

namespace VanillaGenerator {

static const int GRAD3[12][3] = {
	{1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0},
	{1, 0, 1}, {-1, 0, 1}, {1, 0, -1}, {-1, 0, -1},
	{0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1}
};

PerlinNoise::PerlinNoise()
:	offsetX(0.0), offsetY(0.0), offsetZ(0.0)
{
	for (int i = 0; i < 512; ++i) perm[i] = 0;
}

PerlinNoise::PerlinNoise(Random& rand)
{
	offsetX = rand.nextFloat() * 256.0;
	offsetY = rand.nextFloat() * 256.0;
	offsetZ = rand.nextFloat() * 256.0;

	for (int i = 0; i < 256; ++i) {
		perm[i] = i;
	}

	for (int i = 0; i < 256; ++i) {
		int pos = rand.nextInt(256 - i) + i;
		int old = perm[i];
		perm[i] = perm[pos];
		perm[pos] = old;
		perm[i + 256] = perm[i];
	}
}

int PerlinNoise::floor(double x)
{
	int floored = (int)x;
	return x < (double)floored ? floored - 1 : floored;
}

double PerlinNoise::fade(double x)
{
	return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
}

double PerlinNoise::lerp(double x, double y, double z)
{
	return y + x * (z - y);
}

double PerlinNoise::grad(int hash, double x, double y, double z)
{
	const int* g = GRAD3[hash & 11];
	return (double)g[0] * x + (double)g[1] * y + (double)g[2] * z;
}

double PerlinNoise::noise(double x, double y, double z)
{
	x += offsetX;
	y += offsetY;
	z += offsetZ;

	int floorX = floor(x);
	int floorY = floor(y);
	int floorZ = floor(z);

	int X = floorX & 255;
	int Y = floorY & 255;
	int Z = floorZ & 255;

	x -= floorX;
	y -= floorY;
	z -= floorZ;

	double fX = fade(x);
	double fY = fade(y);
	double fZ = fade(z);

	int A = perm[X] + Y;
	int AA = perm[A] + Z;
	int AB = perm[A + 1] + Z;
	int B = perm[X + 1] + Y;
	int BA = perm[B] + Z;
	int BB = perm[B + 1] + Z;

	return lerp(fZ, lerp(fY, lerp(fX, grad(perm[AA], x, y, z),
		grad(perm[BA], x - 1.0, y, z)),
		lerp(fX, grad(perm[AB], x, y - 1.0, z),
			grad(perm[BB], x - 1.0, y - 1.0, z))),
		lerp(fY, lerp(fX, grad(perm[AA + 1], x, y, z - 1.0),
			grad(perm[BA + 1], x - 1.0, y, z - 1.0)),
			lerp(fX, grad(perm[AB + 1], x, y - 1.0, z - 1.0),
				grad(perm[BB + 1], x - 1.0, y - 1.0, z - 1.0))));
}

void PerlinNoise::getNoise(std::vector<double>& noiseVec, double x, double y, double z, int sizeX, int sizeY, int sizeZ, double scaleX, double scaleY, double scaleZ, double amplitude)
{
	if (sizeY == 1) {
		get2dNoise(noiseVec, x, z, sizeX, sizeZ, scaleX, scaleZ, amplitude);
	} else {
		get3dNoise(noiseVec, x, y, z, sizeX, sizeY, sizeZ, scaleX, scaleY, scaleZ, amplitude);
	}
}

void PerlinNoise::get2dNoise(std::vector<double>& noiseVec, double x, double z, int sizeX, int sizeZ, double scaleX, double scaleZ, double amplitude)
{
	int index = 0;
	for (int i = 0; i < sizeX; ++i) {
		double dx = x + offsetX + (double)i * scaleX;
		int floorX = floor(dx);
		int ix = floorX & 255;
		dx -= floorX;
		double fx = fade(dx);
		for (int j = 0; j < sizeZ; ++j) {
			double dz = z + offsetZ + (double)j * scaleZ;
			int floorZ = floor(dz);
			int iz = floorZ & 255;
			dz -= floorZ;
			double fz = fade(dz);

			int a = perm[ix];
			int aa = perm[a] + iz;
			int b = perm[ix + 1];
			int ba = perm[b] + iz;

			double x1 = lerp(fx, grad(perm[aa], dx, 0.0, dz), grad(perm[ba], dx - 1.0, 0.0, dz));
			double x2 = lerp(fx, grad(perm[aa + 1], dx, 0.0, dz - 1.0), grad(perm[ba + 1], dx - 1.0, 0.0, dz - 1.0));

			if ((size_t)index >= noiseVec.size()) noiseVec.resize(index + 1, 0.0);
			noiseVec[index++] += lerp(fz, x1, x2) * amplitude;
		}
	}
}

void PerlinNoise::get3dNoise(std::vector<double>& noiseVec, double x, double y, double z, int sizeX, int sizeY, int sizeZ, double scaleX, double scaleY, double scaleZ, double amplitude)
{
	int index = 0;
	for (int i = 0; i < sizeX; ++i) {
		double dx = x + offsetX + (double)i * scaleX;
		int floorX = floor(dx);
		int ix = floorX & 255;
		dx -= floorX;
		double fx = fade(dx);
		for (int j = 0; j < sizeZ; ++j) {
			double dz = z + offsetZ + (double)j * scaleZ;
			int floorZ = floor(dz);
			int iz = floorZ & 255;
			dz -= floorZ;
			double fz = fade(dz);
			for (int k = 0; k < sizeY; ++k) {
				double dy = y + offsetY + (double)k * scaleY;
				int floorY = floor(dy);
				int iy = floorY & 255;
				dy -= floorY;
				double fy = fade(dy);

				int a = perm[ix] + iy;
				int aa = perm[a] + iz;
				int ab = perm[a + 1] + iz;
				int b = perm[ix + 1] + iy;
				int ba = perm[b] + iz;
				int bb = perm[b + 1] + iz;

				double x1 = lerp(fx, grad(perm[aa], dx, dy, dz), grad(perm[ba], dx - 1.0, dy, dz));
				double x2 = lerp(fx, grad(perm[ab], dx, dy - 1.0, dz), grad(perm[bb], dx - 1.0, dy - 1.0, dz));
				double x3 = lerp(fx, grad(perm[aa + 1], dx, dy, dz - 1.0), grad(perm[ba + 1], dx - 1.0, dy, dz - 1.0));
				double x4 = lerp(fx, grad(perm[ab + 1], dx, dy - 1.0, dz - 1.0), grad(perm[bb + 1], dx - 1.0, dy - 1.0, dz - 1.0));

				double y1 = lerp(fy, x1, x2);
				double y2 = lerp(fy, x3, x4);

				if ((size_t)index >= noiseVec.size()) noiseVec.resize(index + 1, 0.0);
				noiseVec[index++] += lerp(fz, y1, y2) * amplitude;
			}
		}
	}
}

} // namespace VanillaGenerator
