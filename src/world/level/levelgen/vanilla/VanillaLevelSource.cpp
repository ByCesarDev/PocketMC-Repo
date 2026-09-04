#include "VanillaLevelSource.h"
#include "../../Level.h"
#include "../../chunk/LevelChunk.h"
#include "../../tile/Tile.h"
#include "biome/BiomeHeightManager.h"
#include "biome/VanillaBiomeIds.h"

#include <cmath>
#include <algorithm>
#include <cstring>

using namespace VanillaGenerator;

static inline int densityHash(int i, int j, int k) {
	return (i * 5 + j) * 33 + k;
}

VanillaLevelSource::VanillaLevelSource(Level* level, int64_t seed)
:	level(level),
	seed(seed),
	random(seed),
	sandyGround(Tile::sand->id, 0, Tile::sand->id, 0),
	gravelGround(Tile::gravel->id, 0, Tile::gravel->id, 0),
	mycelGround(Tile::mycelium ? Tile::mycelium->id : Tile::grass->id, 0, Tile::dirt->id, 0),
	dirtPatchGround(Tile::podzol ? Tile::podzol->id : Tile::dirt->id, 0, Tile::dirt->id, 0),
	mesaGround(MesaGroundGenerator::NORMAL),
	mesaBryceGround(MesaGroundGenerator::BRYCE)
{
	BiomeHeightManager::init();
	biomeLayers = MapLayer::initialize(seed);

	Random rand(seed);
	heightNoise = PerlinOctaveGenerator(rand, 16, 5, 1, 5);
	heightNoise.scaleX = 200.0;
	heightNoise.scaleZ = 200.0;

	roughnessNoise = PerlinOctaveGenerator(rand, 16, 5, 33, 5);
	roughnessNoise.scaleX = 684.412;
	roughnessNoise.scaleY = 684.412;
	roughnessNoise.scaleZ = 684.412;

	roughnessNoise2 = PerlinOctaveGenerator(rand, 16, 5, 33, 5);
	roughnessNoise2.scaleX = 684.412;
	roughnessNoise2.scaleY = 684.412;
	roughnessNoise2.scaleZ = 684.412;

	detailNoise = PerlinOctaveGenerator(rand, 8, 5, 33, 5);
	detailNoise.scaleX = 684.412 / 80.0;
	detailNoise.scaleY = 684.412 / 160.0;
	detailNoise.scaleZ = 684.412 / 80.0;

	surfaceNoise = SimplexOctaveGenerator(rand, 4, 16, 1, 16);
	surfaceNoise.setScale(0.0625);

	for (int x = 0; x < 5; ++x) {
		for (int z = 0; z < 5; ++z) {
			double sx = x - 2;
			double sz = z - 2;
			elevationWeight[x + z * 5] = 10.0 / std::sqrt(sx * sx + sz * sz + 0.2);
		}
	}
}

VanillaLevelSource::~VanillaLevelSource()
{
	chunkMap.clear();
}

bool VanillaLevelSource::hasChunk(int x, int z)
{
	int hashedPos = ChunkPos::hashCode(x, z);
	return chunkMap.find(hashedPos) != chunkMap.end();
}

LevelChunk* VanillaLevelSource::create(int x, int z)
{
	return getChunk(x, z);
}

