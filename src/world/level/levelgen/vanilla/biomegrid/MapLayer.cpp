#include "MapLayer.h"
#include <algorithm>
#include <map>

MapLayer::MapLayer(int64_t seed)
:	seed(seed), random(seed)
{
}

void MapLayer::setCoordsSeed(int x, int z)
{
	random.setSeed(seed);
	int64_t next1 = random.nextInt();
	int64_t next2 = random.nextInt();
	int64_t s = (int64_t)x * next1 + (int64_t)z * next2 ^ seed;
	random.setSeed(s);
}

int MapLayer::nextInt(int max)
{
	return max > 0 ? random.nextInt(max) : 0;
}

// --------------------------------------------------------------------------
// Layers
// --------------------------------------------------------------------------

class NoiseMapLayer : public MapLayer {
public:
	NoiseMapLayer(int64_t seed) : MapLayer(seed) {}
	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		std::vector<int> values(sizeX * sizeZ);
		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				setCoordsSeed(x + j, z + i);
				values[j + i * sizeX] = (nextInt(10) == 0) ? 1 : 0;
			}
		}
		return values;
	}
};

class WhittakerMapLayer : public MapLayer {
public:
	enum Type { WARM_WET = 0, COLD_DRY = 1, LARGER_BIOMES = 2 };
	std::shared_ptr<MapLayer> below;
	Type type;

	WhittakerMapLayer(int64_t seed, std::shared_ptr<MapLayer> below, Type type)
	:	MapLayer(seed), below(below), type(type) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		if (type == WARM_WET || type == COLD_DRY) {
			int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
			auto vals = below->generateValues(gx, gz, gsx, gsz);
			std::vector<int> res(sizeX * sizeZ);
			int targetVal = (type == WARM_WET) ? 2 : 3;
			int finalVal = (type == WARM_WET) ? 4 : 1;
			int cross1 = (type == WARM_WET) ? 3 : 2;
			int cross2 = (type == WARM_WET) ? 1 : 4;

			for (int i = 0; i < sizeZ; ++i) {
				for (int j = 0; j < sizeX; ++j) {
					int c = vals[(j + 1) + (i + 1) * gsx];
					if (c == targetVal) {
						int u = vals[(j + 1) + i * gsx];
						int d = vals[(j + 1) + (i + 2) * gsx];
						int l = vals[j + (i + 1) * gsx];
						int r = vals[(j + 2) + (i + 1) * gsx];
						if (u == cross1 || u == cross2 || d == cross1 || d == cross2 ||
							l == cross1 || l == cross2 || r == cross1 || r == cross2) {
							c = finalVal;
						}
					}
					res[j + i * sizeX] = c;
				}
			}
			return res;
		}

		auto vals = below->generateValues(x, z, sizeX, sizeZ);
		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int& v = vals[j + i * sizeX];
				if (v != 0) {
					setCoordsSeed(x + j, z + i);
					if (nextInt(13) == 0) {
						v += 1000;
					}
				}
			}
		}
		return vals;
	}
};

class ZoomMapLayer : public MapLayer {
public:
	enum ZoomType { NORMAL = 0, BLURRY = 1 };
	std::shared_ptr<MapLayer> below;
	ZoomType zoomType;

	ZoomMapLayer(int64_t seed, std::shared_ptr<MapLayer> below, ZoomType zoomType = NORMAL)
	:	MapLayer(seed), below(below), zoomType(zoomType) {}

	int getNearest(int a, int b, int c, int d) {
		if (zoomType == NORMAL) {
			if (b == c && c == d) return b;
			if (a == b && a == c) return a;
			if (a == b && a == d) return a;
			if (a == c && a == d) return a;
			if (a == b && c != d) return a;
			if (a == c && b != d) return a;
			if (a == d && b != c) return a;
			if (b == c && a != d) return b;
			if (b == d && a != c) return b;
			if (c == d && a != b) return c;
		}
		int choices[4] = {a, b, c, d};
		return choices[nextInt(4)];
	}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		int gx = x >> 1;
		int gz = z >> 1;
		int gsx = (sizeX >> 1) + 2;
		int gsz = (sizeZ >> 1) + 2;
		auto vals = below->generateValues(gx, gz, gsx, gsz);

		int zsx = (gsx - 1) << 1;
		int zsz = (gsz - 1) << 1;
		std::vector<int> tmp(zsx * zsz, 0);

