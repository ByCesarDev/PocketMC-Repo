#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__StairTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__StairTile_H__

//package net.minecraft.world.level.tile;

#include <vector>

#include "Tile.h"
#include "../material/Material.h"
#include "../../../util/Mth.h"
#include "../../../util/Random.h"
#include "../../entity/Mob.h"

class StairTile: public Tile
{
    typedef Tile super;
    Tile* base;

	bool isClipping;
	int clipStep;
	int baseData;

public:

	static const int UPSIDEDOWN_BIT = 4;

	// the direction is the way going up (for normal non-upsidedown stairs)
	static const int DIR_EAST = 0;
	static const int DIR_WEST = 1;
	static const int DIR_SOUTH = 2;
	static const int DIR_NORTH = 3;

	static const int DEAD_SPACES[8][2];

    StairTile(int id, Tile* base)
    :   super(id, base ? base->tex : 0, base ? base->material : Material::stone),
        base(base),
		isClipping(false),
		clipStep(0),
		baseData(0) // when needed in the future, set this to necessary texture data
    {
        if (base) {
            setDestroyTime(base->destroySpeed);
            setExplodeable(base->explosionResistance / 3);
            if (base->soundType) setSoundType(*base->soundType);
        }
		setLightBlock(255);
    }

    StairTile(int id, int tex, const Material* material)
    :   super(id, tex, material),
        base(NULL),
		isClipping(false),
		clipStep(0),
		baseData(0)
    {
        setDestroyTime(1.5f);
        setExplodeable(10.0f / 3.0f);
        setSoundType(SOUND_STONE);
		setLightBlock(255);
    }

	HitResult clip(Level* level, int xt, int yt, int zt, const Vec3& a, const Vec3& b);

    void updateShape(LevelSource* level, int x, int y, int z) {
		if (isClipping) {
			setShape(0.5f * (clipStep % 2), 0.5f * (clipStep / 2 % 2), 0.5f * (clipStep / 4 % 2), 0.5f + 0.5f * (clipStep % 2), 0.5f + 0.5f * (clipStep / 2 % 2), 0.5f + 0.5f * (clipStep / 4 % 2));
		} else {
			setShape(0, 0, 0, 1, 1, 1);
		}
    }

	void setBaseShape(LevelSource* level, int x, int y, int z) {
		int data = level->getData(x, y, z);

		if ((data & UPSIDEDOWN_BIT) != 0) {
			setShape(0, .5f, 0, 1, 1, 1);
		} else {
			setShape(0, 0, 0, 1, .5f, 1);
		}
	}

	static bool isStairs(int id) {
		return id > 0 && Tile::tiles[id]->getRenderShape() == Tile::SHAPE_STAIRS;
	}

	bool isLockAttached(LevelSource* level, int x, int y, int z, int data) {
		int lockTile = level->getTile(x, y, z);
		if (isStairs(lockTile) && level->getData(x, y, z) == data) {
			return true;
		}

		return false;
	}

	bool setStepShape(LevelSource* level, int x, int y, int z);
	bool setInnerPieceShape(LevelSource* level, int x, int y, int z);

//    AABB* getAABB(Level* level, int x, int y, int z) {
//        return super::getAABB(level, x, y, z);
////        return AABB.newTemp(x, y, z, x + 1, y + 1, z + 1);
//    }

    bool isSolidRender() {
        return false;
    }

    bool isCubeShaped() {
        return false;
    }

    int getRenderShape() {
        return Tile::SHAPE_STAIRS;
    }

    //bool shouldRenderFace(LevelSource* level, int x, int y, int z, int face) {
    //    return super::shouldRenderFace(level, x, y, z, face);
    //}

	void addAABBs(Level* level, int x, int y, int z, const AABB* box, std::vector<AABB>& boxes) {
		setBaseShape(level, x, y, z);
		super::addAABBs(level, x, y, z, box, boxes);

		bool checkInnerPiece = setStepShape(level, x, y, z);
		super::addAABBs(level, x, y, z, box, boxes);

		if (checkInnerPiece) {
			if (setInnerPieceShape(level, x, y, z)) {
				super::addAABBs(level, x, y, z, box, boxes);
			}
		}
        setShape(0, 0, 0, 1, 1, 1);
    }

    /*
     * void neighborChanged(Level level, int x, int y, int z, int type) {
     * if (level.isOnline) return; if (level.getMaterial(x, y + 1, z).isSolid())
     * { level.setTile(x, y, z, base.id); } else { checkStairs(level, x, y, z);
     * checkStairs(level, x + 1, y - 1, z); checkStairs(level, x - 1, y - 1, z);
     * checkStairs(level, x, y - 1, z - 1); checkStairs(level, x, y - 1, z + 1);
     * checkStairs(level, x + 1, y + 1, z); checkStairs(level, x - 1, y + 1, z);
     * checkStairs(level, x, y + 1, z - 1); checkStairs(level, x, y + 1, z + 1);
     * } base.neighborChanged(level, x, y, z, type); }
     */

