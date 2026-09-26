#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__SoulFireTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__SoulFireTile_H__

#include "FireTile.h"

class SoulFireTile : public FireTile {
    typedef FireTile super;
public:
    SoulFireTile(int id, int tex)
        : super(id, tex) {
        setDestroyTime(0.0f);
        setLightEmission(10 / 16.0f);
        setSoundType(SOUND_WOOD);
        setTicking(true);
    }

    AABB* getAABB(Level* level, int x, int y, int z) override {
        return NULL;
    }

    bool blocksLight() {
        return false;
    }

    bool isSolidRender() override {
        return false;
    }

    bool isCubeShaped() override {
        return false;
    }

    int getRenderShape() override {
        return Tile::SHAPE_FIRE;
    }

    int getRenderLayer() override {
        return Tile::RENDERLAYER_ALPHATEST;
    }

    int getResourceCount(Random* random) override {
        return 0;
    }

    bool mayPick() override {
        return false;
    }

    int getTickDelay() override {
        return 10;
    }

    bool mayPlace(Level* level, int x, int y, int z, unsigned char face) override {
        int under = level->getTile(x, y - 1, z);
        return (Tile::soulSand != NULL && under == Tile::soulSand->id) || level->isSolidBlockingTile(x, y - 1, z);
    }

    void neighborChanged(Level* level, int x, int y, int z, int type) override {
        int under = level->getTile(x, y - 1, z);
        if (!level->isSolidBlockingTile(x, y - 1, z) && (Tile::soulSand == NULL || under != Tile::soulSand->id)) {
            level->setTile(x, y, z, 0);
        }
    }

    void onPlace(Level* level, int x, int y, int z) override {
        int under = level->getTile(x, y - 1, z);
        if (!level->isSolidBlockingTile(x, y - 1, z) && (Tile::soulSand == NULL || under != Tile::soulSand->id)) {
            level->setTile(x, y, z, 0);
            return;
        }
        level->addToTickNextTick(x, y, z, id, getTickDelay());
    }

    void tick(Level* level, int x, int y, int z, Random* random) override {
        int under = level->getTile(x, y - 1, z);
        bool infiniBurn = (Tile::soulSand != NULL && under == Tile::soulSand->id);
        if (!infiniBurn && !level->isSolidBlockingTile(x, y - 1, z)) {
            level->setTile(x, y, z, 0);
            return;
        }
        // Soul fire burns indefinitely on soul sand and does not spread destructively to nearby blocks
    }

    void animateTick(Level* level, int x, int y, int z, Random* random) override {
        if (random->nextInt(24) == 0) {
            level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, "fire.fire", 1 + random->nextFloat(), random->nextFloat() * 0.7f + 0.3f);
        }
        for (int i = 0; i < 3; i++) {
            float xx = x + random->nextFloat();
            float yy = y + random->nextFloat() * 0.5f + 0.5f;
            float zz = z + random->nextFloat();
            level->addParticle(PARTICLETYPE(largesmoke), xx, yy, zz, 0, 0, 0);
        }
    }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__SoulFireTile_H__ */