LevelChunk* VanillaLevelSource::getChunk(int chunkX, int chunkZ)
{
	int hashedPos = ChunkPos::hashCode(chunkX, chunkZ);
	auto it = chunkMap.find(hashedPos);
	if (it != chunkMap.end()) return it->second;

	uint16_t* blocks = new uint16_t[LevelChunk::ChunkBlockCount];
	std::memset(blocks, 0, LevelChunk::ChunkBlockCount * sizeof(uint16_t));

	LevelChunk* chunk = new LevelChunk(level, blocks, chunkX, chunkZ);
	chunkMap[hashedPos] = chunk;

	// 1. Biomes (10x10 at 4x scale for height sampling, and 16x16 at 1x scale for chunk)
	int lowX = (chunkX << 2) - 2;
	int lowZ = (chunkZ << 2) - 2;
	auto lowResBiomes = biomeLayers.lowRes->generateValues(lowX, lowZ, 10, 10);
	auto highResBiomes = biomeLayers.highRes->generateValues(chunkX << 4, chunkZ << 4, 16, 16);

	// 2. Raw Terrain (3D Noise + Trilinear Interpolation)
	generateRawTerrain(chunk, chunkX, chunkZ, lowResBiomes);

	// 3. Biome Ground and Surface layers
	applyBiomeGround(chunk, chunkX, chunkZ, highResBiomes);

	chunk->recalcHeightmap();
	return chunk;
}

std::vector<double> VanillaLevelSource::generateTerrainDensity(int chunkX, int chunkZ, const std::vector<int>& lowResBiomes)
{
	int x = chunkX << 2;
	int z = chunkZ << 2;

	auto hNoise = heightNoise.getFractalBrownianMotion(x, 0, z, 0.5, 2.0);
	auto rNoise = roughnessNoise.getFractalBrownianMotion(x, 0, z, 0.5, 2.0);
	auto rNoise2 = roughnessNoise2.getFractalBrownianMotion(x, 0, z, 0.5, 2.0);
	auto dNoise = detailNoise.getFractalBrownianMotion(x, 0, z, 0.5, 2.0);

	std::vector<double> density(5 * 5 * 33, 0.0);

	int index = 0;
	int indexHeight = 0;

	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			double avgHeightScale = 0.0;
			double avgHeightBase = 0.0;
			double totalWeight = 0.0;

			int centerBiome = lowResBiomes[(i + 2) + (j + 2) * 10];
			BiomeHeight centerH = BiomeHeightManager::get(centerBiome);

			for (int m = 0; m < 5; ++m) {
				for (int n = 0; n < 5; ++n) {
					int nearBiome = lowResBiomes[(i + m) + (j + n) * 10];
					BiomeHeight nearH = BiomeHeightManager::get(nearBiome);
					double hBase = nearH.height;
					double hScale = nearH.scale;

					double w = elevationWeight[m + n * 5] / (hBase + 2.0);
					if (nearH.height > centerH.height) {
						w *= 0.5;
					}

					avgHeightScale += hScale * w;
					avgHeightBase += hBase * w;
					totalWeight += w;
				}
			}

			avgHeightScale /= totalWeight;
			avgHeightBase /= totalWeight;
			avgHeightScale = avgHeightScale * 0.9 + 0.1;
			avgHeightBase = (avgHeightBase * 4.0 - 1.0) / 8.0;

			double noiseH = hNoise[indexHeight++] / 8000.0;
			if (noiseH < 0.0) noiseH = -noiseH * 0.3;
			noiseH = noiseH * 3.0 - 2.0;

			if (noiseH < 0.0) {
				noiseH = std::max(noiseH * 0.5, -1.0) / 1.4 * 0.5;
			} else {
				noiseH = std::min(noiseH, 1.0) / 8.0;
			}

			noiseH = (noiseH * 0.2 + avgHeightBase) * 8.5 / 8.0 * 4.0 + 8.5;

			for (int k = 0; k < 33; ++k) {
				double nh = ((double)k - noiseH) * 12.0 * 128.0 / 256.0 / avgHeightScale;
				if (nh < 0.0) nh *= 4.0;

				double nr = rNoise[index] / 512.0;
				double nr2 = rNoise2[index] / 512.0;
				double nd = (dNoise[index] / 10.0 + 1.0) / 2.0;

				double dens = (nd < 0.0) ? nr : ((nd > 1.0) ? nr2 : nr + (nr2 - nr) * nd);
				dens -= nh;
				++index;

				if (k > 29) {
					double lowering = (double)(k - 29) / 3.0;
					dens = dens * (1.0 - lowering) + -10.0 * lowering;
				}

				density[densityHash(i, j, k)] = dens;
			}
		}
	}

	return density;
}

