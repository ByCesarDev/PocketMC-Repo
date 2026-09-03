#include "GroundGenerator.h"
#include "../../../LevelConstants.h"

void GroundGenerator::generateTerrainColumn(LevelChunk* chunk, Random& rand, int x, int z, int biomeId, double surfaceNoise)
{
	int seaLevel = 64;
	int topMat = topTile;
	int topDat = topData;
	int groundMat = groundTile;
	int groundDat = groundData;

	int surfaceHeight = (int)(surfaceNoise / 3.0 + 3.0 + rand.nextFloat() * 0.25);
	if (surfaceHeight < 1) surfaceHeight = 1;

	int deep = -1;

	for (int y = LEVEL_HEIGHT - 1; y >= 0; --y) {
		if (y <= rand.nextInt(5)) {
			chunk->setTile(x, y, z, Tile::unbreakable->id); // Bedrock
		} else {
			int matId = chunk->getTile(x, y, z);
			if (matId == 0) { // Air
				deep = -1;
			} else if (matId == Tile::rock->id) { // Stone
				if (deep == -1) {
					if (surfaceHeight <= 0) {
						topMat = 0;
						groundMat = Tile::rock->id;
					} else if (y >= seaLevel - 4 && y <= seaLevel + 1) {
						topMat = topTile;
						topDat = topData;
						groundMat = groundTile;
						groundDat = groundData;
					}

					if (y < seaLevel && topMat == 0) {
						topMat = Tile::calmWater->id;
						topDat = 0;
					}

					deep = surfaceHeight;
					if (y >= seaLevel - 1) {
						chunk->setTileAndData(x, y, z, topMat, topDat);
					} else {
						chunk->setTileAndData(x, y, z, groundMat, groundDat);
					}
				} else if (deep > 0) {
					--deep;
					chunk->setTileAndData(x, y, z, groundMat, groundDat);
					if (deep == 0 && groundMat == Tile::sand->id) { // Sand -> Sandstone below
						deep = rand.nextInt(4) + 1;
						groundMat = Tile::sandStone->id;
						groundDat = 0;
					}
				}
			}
		}
	}
}
