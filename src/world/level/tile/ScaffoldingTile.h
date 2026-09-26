#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__ScaffoldingTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__ScaffoldingTile_H__

#include "../material/Material.h"
#include "Tile.h"

class ScaffoldingTile : public Tile {
    typedef Tile super;
public:
    ScaffoldingTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    ScaffoldingTile(int id, const Material* mat)
        : super(id, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_SCAFFOLDING; }
    int getRenderLayer() override { return Tile::RENDERLAYER_ALPHATEST; }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__ScaffoldingTile_H__ */
