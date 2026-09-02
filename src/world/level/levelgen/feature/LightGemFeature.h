#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__LightGemFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__LightGemFeature_H__

#include "Feature.h"
#include "../../Level.h"
#include "../../tile/Tile.h"
#include "../../../../util/Random.h"

class LightGemFeature : public Feature
{
public:
    LightGemFeature() : Feature(false) {}

    bool place(Level* level, Random* random, int x, int y, int z) override
    {
        if (!Tile::netherrack || !Tile::lightGem)
            return false;

        int netherrackId = Tile::netherrack->id;
        int lightGemId = Tile::lightGem->id;

        if (!level->isEmptyTile(x, y, z))
            return false;

        if (level->getTile(x, y + 1, z) != netherrackId)
            return false;

        level->setTileNoUpdate(x, y, z, lightGemId);

        for (int i = 0; i < 1500; i++) {
            int x2 = x + random->nextInt(8) - random->nextInt(8);
            int y2 = y - random->nextInt(12);
            int z2 = z + random->nextInt(8) - random->nextInt(8);

            if (level->isEmptyTile(x2, y2, z2)) {
                int relCount = 0;
                if (level->getTile(x2 - 1, y2, z2) == lightGemId) relCount++;
                if (level->getTile(x2 + 1, y2, z2) == lightGemId) relCount++;
                if (level->getTile(x2, y2 - 1, z2) == lightGemId) relCount++;
                if (level->getTile(x2, y2 + 1, z2) == lightGemId) relCount++;
                if (level->getTile(x2, y2, z2 - 1) == lightGemId) relCount++;
                if (level->getTile(x2, y2, z2 + 1) == lightGemId) relCount++;

                if (relCount == 1) {
                    level->setTileNoUpdate(x2, y2, z2, lightGemId);
                }
            }
        }

        return true;
    }
};

#endif
