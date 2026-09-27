#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__CarpetTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__CarpetTile_H__

#include "../material/Material.h"
#include "../LevelSource.h"
#include "../Level.h"
#include "Tile.h"

class CarpetTile : public Tile {
    typedef Tile super;
public:
    CarpetTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f / 16.0f, 1.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_BLOCK; }
    int getRenderLayer() override { return Tile::RENDERLAYER_ALPHATEST; }

    void updateDefaultShape() override {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f / 16.0f, 1.0f);
    }

    void updateShape(LevelSource* level, int x, int y, int z) override {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f / 16.0f, 1.0f);
    }

    AABB* getAABB(Level* level, int x, int y, int z) override {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f / 16.0f, 1.0f);
        return super::getAABB(level, x, y, z);
    }

    bool mayPlace(Level* level, int x, int y, int z) override {
        return level && level->isSolidBlockingTile(x, y - 1, z);
    }

    void neighborChanged(Level* level, int x, int y, int z, int type) override {
        if (!level->isSolidBlockingTile(x, y - 1, z)) {
            spawnResources(level, x, y, z, level->getData(x, y, z));
            level->setTile(x, y, z, 0);
        }
    }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__CarpetTile_H__ */
