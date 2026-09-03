#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_GROUND__MesaGroundGenerator_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_GROUND__MesaGroundGenerator_H__

#include "GroundGenerator.h"
#include "../noise/SimplexOctaveGenerator.h"

class MesaGroundGenerator : public GroundGenerator
{
public:
	enum Type { NORMAL = 0, BRYCE = 1, FOREST = 2 };

	Type type;
	int colorLayer[64];
	VanillaGenerator::SimplexOctaveGenerator colorNoise;
	int64_t seed;
	bool initialized;

	MesaGroundGenerator(Type type = NORMAL);

	void initialize(int64_t worldSeed);
	void initializeColorLayers(Random& rand);
	void generateTerrainColumn(LevelChunk* chunk, Random& rand, int x, int z, int biomeId, double surfaceNoise) override;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_GROUND__MesaGroundGenerator_H__*/
