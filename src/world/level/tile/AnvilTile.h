#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__AnvilTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__AnvilTile_H__

#include "Tile.h"
#include "../material/Material.h"
#include "../../entity/Mob.h"
#include <cmath>

class AnvilTile : public Tile
{
	typedef Tile super;
public:
	int damageType; // 0 = undamaged, 1 = chipped, 2 = damaged

	AnvilTile(int id, int tex, int damageType = 0)
		: super(id, tex, Material::metal), damageType(damageType)
	{
	}

	int getRenderShape() override {
		return Tile::SHAPE_ANVIL;
	}

	bool isSolidRender() override {
		return false;
	}

	bool isCubeShaped() override {
		return false;
	}

	void setPlacedBy(Level* level, int x, int y, int z, Mob* entity) override {
		if (!entity || !level) return;
		int dir = ((int)std::floor(entity->yRot * 4.0f / 360.0f + 0.5f)) & 3;
		level->setData(x, y, z, dir);
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__AnvilTile_H__*/
