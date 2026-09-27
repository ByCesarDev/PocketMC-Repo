#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__CauldronTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__CauldronTile_H__

#include "Tile.h"

class CauldronTile : public Tile
{
	typedef Tile super;
public:
	CauldronTile(int id, int tex, const Material* material)
		: super(id, tex, material)
	{
	}

	int getRenderShape() override {
		return Tile::SHAPE_CAULDRON;
	}

	bool isSolidRender() override {
		return false;
	}

	bool isCubeShaped() override {
		return false;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__CauldronTile_H__*/
