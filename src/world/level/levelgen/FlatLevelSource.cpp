#include "FlatLevelSource.h"
#include "FlatLayerInfo.h"
#include "../Level.h"
#include "../ChunkPos.h"
#include "../biome/Biome.h"
#include "../biome/BiomeSource.h"
#include "../chunk/LevelChunk.h"
#include "../tile/Tile.h"

FlatLevelSource::FlatLevelSource(Level* level, long seed, bool generateStructures)
:	level(level),
	random(seed),
	generateStructures(generateStructures)
{
	FlatGeneratorInfo::getDefault(generatorInfo);
}

FlatLevelSource::~FlatLevelSource()
{
}

void FlatLevelSource::prepareHeights(unsigned char* blocks)
{
	const std::vector<FlatLayerInfo*>& layers = generatorInfo.getLayers();
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			int offs = (x << 12) | (z << 8);
			for (std::vector<FlatLayerInfo*>::const_iterator it = layers.begin(); it != layers.end(); ++it) {
				const FlatLayerInfo* layer = *it;
				int y = layer->getStart();
				for (int h = 0; h < layer->getHeight(); h++) {
					if (y + h < Level::DEPTH)
						blocks[offs + y + h] = (unsigned char) layer->getId();
				}
			}
		}
	}
}

bool FlatLevelSource::hasChunk(int x, int y)
{
	return true;
}

LevelChunk* FlatLevelSource::create(int x, int z)
{
	return getChunk(x, z);
}

LevelChunk* FlatLevelSource::getChunk(int xOffs, int zOffs)
{
	int hashedPos = ChunkPos::hashCode(xOffs, zOffs);

	std::map<int, LevelChunk*>::iterator it = chunkMap.find(hashedPos);
	if (it != chunkMap.end())
		return it->second;

	unsigned char* blocks = new unsigned char[LevelChunk::ChunkBlockCount];
	memset(blocks, 0, LevelChunk::ChunkBlockCount);

	prepareHeights(blocks);

	LevelChunk* levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
	chunkMap.insert(std::make_pair(hashedPos, levelChunk));

	levelChunk->recalcHeightmap();

	return levelChunk;
}

LevelChunk* FlatLevelSource::getChunkDontCreate(int x, int z)
{
	int hashedPos = ChunkPos::hashCode(x, z);
	std::map<int, LevelChunk*>::iterator it = chunkMap.find(hashedPos);
	if (it != chunkMap.end())
		return it->second;

	static unsigned char* emptyBlocks = nullptr;
	static LevelChunk* dummyChunk = nullptr;
	if (!dummyChunk) {
		emptyBlocks = new unsigned char[LevelChunk::ChunkBlockCount];
		memset(emptyBlocks, 0, LevelChunk::ChunkBlockCount);
		dummyChunk = new LevelChunk(level, emptyBlocks, 0, 0);
	}
	return dummyChunk;
}

void FlatLevelSource::postProcess(ChunkSource* parent, int xt, int zt)
{
}

bool FlatLevelSource::tick()
{
	return false;
}

bool FlatLevelSource::shouldSave()
{
	return true;
}

std::string FlatLevelSource::gatherStats()
{
	return "FlatLevelSource";
}

Biome::MobList FlatLevelSource::getMobsAt(const MobCategory& mobCategory, int x, int y, int z)
{
	BiomeSource* biomeSource = level->getBiomeSource();
	if (biomeSource == NULL)
		return Biome::MobList();

	Biome* biome = biomeSource->getBiome(x, z);
	if (biome == NULL)
		return Biome::MobList();

	return biome->getMobs(mobCategory);
}
