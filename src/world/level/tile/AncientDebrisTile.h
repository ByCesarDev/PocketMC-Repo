#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE_ANCIENTDEBRISTILE_H
#define NET_MINECRAFT_WORLD_LEVEL_TILE_ANCIENTDEBRISTILE_H

#include "Tile.h"

class AncientDebrisTile : public Tile {
public:
    AncientDebrisTile(int id);

    int getTexture(int face) override;
    int getTexture(int face, int data) override;
    int getTexture(LevelSource* level, int x, int y, int z, int face) override;
};

#endif
