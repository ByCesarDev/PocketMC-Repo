#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__ComposterTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__ComposterTile_H__

#include "../material/Material.h"
#include "Tile.h"

class ComposterTile : public Tile {
    typedef Tile super;
public:
    ComposterTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    ComposterTile(int id, const Material* mat)
        : super(id, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_COMPOSTER; }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__ComposterTile_H__ */