void VanillaLevelSource::generateRawTerrain(LevelChunk* chunk, int chunkX, int chunkZ, const std::vector<int>& lowResBiomes)
{
	auto density = generateTerrainDensity(chunkX, chunkZ, lowResBiomes);
	int seaLevel = 64;

	uint16_t* blocks = chunk->getBlockData();
	int stoneId = Tile::rock->id;
	int waterId = Tile::calmWater->id;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			for (int k = 0; k < 32; ++k) {
				double d1 = density[densityHash(i, j, k)];
				double d2 = density[densityHash(i + 1, j, k)];
				double d3 = density[densityHash(i, j + 1, k)];
				double d4 = density[densityHash(i + 1, j + 1, k)];

				double d5 = (density[densityHash(i, j, k + 1)] - d1) / 8.0;
				double d6 = (density[densityHash(i + 1, j, k + 1)] - d2) / 8.0;
				double d7 = (density[densityHash(i, j + 1, k + 1)] - d3) / 8.0;
				double d8 = (density[densityHash(i + 1, j + 1, k + 1)] - d4) / 8.0;

				for (int l = 0; l < 8; ++l) {
					double d9 = d1;
					double d10 = d3;
					int yPos = l + (k << 3);

					for (int m = 0; m < 4; ++m) {
						double dens = d9;
						for (int n = 0; n < 4; ++n) {
							int bx = m + (i << 2);
							int bz = n + (j << 2);

							if (dens > 0.0) {
								blocks[bx << 12 | bz << 8 | yPos] = (uint16_t)stoneId;
							} else if (yPos < seaLevel) {
								blocks[bx << 12 | bz << 8 | yPos] = (uint16_t)waterId;
							}

							dens += (d10 - d9) / 4.0;
						}
						d9 += (d2 - d1) / 4.0;
						d10 += (d4 - d3) / 4.0;
					}

					d1 += d5;
					d3 += d7;
					d2 += d6;
					d4 += d8;
				}
			}
		}
	}
}

void VanillaLevelSource::applyBiomeGround(LevelChunk* chunk, int chunkX, int chunkZ, const std::vector<int>& biomes)
{
	int cx = chunkX << 4;
	int cz = chunkZ << 4;
	auto sNoise = surfaceNoise.getFractalBrownianMotion(cx, 0.0, cz, 0.5, 0.5);

	for (int x = 0; x < 16; ++x) {
		for (int z = 0; z < 16; ++z) {
			int biomeId = biomes[x + z * 16];
			double noiseVal = sNoise[x + z * 16];

			using namespace VanillaBiomeIds;
			if (biomeId == DESERT || biomeId == DESERT_HILLS || biomeId == DESERT_MUTATED || biomeId == BEACH || biomeId == COLD_BEACH) {
				sandyGround.generateTerrainColumn(chunk, random, x, z, biomeId, noiseVal);
			} else if (biomeId == STONE_BEACH || biomeId == EXTREME_HILLS_MUTATED) {
				gravelGround.generateTerrainColumn(chunk, random, x, z, biomeId, noiseVal);
			} else if (biomeId == MUSHROOM_ISLAND || biomeId == MUSHROOM_ISLAND_SHORE) {
				mycelGround.generateTerrainColumn(chunk, random, x, z, biomeId, noiseVal);
			} else if (biomeId == MEGA_TAIGA || biomeId == MEGA_TAIGA_HILLS || biomeId == REDWOOD_TAIGA_MUTATED || biomeId == REDWOOD_TAIGA_HILLS_MUTATED) {
				dirtPatchGround.generateTerrainColumn(chunk, random, x, z, biomeId, noiseVal);
			} else if (biomeId == MESA || biomeId == MESA_PLATEAU || biomeId == MESA_PLATEAU_STONE) {
				mesaGround.generateTerrainColumn(chunk, random, x, z, biomeId, noiseVal);
			} else if (biomeId == MESA_BRYCE) {
				mesaBryceGround.generateTerrainColumn(chunk, random, x, z, biomeId, noiseVal);
			} else {
				defaultGround.generateTerrainColumn(chunk, random, x, z, biomeId, noiseVal);
			}
		}
	}
}

