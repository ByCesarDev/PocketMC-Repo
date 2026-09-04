#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__SlabTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__SlabTile_H__

#include "Tile.h"
#include "../material/Material.h"
#include "../../Facing.h"
#include "../LevelSource.h"

class Level;

class SlabTile : public Tile
{
	typedef Tile super;
public:
	static const int TOP_SLOT_BIT = 8;

	SlabTile(int id, int tex, const Material* material, bool fullSize = false)
	:	super(id, tex, material),
		fullSize(fullSize)
	{
		if (!fullSize) {
			setShape(0, 0, 0, 1, 0.5f, 1);
		}
		setLightBlock(255);
	}

	SlabTile(int id, const Material* material, bool fullSize = false)
	:	super(id, material),
		fullSize(fullSize)
	{
		if (!fullSize) {
			setShape(0, 0, 0, 1, 0.5f, 1);
		}
		setLightBlock(255);
	}

	bool isSolidRender() override {
		return fullSize;
	}

	bool isCubeShaped() override {
		return fullSize;
	}

	int getRenderShape() override {
		return Tile::SHAPE_BLOCK;
	}

	void updateShape(LevelSource* level, int x, int y, int z) override {
		if (fullSize) {
			setShape(0, 0, 0, 1, 1, 1);
		} else {
			bool upper = level && ((level->getData(x, y, z) & TOP_SLOT_BIT) != 0);
			if (upper) {
				setShape(0, 0.5f, 0, 1, 1, 1);
			} else {
				setShape(0, 0, 0, 1, 0.5f, 1);
			}
		}
	}

	void updateDefaultShape() override {
		if (fullSize) {
			setShape(0, 0, 0, 1, 1, 1);
		} else {
			setShape(0, 0, 0, 1, 0.5f, 1);
		}
	}

	int getPlacedOnFaceDataValue(Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue) override {
		if (fullSize) return itemValue;
		if (face == Facing::DOWN || (face != Facing::UP && clickY > 0.5f)) {
			return itemValue | TOP_SLOT_BIT;
		}
		return itemValue;
	}

	bool shouldRenderFace(LevelSource* level, int x, int y, int z, int face) override {
		if (fullSize) return super::shouldRenderFace(level, x, y, z, face);
		if (face != Facing::UP && face != Facing::DOWN && !super::shouldRenderFace(level, x, y, z, face)) {
			return false;
		}
		int tx = x;
		int ty = y;
		int tz = z;
		if (face == Facing::DOWN) ty++;
		if (face == Facing::UP) ty--;
		if (face == Facing::NORTH) tz++;
		if (face == Facing::SOUTH) tz--;
		if (face == Facing::WEST) tx++;
		if (face == Facing::EAST) tx--;
		bool isUpper = level && ((level->getData(tx, ty, tz) & TOP_SLOT_BIT) != 0);
		if (isUpper) {
			if (face == Facing::DOWN) return true;
			if (face == Facing::UP && super::shouldRenderFace(level, x, y, z, face)) return true;
			return level && (level->getTile(x, y, z) != id || (level->getData(x, y, z) & TOP_SLOT_BIT) == 0);
		} else {
			if (face == Facing::UP) return true;
			if (face == Facing::DOWN && super::shouldRenderFace(level, x, y, z, face)) return true;
			return level && (level->getTile(x, y, z) != id || (level->getData(x, y, z) & TOP_SLOT_BIT) != 0);
		}
	}

private:
	bool fullSize;
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__SlabTile_H__ */
