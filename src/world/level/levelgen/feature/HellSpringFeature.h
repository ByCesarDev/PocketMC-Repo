#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__HellSpringFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_FEATURE__HellSpringFeature_H__

#include "Feature.h"
#include "../../Level.h"
#include "../../tile/Tile.h"
#include "../../material/Material.h"
#include "../../../../util/Random.h"

class HellSpringFeature : public Feature
{
    int tile;
public:
    HellSpringFeature(int tile) : Feature(false), tile(tile) {}

    bool place(Level* level, Random* random, int x, int y, int z) override
    {
        if (!Tile::netherrack) return false;
        int netherrackId = Tile::netherrack->id;

        if (level->getTile(x, y + 1, z) != netherrackId) return false;
        if (level->getTile(x, y, z) != 0 && level->getTile(x, y, z) != netherrackId) return false;

        int rockCount = 0;
        if (level->getTile(x - 1, y, z) == netherrackId) rockCount++;
        if (level->getTile(x + 1, y, z) == netherrackId) rockCount++;
        if (level->getTile(x, y, z - 1) == netherrackId) rockCount++;
        if (level->getTile(x, y, z + 1) == netherrackId) rockCount++;
        if (level->getTile(x, y - 1, z) == netherrackId) rockCount++;

        int holeCount = 0;
        if (level->isEmptyTile(x - 1, y, z)) holeCount++;
        if (level->isEmptyTile(x + 1, y, z)) holeCount++;
        if (level->isEmptyTile(x, y, z - 1)) holeCount++;
        if (level->isEmptyTile(x, y, z + 1)) holeCount++;
        if (level->isEmptyTile(x, y - 1, z)) holeCount++;

        if (holeCount != 1) return true;
        if (rockCount != 4) return true;

        level->setTile(x, y, z, tile);
        level->instaTick = true;
        if (Tile::tiles[tile]) {
            Tile::tiles[tile]->tick(level, x, y, z, random);
        }
        level->instaTick = false;
        return true;
    }
};

#endif