		for (int i = 0; i < gsz - 1; ++i) {
			int n = i * 2 * zsx;
			int ul = vals[i * gsx];
			int ll = vals[(i + 1) * gsx];
			for (int j = 0; j < gsx - 1; ++j) {
				setCoordsSeed((gx + j) << 1, (gz + i) << 1);
				tmp[n] = ul;
				tmp[n + zsx] = (nextInt(2) > 0) ? ul : ll;
				int ur = vals[j + 1 + i * gsx];
				int lr = vals[j + 1 + (i + 1) * gsx];
				tmp[n + 1] = (nextInt(2) > 0) ? ul : ur;
				tmp[n + 1 + zsx] = getNearest(ul, ur, ll, lr);
				ul = ur;
				ll = lr;
				n += 2;
			}
		}

		std::vector<int> res(sizeX * sizeZ);
		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				res[j + i * sizeX] = tmp[j + (i + (z & 1)) * zsx + (x & 1)];
			}
		}
		return res;
	}
};

class ErosionMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	ErosionMapLayer(int64_t seed, std::shared_ptr<MapLayer> below) : MapLayer(seed), below(below) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
		auto vals = below->generateValues(gx, gz, gsx, gsz);
		std::vector<int> res(sizeX * sizeZ);

		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int c = vals[(j + 1) + (i + 1) * gsx];
				int u = vals[(j + 1) + i * gsx];
				int d = vals[(j + 1) + (i + 2) * gsx];
				int l = vals[j + (i + 1) * gsx];
				int r = vals[(j + 2) + (i + 1) * gsx];

				if (c != 0 && (u == 0 || d == 0 || l == 0 || r == 0)) {
					setCoordsSeed(x + j, z + i);
					c = (nextInt(2) == 0) ? 0 : c;
				}
				res[j + i * sizeX] = c;
			}
		}
		return res;
	}
};

class DeepOceanMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	DeepOceanMapLayer(int64_t seed, std::shared_ptr<MapLayer> below) : MapLayer(seed), below(below) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
		auto vals = below->generateValues(gx, gz, gsx, gsz);
		std::vector<int> res(sizeX * sizeZ);

		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int c = vals[(j + 1) + (i + 1) * gsx];
				if (c == 0) {
					int u = vals[(j + 1) + i * gsx];
					int d = vals[(j + 1) + (i + 2) * gsx];
					int l = vals[j + (i + 1) * gsx];
					int r = vals[(j + 2) + (i + 1) * gsx];
					if (u == 0 && d == 0 && l == 0 && r == 0) {
						c = VanillaBiomeIds::DEEP_OCEAN;
					}
				}
				res[j + i * sizeX] = c;
			}
		}
		return res;
	}
};

class BiomeMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	BiomeMapLayer(int64_t seed, std::shared_ptr<MapLayer> below) : MapLayer(seed), below(below) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		using namespace VanillaBiomeIds;
		static const int WARM[] = {DESERT, DESERT, DESERT, SAVANNA, SAVANNA, PLAINS};
		static const int WET[]  = {PLAINS, PLAINS, FOREST, BIRCH_FOREST, ROOFED_FOREST, EXTREME_HILLS, SWAMPLAND};
		static const int DRY[]  = {PLAINS, FOREST, TAIGA, EXTREME_HILLS};
		static const int COLD[] = {ICE_PLAINS, ICE_PLAINS, COLD_TAIGA};
		static const int WARM_L[] = {MESA_PLATEAU_STONE, MESA_PLATEAU_STONE, MESA_PLATEAU};
		static const int DRY_L[]  = {MEGA_TAIGA};
		static const int WET_L[]  = {JUNGLE};

		auto vals = below->generateValues(x, z, sizeX, sizeZ);
		std::vector<int> res(sizeX * sizeZ);

		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int v = vals[j + i * sizeX];
				if (v != 0) {
					setCoordsSeed(x + j, z + i);
					switch (v) {
					case 1: v = DRY[nextInt(4)]; break;
					case 2: v = WARM[nextInt(6)]; break;
					case 3:
					case 1003: v = COLD[nextInt(3)]; break;
					case 4: v = WET[nextInt(7)]; break;
					case 1001: v = DRY_L[nextInt(1)]; break;
					case 1002: v = WARM_L[nextInt(3)]; break;
					case 1004: v = WET_L[nextInt(1)]; break;
					}
				}
				res[j + i * sizeX] = v;
			}
		}
		return res;
	}
};

class BiomeVariationMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	std::shared_ptr<MapLayer> variation;

	BiomeVariationMapLayer(int64_t seed, std::shared_ptr<MapLayer> below, std::shared_ptr<MapLayer> variation = nullptr)
	:	MapLayer(seed), below(below), variation(variation) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		using namespace VanillaBiomeIds;
		if (!variation) {
			auto vals = below->generateValues(x, z, sizeX, sizeZ);
			for (int i = 0; i < sizeZ; ++i) {
				for (int j = 0; j < sizeX; ++j) {
					int& v = vals[j + i * sizeX];
					if (v > 0) {
						setCoordsSeed(x + j, z + i);
						v = nextInt(30) + 2;
					}
				}
			}
			return vals;
		}

		int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
		auto vals = below->generateValues(gx, gz, gsx, gsz);
		auto vars = variation->generateValues(x, z, sizeX, sizeZ);
		std::vector<int> res(sizeX * sizeZ);

		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int c = vals[(j + 1) + (i + 1) * gsx];
				int var = vars[j + i * sizeX];
				setCoordsSeed(x + j, z + i);

				if (c != 0 && var >= 2 && (var - 2) % 29 == 0) {
					if (c == DESERT) c = DESERT_HILLS;
					else if (c == FOREST) c = FOREST_HILLS;
					else if (c == BIRCH_FOREST) c = BIRCH_FOREST_HILLS;
					else if (c == ROOFED_FOREST) c = PLAINS;
					else if (c == TAIGA) c = TAIGA_HILLS;
					else if (c == MEGA_TAIGA) c = MEGA_TAIGA_HILLS;
					else if (c == COLD_TAIGA) c = COLD_TAIGA_HILLS;
					else if (c == PLAINS) c = (nextInt(3) == 0) ? FOREST_HILLS : FOREST;
					else if (c == ICE_PLAINS) c = ICE_MOUNTAINS;
					else if (c == JUNGLE) c = JUNGLE_HILLS;
					else if (c == OCEAN) c = DEEP_OCEAN;
					else if (c == EXTREME_HILLS) c = EXTREME_HILLS_PLUS_TREES;
					else if (c == SAVANNA) c = SAVANNA_PLATEAU;
					else if (c == MESA_PLATEAU_STONE || c == MESA_PLATEAU || c == MESA) c = MESA;
				} else if (nextInt(3) == 0 || var == 0) {
					int mut = c;
					if (c == PLAINS) mut = SUNFLOWER_PLAINS;
					else if (c == DESERT) mut = DESERT_MUTATED;
					else if (c == FOREST) mut = FLOWER_FOREST;
					else if (c == TAIGA) mut = TAIGA_MUTATED;
					else if (c == SWAMPLAND) mut = SWAMPLAND_MUTATED;
					else if (c == ICE_PLAINS) mut = ICE_PLAINS_SPIKES;
					else if (c == JUNGLE) mut = JUNGLE_MUTATED;
					else if (c == JUNGLE_EDGE) mut = JUNGLE_EDGE_MUTATED;
					else if (c == BIRCH_FOREST) mut = BIRCH_FOREST_MUTATED;
					else if (c == BIRCH_FOREST_HILLS) mut = BIRCH_FOREST_HILLS_MUTATED;
					else if (c == ROOFED_FOREST) mut = ROOFED_FOREST_MUTATED;
					else if (c == COLD_TAIGA) mut = COLD_TAIGA_MUTATED;
					else if (c == REDWOOD_TAIGA_MUTATED) mut = REDWOOD_TAIGA_HILLS_MUTATED;
					else if (c == MEGA_TAIGA) mut = REDWOOD_TAIGA_MUTATED;
					else if (c == MEGA_TAIGA_HILLS) mut = REDWOOD_TAIGA_HILLS_MUTATED;
					else if (c == EXTREME_HILLS) mut = EXTREME_HILLS_MUTATED;
					else if (c == EXTREME_HILLS_PLUS_TREES) mut = EXTREME_HILLS_PLUS_TREES_MUTATED;
					else if (c == SAVANNA) mut = SAVANNA_MUTATED;
					else if (c == SAVANNA_PLATEAU) mut = SAVANNA_PLATEAU_MUTATED;
					else if (c == MESA) mut = MESA_BRYCE;
					else if (c == MESA_PLATEAU_STONE) mut = MESA_PLATEAU_STONE_MUTATED;
					else if (c == MESA_PLATEAU) mut = MESA_PLATEAU_MUTATED;

					if (mut != c) {
						int u = vals[(j + 1) + i * gsx];
						int d = vals[(j + 1) + (i + 2) * gsx];
						int l = vals[j + (i + 1) * gsx];
						int r = vals[(j + 2) + (i + 1) * gsx];
						if (u == c && d == c && l == c && r == c) {
							c = mut;
						}
					}
				}
				res[j + i * sizeX] = c;
			}
		}
		return res;
	}
};

class BiomeEdgeMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	BiomeEdgeMapLayer(int64_t seed, std::shared_ptr<MapLayer> below) : MapLayer(seed), below(below) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		using namespace VanillaBiomeIds;
		int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
		auto vals = below->generateValues(gx, gz, gsx, gsz);
		std::vector<int> res(sizeX * sizeZ);

		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int c = vals[(j + 1) + (i + 1) * gsx];
				int u = vals[(j + 1) + i * gsx];
				int d = vals[(j + 1) + (i + 2) * gsx];
				int l = vals[j + (i + 1) * gsx];
				int r = vals[(j + 2) + (i + 1) * gsx];

				if (c == MESA_PLATEAU_STONE || c == MESA_PLATEAU) {
					if (u != c || d != c || l != c || r != c) c = MESA;
				} else if (c == MEGA_TAIGA) {
					if (u != c || d != c || l != c || r != c) c = TAIGA;
				} else if (c == DESERT) {
					if (u == ICE_PLAINS || d == ICE_PLAINS || l == ICE_PLAINS || r == ICE_PLAINS) {
						c = EXTREME_HILLS_PLUS_TREES;
					}
				} else if (c == SWAMPLAND) {
					if (u == DESERT || d == DESERT || l == DESERT || r == DESERT ||
						u == COLD_TAIGA || d == COLD_TAIGA || l == COLD_TAIGA || r == COLD_TAIGA ||
						u == ICE_PLAINS || d == ICE_PLAINS || l == ICE_PLAINS || r == ICE_PLAINS) {
						c = PLAINS;
					} else if (u == JUNGLE || d == JUNGLE || l == JUNGLE || r == JUNGLE) {
						c = JUNGLE_EDGE;
					}
				}
				res[j + i * sizeX] = c;
			}
		}
		return res;
	}
};

class BiomeThinEdgeMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	BiomeThinEdgeMapLayer(int64_t seed, std::shared_ptr<MapLayer> below) : MapLayer(seed), below(below) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		using namespace VanillaBiomeIds;
		int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
		auto vals = below->generateValues(gx, gz, gsx, gsz);
		std::vector<int> res(sizeX * sizeZ);

		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int c = vals[(j + 1) + (i + 1) * gsx];
				int u = vals[(j + 1) + i * gsx];
				int d = vals[(j + 1) + (i + 2) * gsx];
				int l = vals[j + (i + 1) * gsx];
				int r = vals[(j + 2) + (i + 1) * gsx];

				if (c == JUNGLE || c == JUNGLE_MUTATED || c == JUNGLE_HILLS) {
					auto isJungle = [](int b) {
						return b == JUNGLE || b == JUNGLE_MUTATED || b == JUNGLE_HILLS || b == JUNGLE_EDGE || b == JUNGLE_EDGE_MUTATED;
					};
					if (!isJungle(u) || !isJungle(d) || !isJungle(l) || !isJungle(r)) {
						c = (c == JUNGLE_MUTATED) ? JUNGLE_EDGE_MUTATED : JUNGLE_EDGE;
					}
				}
				res[j + i * sizeX] = c;
			}
		}
		return res;
	}
};

class RarePlainsMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	RarePlainsMapLayer(int64_t seed, std::shared_ptr<MapLayer> below) : MapLayer(seed), below(below) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		auto vals = below->generateValues(x, z, sizeX, sizeZ);
		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int& v = vals[j + i * sizeX];
				if (v == VanillaBiomeIds::PLAINS) {
					setCoordsSeed(x + j, z + i);
					if (nextInt(57) == 0) {
						v = VanillaBiomeIds::SUNFLOWER_PLAINS;
					}
				}
			}
		}
		return vals;
	}
};

class ShoreMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	ShoreMapLayer(int64_t seed, std::shared_ptr<MapLayer> below) : MapLayer(seed), below(below) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		using namespace VanillaBiomeIds;
		int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
		auto vals = below->generateValues(gx, gz, gsx, gsz);
		std::vector<int> res(sizeX * sizeZ);

		auto isOcean = [](int b) { return b == OCEAN || b == DEEP_OCEAN; };

		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int c = vals[(j + 1) + (i + 1) * gsx];
				if (!isOcean(c)) {
					int u = vals[(j + 1) + i * gsx];
					int d = vals[(j + 1) + (i + 2) * gsx];
					int l = vals[j + (i + 1) * gsx];
					int r = vals[(j + 2) + (i + 1) * gsx];
					if (isOcean(u) || isOcean(d) || isOcean(l) || isOcean(r)) {
						if (c == EXTREME_HILLS || c == EXTREME_HILLS_PLUS_TREES || c == EXTREME_HILLS_MUTATED || c == EXTREME_HILLS_PLUS_TREES_MUTATED) {
							c = STONE_BEACH;
						} else if (c == ICE_PLAINS || c == ICE_MOUNTAINS || c == ICE_PLAINS_SPIKES || c == COLD_TAIGA || c == COLD_TAIGA_HILLS || c == COLD_TAIGA_MUTATED) {
							c = COLD_BEACH;
						} else if (c == MUSHROOM_ISLAND) {
							c = MUSHROOM_ISLAND_SHORE;
						} else if (c == SWAMPLAND || c == MESA || c == MESA_PLATEAU || c == MESA_PLATEAU_STONE || c == MESA_BRYCE || c == MESA_PLATEAU_MUTATED || c == MESA_PLATEAU_STONE_MUTATED) {
							// Retain biome
						} else {
							c = BEACH;
						}
					}
				}
				res[j + i * sizeX] = c;
			}
		}
		return res;
	}
};

class RiverMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	std::shared_ptr<MapLayer> merge;

	RiverMapLayer(int64_t seed, std::shared_ptr<MapLayer> below, std::shared_ptr<MapLayer> merge = nullptr)
	:	MapLayer(seed), below(below), merge(merge) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		using namespace VanillaBiomeIds;
		if (!merge) {
			int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
			auto vals = below->generateValues(gx, gz, gsx, gsz);
			std::vector<int> res(sizeX * sizeZ);
			for (int i = 0; i < sizeZ; ++i) {
				for (int j = 0; j < sizeX; ++j) {
					int c = vals[(j + 1) + (i + 1) * gsx] & 1;
					int u = vals[(j + 1) + i * gsx] & 1;
					int d = vals[(j + 1) + (i + 2) * gsx] & 1;
					int l = vals[j + (i + 1) * gsx] & 1;
					int r = vals[(j + 2) + (i + 1) * gsx] & 1;
					res[j + i * sizeX] = (c != u || c != d || c != l || c != r) ? 1 : 0;
				}
			}
			return res;
		}

		auto riverVals = below->generateValues(x, z, sizeX, sizeZ);
		auto biomeVals = merge->generateValues(x, z, sizeX, sizeZ);
		std::vector<int> res(sizeX * sizeZ);

		for (int i = 0; i < sizeX * sizeZ; ++i) {
			int b = biomeVals[i];
			if (b == OCEAN || b == DEEP_OCEAN) {
				res[i] = b;
			} else if (riverVals[i] == 1) {
				if (b == ICE_PLAINS) res[i] = FROZEN_RIVER;
				else if (b == MUSHROOM_ISLAND || b == MUSHROOM_ISLAND_SHORE) res[i] = MUSHROOM_ISLAND_SHORE;
				else res[i] = RIVER;
			} else {
				res[i] = b;
			}
		}
		return res;
	}
};

class SmoothMapLayer : public MapLayer {
public:
	std::shared_ptr<MapLayer> below;
	SmoothMapLayer(int64_t seed, std::shared_ptr<MapLayer> below) : MapLayer(seed), below(below) {}

