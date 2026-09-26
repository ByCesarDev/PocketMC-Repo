#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__GrindstoneTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__GrindstoneTile_H__

#include "../material/Material.h"
#include "Tile.h"

class GrindstoneTile : public Tile {
    typedef Tile super;
public:
    GrindstoneTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(2.0f / 16.0f, 0.0f, 2.0f / 16.0f, 14.0f / 16.0f, 1.0f, 14.0f / 16.0f);
    }

    GrindstoneTile(int id, const Material* mat)
        : super(id, mat) {
        setShape(2.0f / 16.0f, 0.0f, 2.0f / 16.0f, 14.0f / 16.0f, 1.0f, 14.0f / 16.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_GRINDSTONE; }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__GrindstoneTile_H__ */