    /** DELEGATES: **/

    void addLights(Level* level, int x, int y, int z) {
        if (base) base->addLights(level, x, y, z);
        else super::addLights(level, x, y, z);
    }

    void animateTick(Level* level, int x, int y, int z, Random* random) {
        if (base) base->animateTick(level, x, y, z, random);
        else super::animateTick(level, x, y, z, random);
    }

    void attack(Level* level, int x, int y, int z, Player* player) {
        if (base) base->attack(level, x, y, z, player);
        else super::attack(level, x, y, z, player);
    }

    void destroy(Level* level, int x, int y, int z, int data) {
        if (base) base->destroy(level, x, y, z, data);
        else super::destroy(level, x, y, z, data);
    }

    float getBrightness(LevelSource* level, int x, int y, int z) {
        if (base) return base->getBrightness(level, x, y, z);
        return super::getBrightness(level, x, y, z);
    }

    float getExplosionResistance(Entity* source) {
        if (base) return base->getExplosionResistance(source);
        return explosionResistance;
    }

    int getRenderLayer() {
        if (base) return base->getRenderLayer();
        return super::getRenderLayer();
    }

    int getResourceCount(Random* random) {
        if (base) return base->getResourceCount(random);
        return super::getResourceCount(random);
    }

    int getTexture(int face, int data) {
        if (useMaterialInstances || !base) return super::getTexture(face, data);
        return base->getTexture(face, baseData);
    }

    int getTexture(int face) {
        if (useMaterialInstances || !base) return super::getTexture(face);
        return base->getTexture(face, baseData);
    }

    int getTexture(LevelSource* level, int x, int y, int z, int face) {
        if (useMaterialInstances || !base) return super::getTexture(level, x, y, z, face);
        return base->getTexture(face, baseData);
    }

    int getTickDelay() {
        if (base) return base->getTickDelay();
        return super::getTickDelay();
    }

    AABB getTileAABB(Level* level, int x, int y, int z) {
        if (base) return base->getTileAABB(level, x, y, z);
        return super::getTileAABB(level, x, y, z);
    }

    void handleEntityInside(Level* level, int x, int y, int z, Entity* e, Vec3& current) {
        if (base) base->handleEntityInside(level, x, y, z, e, current);
        else super::handleEntityInside(level, x, y, z, e, current);
    }

    bool mayPick() {
        if (base) return base->mayPick();
        return super::mayPick();
    }

    bool mayPick(int data, bool liquid) {
        if (base) return base->mayPick(data, liquid);
        return super::mayPick(data, liquid);
    }

    bool mayPlace(Level* level, int x, int y, int z, unsigned char face) {
        if (base) return base->mayPlace(level, x, y, z);
        return super::mayPlace(level, x, y, z, face);
    }

    void onPlace(Level* level, int x, int y, int z) {
        neighborChanged(level, x, y, z, 0);
        if (base) base->onPlace(level, x, y, z);
    }

    void onRemove(Level* level, int x, int y, int z) {
        if (base) base->onRemove(level, x, y, z);
    }

    void prepareRender(Level* level, int x, int y, int z) {
        if (base) base->prepareRender(level, x, y, z);
    }

    void stepOn(Level* level, int x, int y, int z, Entity* entity) {
        if (base) base->stepOn(level, x, y, z, entity);
    }

    void tick(Level* level, int x, int y, int z, Random* random) {
        if (base) base->tick(level, x, y, z, random);
    }

    bool use(Level* level, int x, int y, int z, Player* player) {
        if (base) return base->use(level, x, y, z, player);
        return super::use(level, x, y, z, player);
    }

    void wasExploded(Level* level, int x, int y, int z) {
        if (base) base->wasExploded(level, x, y, z);
    }

    void setPlacedBy(Level* level, int x, int y, int z, Mob* by) {
        int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5f)) & 3;
		int usd = level->getData(x, y, z) & UPSIDEDOWN_BIT;

        if (dir == 0) level->setData(x, y, z, 2 | usd);
        if (dir == 1) level->setData(x, y, z, 1 | usd);
        if (dir == 2) level->setData(x, y, z, 3 | usd);
        if (dir == 3) level->setData(x, y, z, 0 | usd);
    }

	int getPlacedOnFaceDataValue(Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);

};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__StairTile_H__*/
