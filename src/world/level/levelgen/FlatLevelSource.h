#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatLevelSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatLevelSource_H__

#include "../chunk/ChunkSource.h"
#include "FlatGeneratorInfo.h"
#include "../../../util/Random.h"

#include <map>
#include <string>

class Level;
class LevelChunk;
class Biome;

class FlatLevelSource : public ChunkSource
{
public:
	FlatLevelSource(Level* level, long seed, bool generateStructures);
	~FlatLevelSource();

	bool hasChunk(int x, int y) override;
	LevelChunk* create(int x, int z) override;
	LevelChunk* getChunk(int xOffs, int zOffs) override;
	LevelChunk* getChunkDontCreate(int x, int z);
	void postProcess(ChunkSource* parent, int xt, int zt) override;
	bool tick() override;
	bool shouldSave() override;
	std::string gatherStats() override;
	Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z) override;

private:
	void prepareHeights(uint16_t* blocks);

	Level* level;
	Random random;
	bool generateStructures;
	FlatGeneratorInfo generatorInfo;

	std::map<int, LevelChunk*> chunkMap;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatLevelSource_H__*/
