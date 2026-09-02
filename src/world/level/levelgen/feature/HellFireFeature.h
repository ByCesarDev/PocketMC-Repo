#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__HellFireFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__HellFireFeature_H__

#include "Feature.h"
#include "../../Level.h"
#include "../../tile/Tile.h"
#include "../../tile/FireTile.h"
#include "../../../../util/Random.h"

class HellFireFeature : public Feature
{
public:
    HellFireFeature() : Feature(false) {}

    bool place(Level* level, Random* random, int x, int y, int z) override
    {
        for (int i = 0; i < 64; i++) {
            int x2 = x + random->nextInt(8) - random->nextInt(8);
            int y2 = y + random->nextInt(4) - random->nextInt(4);
            int z2 = z + random->nextInt(8) - random->nextInt(8);

            if (level->isEmptyTile(x2, y2, z2) && Tile::netherrack && level->getTile(x2, y2 - 1, z2) == Tile::netherrack->id) {
                level->setTileNoUpdate(x2, y2, z2, Tile::fire->id);
            }
        }
        return true;
    }
};

#endif
