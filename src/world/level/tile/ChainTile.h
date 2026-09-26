#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__ChainTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__ChainTile_H__

#include "../material/Material.h"
#include "Tile.h"

class ChainTile : public Tile {
    typedef Tile super;
public:
    ChainTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(6.5f / 16.0f, 0.0f, 6.5f / 16.0f, 9.5f / 16.0f, 1.0f, 9.5f / 16.0f);
    }

    ChainTile(int id, const Material* mat)
        : super(id, mat) {
        setShape(6.5f / 16.0f, 0.0f, 6.5f / 16.0f, 9.5f / 16.0f, 1.0f, 9.5f / 16.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_CHAIN; }
    int getRenderLayer() override { return Tile::RENDERLAYER_ALPHATEST; }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__ChainTile_H__ */
