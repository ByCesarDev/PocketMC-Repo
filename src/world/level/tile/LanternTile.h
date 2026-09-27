#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__LanternTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__LanternTile_H__

#include "../material/Material.h"
#include "../LevelSource.h"
#include "../Level.h"
#include "Tile.h"

class LanternTile : public Tile {
    typedef Tile super;
public:
    LanternTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(5.0f / 16.0f, 0.0f, 5.0f / 16.0f, 11.0f / 16.0f, 9.0f / 16.0f, 11.0f / 16.0f);
    }

    LanternTile(int id, const Material* mat)
        : super(id, mat) {
        setShape(5.0f / 16.0f, 0.0f, 5.0f / 16.0f, 11.0f / 16.0f, 9.0f / 16.0f, 11.0f / 16.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_LANTERN; }
    int getRenderLayer() override { return Tile::RENDERLAYER_ALPHATEST; }

    static bool isHanging(LevelSource* level, int x, int y, int z) {
        if (!level) return false;
        return (level->isSolidBlockingTile(x, y + 1, z) || 
            (Tile::ironChain && level->getTile(x, y + 1, z) == Tile::ironChain->id) || 
            (Tile::fence && level->getTile(x, y + 1, z) == Tile::fence->id) || 
            (Tile::ironBars && level->getTile(x, y + 1, z) == Tile::ironBars->id) || 
            (Tile::lantern && level->getTile(x, y + 1, z) == Tile::lantern->id) || 
            (Tile::soulLantern && level->getTile(x, y + 1, z) == Tile::soulLantern->id) ||
            (level->getTile(x, y + 1, z) != 0 && !level->isSolidRenderTile(x, y - 1, z)));
    }

    void updateShape(LevelSource* level, int x, int y, int z) override {
        if (isHanging(level, x, y, z)) {
            setShape(5.0f / 16.0f, 1.0f / 16.0f, 5.0f / 16.0f, 11.0f / 16.0f, 10.0f / 16.0f, 11.0f / 16.0f);
        } else {
            setShape(5.0f / 16.0f, 0.0f, 5.0f / 16.0f, 11.0f / 16.0f, 9.0f / 16.0f, 11.0f / 16.0f);
        }
    }

    AABB* getAABB(Level* level, int x, int y, int z) override {
        updateShape(level, x, y, z);
        return super::getAABB(level, x, y, z);
    }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__LanternTile_H__ */

