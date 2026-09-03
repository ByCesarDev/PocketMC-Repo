#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_POPULATOR__SnowPopulator_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_POPULATOR__SnowPopulator_H__

#include "../../../Level.h"
#include "../../../../../util/Random.h"
#include <vector>

class SnowPopulator
{
public:
	SnowPopulator();
	void populate(Level* level, int chunkX, int chunkZ, Random& rand, const std::vector<int>& biomes);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_POPULATOR__SnowPopulator_H__*/
