#ifndef FLOWER_TILE_H
#define FLOWER_TILE_H

#include "Bush.h"

class FlowerTile : public Bush {
public:
    FlowerTile(int id, int textureIndex) : Bush(id, textureIndex) {
        this->setDestroyTime(0.0f);
        this->setSoundType(SOUND_GRASS);
        this->category = 2; // Decorations
    }

    virtual int getTexture(int face) override { return tex; }
    virtual int getTexture(int face, int data) override { return tex; }
    virtual int getTexture(LevelSource* level, int x, int y, int z, int face) override { return tex; }
};

#endif