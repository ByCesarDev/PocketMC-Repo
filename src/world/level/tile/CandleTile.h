#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__CandleTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__CandleTile_H__

#include "../material/Material.h"
#include "../LevelSource.h"
#include "../Level.h"
#include "Tile.h"

class CandleTile : public Tile {
    typedef Tile super;
public:
    CandleTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(7.0f / 16.0f, 0.0f, 7.0f / 16.0f, 9.0f / 16.0f, 6.0f / 16.0f, 9.0f / 16.0f);
    }

    CandleTile(int id, const Material* mat)
        : super(id, mat) {
        setShape(7.0f / 16.0f, 0.0f, 7.0f / 16.0f, 9.0f / 16.0f, 6.0f / 16.0f, 9.0f / 16.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_CANDLE; }
    int getRenderLayer() override { return Tile::RENDERLAYER_ALPHATEST; }

    AABB* getAABB(Level* level, int x, int y, int z) override {
        setShape(7.0f / 16.0f, 0.0f, 7.0f / 16.0f, 9.0f / 16.0f, 6.0f / 16.0f, 9.0f / 16.0f);
        return super::getAABB(level, x, y, z);
    }

    bool mayPlace(Level* level, int x, int y, int z, unsigned char face = 0) override {
        if (!level) return false;
        return level->isSolidBlockingTile(x, y - 1, z);
    }

    void neighborChanged(Level* level, int x, int y, int z, int type) override {
        super::neighborChanged(level, x, y, z, type);
        if (!mayPlace(level, x, y, z)) {
            spawnResources(level, x, y, z, level->getData(x, y, z));
            level->setTile(x, y, z, 0);
        }
    }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__CandleTile_H__ */
