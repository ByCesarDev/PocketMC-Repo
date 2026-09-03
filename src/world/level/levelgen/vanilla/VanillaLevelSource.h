#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA__VanillaLevelSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA__VanillaLevelSource_H__

#include "../../chunk/ChunkSource.h"
#include "../../ChunkPos.h"
#include "../../../../util/Random.h"
#include "noise/PerlinOctaveGenerator.h"
#include "noise/SimplexOctaveGenerator.h"
#include "biomegrid/MapLayer.h"
#include "ground/GroundGenerator.h"
#include "ground/MesaGroundGenerator.h"
#include "populator/VanillaPopulator.h"
#include "populator/SnowPopulator.h"

#include <map>
#include <memory>

class Level;
class LevelChunk;

class VanillaLevelSource : public ChunkSource
{
public:
	VanillaLevelSource(Level* level, int64_t seed);
	~VanillaLevelSource();

	bool hasChunk(int x, int y) override;
	LevelChunk* create(int x, int z) override;
	LevelChunk* getChunk(int xOffs, int zOffs) override;
	void postProcess(ChunkSource* parent, int xt, int zt) override;
	bool tick() override;
	bool shouldSave() override;
	std::string gatherStats() override;
	Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z) override;
	bool findSpawnPosition(int& spawnX, int& spawnZ) override;

private:
	void generateRawTerrain(LevelChunk* chunk, int chunkX, int chunkZ, const std::vector<int>& lowResBiomes);
	std::vector<double> generateTerrainDensity(int chunkX, int chunkZ, const std::vector<int>& lowResBiomes);
	void applyBiomeGround(LevelChunk* chunk, int chunkX, int chunkZ, const std::vector<int>& biomes);

	Level* level;
	int64_t seed;
	Random random;

	MapLayer::MapLayerPair biomeLayers;

	VanillaGenerator::PerlinOctaveGenerator heightNoise;
	VanillaGenerator::PerlinOctaveGenerator roughnessNoise;
	VanillaGenerator::PerlinOctaveGenerator roughnessNoise2;
	VanillaGenerator::PerlinOctaveGenerator detailNoise;
	VanillaGenerator::SimplexOctaveGenerator surfaceNoise;

	GroundGenerator defaultGround;
	GroundGenerator sandyGround;
	GroundGenerator gravelGround;
	GroundGenerator mycelGround;
	GroundGenerator dirtPatchGround;
	MesaGroundGenerator mesaGround;
	MesaGroundGenerator mesaBryceGround;

	VanillaPopulator populator;
	SnowPopulator snowPopulator;

	double elevationWeight[25];

	std::map<int, LevelChunk*> chunkMap;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA__VanillaLevelSource_H__*/
