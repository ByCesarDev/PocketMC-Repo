#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_POPULATOR__VanillaPopulator_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_POPULATOR__VanillaPopulator_H__

#include "../../../Level.h"
#include "../../../../../util/Random.h"

class VanillaPopulator
{
public:
	VanillaPopulator();
	virtual ~VanillaPopulator() {}

	virtual void populate(Level* level, int chunkX, int chunkZ, Random& rand, int biomeId);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_POPULATOR__VanillaPopulator_H__*/
