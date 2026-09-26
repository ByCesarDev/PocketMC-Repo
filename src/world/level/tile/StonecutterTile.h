#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__StonecutterTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__StonecutterTile_H__

#include "../material/Material.h"
#include "Tile.h"

class StonecutterTile : public Tile {
    typedef Tile super;
public:
    StonecutterTile(int id, int tex, const Material* mat = Material::stone)
        : super(id, tex, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 9.0f / 16.0f, 1.0f);
    }

    StonecutterTile(int id, const Material* mat = Material::stone)
        : super(id, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 9.0f / 16.0f, 1.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_STONECUTTER; }
    int getRenderLayer() override { return Tile::RENDERLAYER_ALPHATEST; }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__StonecutterTile_H__ */
