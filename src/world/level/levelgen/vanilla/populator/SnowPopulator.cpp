#include "SnowPopulator.h"
#include "../biome/VanillaBiomeIds.h"
#include "../../../tile/Tile.h"

SnowPopulator::SnowPopulator() {}

void SnowPopulator::populate(Level* level, int chunkX, int chunkZ, Random& rand, const std::vector<int>& biomes)
{
	int bx = chunkX << 4;
	int bz = chunkZ << 4;

	for (int x = 0; x < 16; ++x) {
		for (int z = 0; z < 16; ++z) {
			int wx = bx + x;
			int wz = bz + z;
			int biomeId = biomes[x + z * 16];

			bool isCold = (biomeId == VanillaBiomeIds::ICE_PLAINS ||
			               biomeId == VanillaBiomeIds::ICE_PLAINS_SPIKES ||
			               biomeId == VanillaBiomeIds::ICE_MOUNTAINS ||
			               biomeId == VanillaBiomeIds::COLD_TAIGA ||
			               biomeId == VanillaBiomeIds::COLD_TAIGA_HILLS ||
			               biomeId == VanillaBiomeIds::COLD_TAIGA_MUTATED ||
			               biomeId == VanillaBiomeIds::COLD_BEACH ||
			               biomeId == VanillaBiomeIds::FROZEN_OCEAN ||
			               biomeId == VanillaBiomeIds::FROZEN_RIVER);

			if (isCold) {
				int topY = level->getHeightmap(wx, wz);
				if (topY > 0) {
					int topTile = level->getTile(wx, topY - 1, wz);
					if (topTile == Tile::calmWater->id || topTile == Tile::water->id) {
						level->setTile(wx, topY - 1, wz, Tile::ice->id);
					} else if (Tile::topSnow && level->getTile(wx, topY, wz) == 0) {
						level->setTile(wx, topY, wz, Tile::topSnow->id);
					}
				}
			}
		}
	}
}
