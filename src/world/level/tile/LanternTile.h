#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__LanternTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__LanternTile_H__

#include "../material/Material.h"
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
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__LanternTile_H__ */
