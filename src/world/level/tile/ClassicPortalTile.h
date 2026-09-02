#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__ClassicPortalTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__ClassicPortalTile_H__

#include "Tile.h"
#include "../../../util/Random.h"

class ClassicPortalTile : public Tile
{
    typedef Tile super;
public:
    ClassicPortalTile(int id, int tex, const Material* material);

    AABB* getAABB(Level* level, int x, int y, int z) override;
    bool isCubeShaped() override;

    bool isSolidRender() override { return false; }

    void updateShape(LevelSource* level, int x, int y, int z) override;

    int getRenderLayer() override;
    int getResourceCount(Random* random) override;

    void neighborChanged(Level* level, int x, int y, int z, int type) override;
    void entityInside(Level* level, int x, int y, int z, Entity* entity) override;
    bool shouldRenderFace(LevelSource* level, int x, int y, int z, int face) override;
    void animateTick(Level* level, int x, int y, int z, Random* random) override;

    bool trySpawnPortal(Level* level, int x, int y, int z);
};

#endif
