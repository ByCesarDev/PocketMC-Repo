#include "BiomeHeightManager.h"
#include "VanillaBiomeIds.h"
#include <map>

static std::map<int, BiomeHeight> s_heights;
static bool s_initialized = false;

void BiomeHeightManager::init()
{
	if (s_initialized) return;
	s_initialized = true;

	auto reg = [](const BiomeHeight& h, std::initializer_list<int> biomes) {
		for (int b : biomes) {
			s_heights[b] = h;
		}
	};

	using namespace VanillaBiomeIds;

	reg(BiomeHeight(-1.0, 0.1), {OCEAN, FROZEN_OCEAN});
	reg(BiomeHeight(-1.8, 0.1), {DEEP_OCEAN});
	reg(BiomeHeight(-0.5, 0.0), {RIVER, FROZEN_RIVER});
	reg(BiomeHeight(0.0, 0.025), {BEACH, COLD_BEACH, MUSHROOM_ISLAND_SHORE});
	reg(BiomeHeight(0.1, 0.8), {STONE_BEACH});
	reg(BiomeHeight(0.125, 0.05), {DESERT, ICE_PLAINS, SAVANNA});

	reg(BiomeHeight(1.0, 0.5), {
		EXTREME_HILLS,
		EXTREME_HILLS_PLUS_TREES,
		EXTREME_HILLS_MUTATED,
		EXTREME_HILLS_PLUS_TREES_MUTATED
	});

	reg(BiomeHeight(0.2, 0.2), {TAIGA, COLD_TAIGA, MEGA_TAIGA});
	reg(BiomeHeight(-0.2, 0.1), {SWAMPLAND});
	reg(BiomeHeight(0.2, 0.3), {MUSHROOM_ISLAND});

	reg(BiomeHeight(0.45, 0.3), {
		ICE_MOUNTAINS,
		DESERT_HILLS,
		FOREST_HILLS,
		TAIGA_HILLS,
		EXTREME_HILLS_EDGE,
		JUNGLE_HILLS,
		BIRCH_FOREST_HILLS,
		COLD_TAIGA_HILLS,
		MEGA_TAIGA_HILLS,
		MESA_PLATEAU_STONE_MUTATED,
		MESA_PLATEAU_MUTATED
	});

	reg(BiomeHeight(1.5, 0.025), {SAVANNA_PLATEAU, MESA_PLATEAU_STONE, MESA_PLATEAU});
	reg(BiomeHeight(0.275, 0.25), {DESERT_MUTATED});
	reg(BiomeHeight(0.525, 0.55), {ICE_PLAINS_SPIKES});
	reg(BiomeHeight(0.55, 0.5), {BIRCH_FOREST_HILLS_MUTATED});
	reg(BiomeHeight(-0.1, 0.3), {SWAMPLAND_MUTATED});
	reg(BiomeHeight(0.2, 0.4), {JUNGLE_MUTATED, JUNGLE_EDGE_MUTATED, BIRCH_FOREST_MUTATED, ROOFED_FOREST_MUTATED});
	reg(BiomeHeight(0.3, 0.4), {TAIGA_MUTATED, COLD_TAIGA_MUTATED, REDWOOD_TAIGA_MUTATED, REDWOOD_TAIGA_HILLS_MUTATED});
	reg(BiomeHeight(0.1, 0.4), {FLOWER_FOREST});
	reg(BiomeHeight(0.4125, 1.325), {SAVANNA_MUTATED});
	reg(BiomeHeight(1.1, 1.3125), {SAVANNA_PLATEAU_MUTATED});
}

BiomeHeight BiomeHeightManager::get(int biomeId)
{
	if (!s_initialized) init();
	auto it = s_heights.find(biomeId);
	if (it != s_heights.end()) {
		return it->second;
	}
	return BiomeHeight(0.1, 0.2); // Default
}
