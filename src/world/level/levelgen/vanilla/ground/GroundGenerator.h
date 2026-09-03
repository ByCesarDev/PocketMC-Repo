#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_GROUND__GroundGenerator_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_GROUND__GroundGenerator_H__

#include <memory>
#include "../../../../../util/Random.h"
#include "../../../tile/Tile.h"
#include "../../../chunk/LevelChunk.h"

class GroundGenerator
{
public:
	int topTile;
	int topData;
	int groundTile;
	int groundData;

	GroundGenerator(int topTile = 2, int topData = 0, int groundTile = 3, int groundData = 0) // Grass (2), Dirt (3)
	:	topTile(topTile), topData(topData), groundTile(groundTile), groundData(groundData) {}

	virtual ~GroundGenerator() {}

	virtual void generateTerrainColumn(LevelChunk* chunk, Random& rand, int x, int z, int biomeId, double surfaceNoise);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_GROUND__GroundGenerator_H__*/
