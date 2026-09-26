#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__LecternTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__LecternTile_H__

#include "../material/Material.h"
#include "Tile.h"

class LecternTile : public Tile {
    typedef Tile super;
public:
    LecternTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    LecternTile(int id, const Material* mat)
        : super(id, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_LECTERN; }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__LecternTile_H__ */
