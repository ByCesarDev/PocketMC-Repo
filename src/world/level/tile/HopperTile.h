#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__HopperTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__HopperTile_H__

#include "Tile.h"
#include "../material/Material.h"
#include "../../Facing.h"

class HopperTile : public Tile
{
	typedef Tile super;
public:
	HopperTile(int id, int tex)
		: super(id, tex, Material::metal)
	{
	}

	int getRenderShape() override {
		return Tile::SHAPE_HOPPER;
	}

	bool isSolidRender() override {
		return false;
	}

	bool isCubeShaped() override {
		return false;
	}

	int getPlacedOnFaceDataValue(Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue) override {
		if (face >= 0 && face < 6) {
			int facing = Facing::OPPOSITE_FACING[face];
			if (facing == 1) facing = 0; // cannot point up, default down
			return facing;
		}
		return 0;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__HopperTile_H__*/