	std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) override {
		int gx = x - 1, gz = z - 1, gsx = sizeX + 2, gsz = sizeZ + 2;
		auto vals = below->generateValues(gx, gz, gsx, gsz);
		std::vector<int> res(sizeX * sizeZ);

		for (int i = 0; i < sizeZ; ++i) {
			for (int j = 0; j < sizeX; ++j) {
				int c = vals[(j + 1) + (i + 1) * gsx];
				int u = vals[(j + 1) + i * gsx];
				int d = vals[(j + 1) + (i + 2) * gsx];
				int l = vals[j + (i + 1) * gsx];
				int r = vals[(j + 2) + (i + 1) * gsx];

				if (l == r && u == d) {
					setCoordsSeed(x + j, z + i);
					c = (nextInt(2) == 0) ? l : u;
				} else if (l == r) {
					c = l;
				} else if (u == d) {
					c = u;
				}
				res[j + i * sizeX] = c;
			}
		}
		return res;
	}
};

MapLayer::MapLayerPair MapLayer::initialize(int64_t seed)
{
	std::shared_ptr<MapLayer> layer = std::make_shared<NoiseMapLayer>(seed);
	layer = std::make_shared<WhittakerMapLayer>(seed + 1, layer, WhittakerMapLayer::WARM_WET);
	layer = std::make_shared<WhittakerMapLayer>(seed + 1, layer, WhittakerMapLayer::COLD_DRY);
	layer = std::make_shared<WhittakerMapLayer>(seed + 2, layer, WhittakerMapLayer::LARGER_BIOMES);

	for (int i = 0; i < 2; ++i) {
		layer = std::make_shared<ZoomMapLayer>(seed + 100 + i, layer, ZoomMapLayer::BLURRY);
	}

	for (int i = 0; i < 2; ++i) {
		layer = std::make_shared<ErosionMapLayer>(seed + 3 + i, layer);
	}

	layer = std::make_shared<DeepOceanMapLayer>(seed + 4, layer);

	std::shared_ptr<MapLayer> layerMountains = std::make_shared<BiomeVariationMapLayer>(seed + 200, layer);
	for (int i = 0; i < 2; ++i) {
		layerMountains = std::make_shared<ZoomMapLayer>(seed + 200 + i, layerMountains);
	}

	layer = std::make_shared<BiomeMapLayer>(seed + 5, layer);
	for (int i = 0; i < 2; ++i) {
		layer = std::make_shared<ZoomMapLayer>(seed + 200 + i, layer);
	}

	layer = std::make_shared<BiomeEdgeMapLayer>(seed + 200, layer);
	layer = std::make_shared<BiomeVariationMapLayer>(seed + 200, layer, layerMountains);
	layer = std::make_shared<RarePlainsMapLayer>(seed + 201, layer);
	layer = std::make_shared<ZoomMapLayer>(seed + 300, layer);
	layer = std::make_shared<ErosionMapLayer>(seed + 6, layer);
	layer = std::make_shared<ZoomMapLayer>(seed + 400, layer);
	layer = std::make_shared<BiomeThinEdgeMapLayer>(seed + 400, layer);
	layer = std::make_shared<ShoreMapLayer>(seed + 7, layer);
	for (int i = 0; i < 2; ++i) {
		layer = std::make_shared<ZoomMapLayer>(seed + 500 + i, layer);
	}

	std::shared_ptr<MapLayer> layerRiver = layerMountains;
	layerRiver = std::make_shared<ZoomMapLayer>(seed + 300, layerRiver);
	layerRiver = std::make_shared<ZoomMapLayer>(seed + 400, layerRiver);
	for (int i = 0; i < 2; ++i) {
		layerRiver = std::make_shared<ZoomMapLayer>(seed + 500 + i, layerRiver);
	}
	layerRiver = std::make_shared<RiverMapLayer>(seed + 10, layerRiver);
	layer = std::make_shared<RiverMapLayer>(seed + 1000, layerRiver, layer);

	std::shared_ptr<MapLayer> lowRes = layer;
	for (int i = 0; i < 2; ++i) {
		layer = std::make_shared<ZoomMapLayer>(seed + 2000 + i, layer);
	}

	layer = std::make_shared<SmoothMapLayer>(seed + 1001, layer);

	MapLayerPair pair;
	pair.highRes = layer;
	pair.lowRes = lowRes;
	return pair;
}
