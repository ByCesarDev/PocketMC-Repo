#include "SimplexNoise.h"
#include <cmath>

namespace VanillaGenerator {

static const double SQRT_3 = 1.7320508075688772;
static const double F2 = 0.5 * (SQRT_3 - 1.0);
static const double G2 = (3.0 - SQRT_3) / 6.0;
static const double G22 = G2 * 2.0 - 1.0;
static const double F3 = 1.0 / 3.0;
static const double G3 = 1.0 / 6.0;
static const double G32 = G3 * 2.0;

struct Grad {
	double x, y, z;
};

static const Grad GRAD_3[12] = {
	{1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0},
	{1, 0, 1}, {-1, 0, 1}, {1, 0, -1}, {-1, 0, -1},
	{0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1}
};

static inline double dot(const Grad& g, double x, double y, double z = 0.0) {
	return g.x * x + g.y * y + g.z * z;
}

static inline int fastFloor(double x) {
	return x > 0.0 ? (int)x : (int)x - 1;
}

SimplexNoise::SimplexNoise()
:	PerlinNoise()
{
	for (int i = 0; i < 512; ++i) permMod12[i] = 0;
}

SimplexNoise::SimplexNoise(Random& rand)
:	PerlinNoise(rand)
{
	for (int i = 0; i < 512; ++i) {
		permMod12[i] = perm[i] % 12;
	}
}

double SimplexNoise::simplex2D(double xin, double yin)
{
	double s = (xin + yin) * F2;
	int i = fastFloor(xin + s);
	int j = fastFloor(yin + s);
	double t = (double)(i + j) * G2;
	double dx0 = (double)i - t;
	double dy0 = (double)j - t;
	double x0 = xin - dx0;
	double y0 = yin - dy0;

	int i1 = 0;
	int j1 = 0;
	if (x0 > y0) {
		i1 = 1;
		j1 = 0;
	} else {
		i1 = 0;
		j1 = 1;
	}

	double x1 = x0 - (double)i1 + G2;
	double y1 = y0 - (double)j1 + G2;
	double x2 = x0 + G22;
	double y2 = y0 + G22;

	int ii = i & 255;
	int jj = j & 255;
	int gi0 = permMod12[ii + perm[jj]];
	int gi1 = permMod12[ii + i1 + perm[jj + j1]];
	int gi2 = permMod12[ii + 1 + perm[jj + 1]];

	double t0 = 0.5 - x0 * x0 - y0 * y0;
	double n0 = 0.0;
	if (t0 >= 0.0) {
		t0 *= t0;
		n0 = t0 * t0 * dot(GRAD_3[gi0], x0, y0);
	}

	double t1 = 0.5 - x1 * x1 - y1 * y1;
	double n1 = 0.0;
	if (t1 >= 0.0) {
		t1 *= t1;
		n1 = t1 * t1 * dot(GRAD_3[gi1], x1, y1);
	}

	double t2 = 0.5 - x2 * x2 - y2 * y2;
	double n2 = 0.0;
	if (t2 >= 0.0) {
		t2 *= t2;
		n2 = t2 * t2 * dot(GRAD_3[gi2], x2, y2);
	}

	return 70.0 * (n0 + n1 + n2);
}

double SimplexNoise::simplex3D(double xin, double yin, double zin)
{
	double s = (xin + yin + zin) * F3;
	int i = fastFloor(xin + s);
	int j = fastFloor(yin + s);
	int k = fastFloor(zin + s);
	double t = (double)(i + j + k) * G3;
	double dx0 = (double)i - t;
	double dy0 = (double)j - t;
	double dz0 = (double)k - t;

	int i1 = 0, j1 = 0, k1 = 0;
	int i2 = 0, j2 = 0, k2 = 0;

	double x0 = xin - dx0;
	double y0 = yin - dy0;
	double z0 = zin - dz0;

	if (x0 >= y0) {
		if (y0 >= z0) {
			i1 = 1; j1 = 0; k1 = 0;
			i2 = 1; j2 = 1; k2 = 0;
		} else if (x0 >= z0) {
			i1 = 1; j1 = 0; k1 = 0;
			i2 = 1; j2 = 0; k2 = 1;
		} else {
			i1 = 0; j1 = 0; k1 = 1;
			i2 = 1; j2 = 0; k2 = 1;
		}
	} else {
		if (y0 < z0) {
			i1 = 0; j1 = 0; k1 = 1;
			i2 = 0; j2 = 1; k2 = 1;
		} else if (x0 < z0) {
			i1 = 0; j1 = 1; k1 = 0;
			i2 = 0; j2 = 1; k2 = 1;
		} else {
			i1 = 0; j1 = 1; k1 = 0;
			i2 = 1; j2 = 1; k2 = 0;
		}
	}

	double x1 = x0 - (double)i1 + G3;
	double y1 = y0 - (double)j1 + G3;
	double z1 = z0 - (double)k1 + G3;
	double x2 = x0 - (double)i2 + G32;
	double y2 = y0 - (double)j2 + G32;
	double z2 = z0 - (double)k2 + G32;
	double x3 = x0 - 1.0 + 3.0 * G3;
	double y3 = y0 - 1.0 + 3.0 * G3;
	double z3 = z0 - 1.0 + 3.0 * G3;

	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;
	int gi0 = permMod12[ii + perm[jj + perm[kk]]];
	int gi1 = permMod12[ii + i1 + perm[jj + j1 + perm[kk + k1]]];
	int gi2 = permMod12[ii + i2 + perm[jj + j2 + perm[kk + k2]]];
	int gi3 = permMod12[ii + 1 + perm[jj + 1 + perm[kk + 1]]];

	double t0 = 0.5 - x0 * x0 - y0 * y0 - z0 * z0;
	double n0 = 0.0;
	if (t0 >= 0.0) {
		t0 *= t0;
		n0 = t0 * t0 * dot(GRAD_3[gi0], x0, y0, z0);
	}

	double t1 = 0.5 - x1 * x1 - y1 * y1 - z1 * z1;
	double n1 = 0.0;
	if (t1 >= 0.0) {
		t1 *= t1;
		n1 = t1 * t1 * dot(GRAD_3[gi1], x1, y1, z1);
	}

	double t2 = 0.5 - x2 * x2 - y2 * y2 - z2 * z2;
	double n2 = 0.0;
	if (t2 >= 0.0) {
		t2 *= t2;
		n2 = t2 * t2 * dot(GRAD_3[gi2], x2, y2, z2);
	}

	double t3 = 0.5 - x3 * x3 - y3 * y3 - z3 * z3;
	double n3 = 0.0;
	if (t3 >= 0.0) {
		t3 *= t3;
		n3 = t3 * t3 * dot(GRAD_3[gi3], x3, y3, z3);
	}

	return 32.0 * (n0 + n1 + n2 + n3);
}

void SimplexNoise::get2dNoise(std::vector<double>& noiseVec, double x, double z, int sizeX, int sizeZ, double scaleX, double scaleZ, double amplitude)
{
	int index = 0;
	for (int i = 0; i < sizeZ; ++i) {
		double zin = offsetY + (z + (double)i) * scaleZ;
		for (int j = 0; j < sizeX; ++j) {
			double xin = offsetX + (x + (double)j) * scaleX;
			if ((size_t)index >= noiseVec.size()) noiseVec.resize(index + 1, 0.0);
			noiseVec[index++] += simplex2D(xin, zin) * amplitude;
		}
	}
}

void SimplexNoise::get3dNoise(std::vector<double>& noiseVec, double x, double y, double z, int sizeX, int sizeY, int sizeZ, double scaleX, double scaleY, double scaleZ, double amplitude)
{
	int index = 0;
	for (int i = 0; i < sizeZ; ++i) {
		double zin = offsetZ + (z + (double)i) * scaleZ;
		for (int j = 0; j < sizeX; ++j) {
			double xin = offsetX + (x + (double)j) * scaleX;
			for (int k = 0; k < sizeY; ++k) {
				double yin = offsetY + (y + (double)k) * scaleY;
				if ((size_t)index >= noiseVec.size()) noiseVec.resize(index + 1, 0.0);
				noiseVec[index++] += simplex3D(xin, yin, zin) * amplitude;
			}
		}
	}
}

} // namespace VanillaGenerator