void VanillaLevelSource::postProcess(ChunkSource* parent, int chunkX, int chunkZ)
{
	auto highResBiomes = biomeLayers.highRes->generateValues(chunkX << 4, chunkZ << 4, 16, 16);
	int centerBiome = highResBiomes[8 + 8 * 16];

	populator.populate(level, chunkX, chunkZ, random, centerBiome);
	snowPopulator.populate(level, chunkX, chunkZ, random, highResBiomes);
}

bool VanillaLevelSource::tick()
{
	return false;
}

bool VanillaLevelSource::shouldSave()
{
	return true;
}

std::string VanillaLevelSource::gatherStats()
{
	return "VanillaLevelSource: 1:1 Java 1.7+ Generator";
}

Biome::MobList VanillaLevelSource::getMobsAt(const MobCategory& mobCategory, int x, int y, int z)
{
	return Biome::MobList();
}

bool VanillaLevelSource::findSpawnPosition(int& spawnX, int& spawnZ)
{
	int startX = 0;
	int startZ = 0;
	int range = 256;

	using namespace VanillaBiomeIds;
	std::vector<int> allowedBiomes = {
		PLAINS,
		FOREST,
		TAIGA,
		TAIGA_HILLS,
		FOREST_HILLS,
		JUNGLE,
		JUNGLE_HILLS,
		SAVANNA
	};

	int minX = (startX - range) >> 2;
	int minZ = (startZ - range) >> 2;
	int maxX = (startX + range) >> 2;
	int maxZ = (startZ + range) >> 2;
	int sizeX = maxX - minX + 1;
	int sizeZ = maxZ - minZ + 1;

	auto biomes = biomeLayers.lowRes->generateValues(minX, minZ, sizeX, sizeZ);

	int foundCount = 0;
	int bestX = 0;
	int bestZ = 0;

	for (int i = 0; i < sizeX * sizeZ; ++i) {
		int bx = (minX + (i % sizeX)) << 2;
		int bz = (minZ + (i / sizeX)) << 2;
		int biomeId = biomes[i];

		for (int allowed : allowedBiomes) {
			if (biomeId == allowed) {
				if (foundCount == 0 || random.nextInt(foundCount + 1) == 0) {
					bestX = bx;
					bestZ = bz;
				}
				foundCount++;
				break;
			}
		}
	}

	if (foundCount > 0) {
		spawnX = bestX;
		spawnZ = bestZ;
		return true;
	}

	// Fallback wider search (512 radius)
	range = 512;
	minX = (startX - range) >> 2;
	minZ = (startZ - range) >> 2;
	maxX = (startX + range) >> 2;
	maxZ = (startZ + range) >> 2;
	sizeX = maxX - minX + 1;
	sizeZ = maxZ - minZ + 1;

	biomes = biomeLayers.lowRes->generateValues(minX, minZ, sizeX, sizeZ);
	for (int i = 0; i < sizeX * sizeZ; ++i) {
		int bx = (minX + (i % sizeX)) << 2;
		int bz = (minZ + (i / sizeX)) << 2;
		int biomeId = biomes[i];

		for (int allowed : allowedBiomes) {
			if (biomeId == allowed) {
				if (foundCount == 0 || random.nextInt(foundCount + 1) == 0) {
					bestX = bx;
					bestZ = bz;
				}
				foundCount++;
				break;
			}
		}
	}

	if (foundCount > 0) {
		spawnX = bestX;
		spawnZ = bestZ;
		return true;
	}

	spawnX = 0;
	spawnZ = 0;
	return false;
}
