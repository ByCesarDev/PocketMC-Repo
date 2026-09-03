#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_BIOMEGRID__MapLayer_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_BIOMEGRID__MapLayer_H__

#include <vector>
#include <memory>
#include "../../../../../util/Random.h"
#include "../biome/VanillaBiomeIds.h"

class MapLayer
{
public:
	int64_t seed;
	Random random;

	MapLayer(int64_t seed);
	virtual ~MapLayer() {}

	void setCoordsSeed(int x, int z);
	int nextInt(int max);

	virtual std::vector<int> generateValues(int x, int z, int sizeX, int sizeZ) = 0;

	struct MapLayerPair {
		std::shared_ptr<MapLayer> highRes;
		std::shared_ptr<MapLayer> lowRes;
	};

	static MapLayerPair initialize(int64_t seed);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_BIOMEGRID__MapLayer_H__*/
