#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE_NETHERSTALKTILE_H
#define NET_MINECRAFT_WORLD_LEVEL_TILE_NETHERSTALKTILE_H

#include "Bush.h"

class NetherStalkTile : public Bush {
public:
	NetherStalkTile(int id);

	bool mayPlaceOn(int tile) override;
	bool canSurvive(Level* level, int x, int y, int z) override;
	void tick(Level* level, int x, int y, int z, Random* random) override;
	int getTexture(int face, int data) override;
	int getTexture(LevelSource* level, int x, int y, int z, int face) override;
	int getRenderShape() override;
	int getResource(int data, Random* random) override;
	int getResourceCount(Random* random) override;
	void spawnResources(Level* level, int x, int y, int z, int data, float odds) override;
};

#endif
