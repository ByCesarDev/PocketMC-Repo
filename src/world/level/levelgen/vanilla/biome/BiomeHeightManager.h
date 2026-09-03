#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_BIOME__BiomeHeightManager_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_BIOME__BiomeHeightManager_H__

struct BiomeHeight {
	double height;
	double scale;

	BiomeHeight(double height = 0.1, double scale = 0.2)
	:	height(height), scale(scale) {}
};

class BiomeHeightManager {
public:
	static void init();
	static BiomeHeight get(int biomeId);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_BIOME__BiomeHeightManager_H__*/
