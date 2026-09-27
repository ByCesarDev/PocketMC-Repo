#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__CampfireTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__CampfireTile_H__

#include "../material/Material.h"
#include "Tile.h"
#include "../../entity/Entity.h"

class CampfireTile : public Tile {
    typedef Tile super;
public:
    CampfireTile(int id, int tex, const Material* mat)
        : super(id, tex, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 7.0f / 16.0f, 1.0f);
    }

    CampfireTile(int id, const Material* mat)
        : super(id, mat) {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 7.0f / 16.0f, 1.0f);
    }

    bool isSolidRender() override { return false; }
    bool isCubeShaped() override { return false; }
    int getRenderShape() override { return Tile::SHAPE_CAMPFIRE; }
    int getRenderLayer() override { return Tile::RENDERLAYER_ALPHATEST; }

    void entityInside(Level* level, int x, int y, int z, Entity* entity) override {
        if (entity) {
            entity->hurt(NULL, 1);
        }
    }

    void stepOn(Level* level, int x, int y, int z, Entity* entity) override {
        if (entity) {
            entity->hurt(NULL, 1);
        }
    }
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__CampfireTile_H__ */
