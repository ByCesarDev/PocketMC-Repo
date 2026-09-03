#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__Sapling_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__Sapling_H__

//package net.minecraft.world.level->tile;

#include "Bush.h"
#include "../Level.h"
#include "../levelgen/feature/SpruceFeature.h"
#include "../levelgen/feature/BirchFeature.h"
#include "../levelgen/feature/TreeFeature.h"

class Sapling: public Bush
{
    typedef Bush super;

    static const int TYPE_MASK = 3;
    static const int AGE_BIT = 8;
    int treeType;
public:
    Sapling(int id, int tex, int treeType = LeafTile::NORMAL_LEAF)
    :   super(id, tex), treeType(treeType)
    {
        float ss = 0.4f;
        setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, ss * 2, 0.5f + ss);
    }

    int getTreeType(int data = 0) const {
        if (this == Tile::spruceSapling)  return LeafTile::EVERGREEN_LEAF;
        if (this == Tile::birchSapling)   return LeafTile::BIRCH_LEAF;
        if (this == Tile::jungleSapling)  return LeafTile::JUNGLE_LEAF;
        if (this == Tile::acaciaSapling)  return LeafTile::ACACIA_LEAF;
        if (this == Tile::darkOakSapling) return LeafTile::DARK_OAK_LEAF;
        if (treeType != LeafTile::NORMAL_LEAF) return treeType;
        return (data & LeafTile::LEAF_TYPE_MASK);
    }

    void tick(Level* level, int x, int y, int z, Random* random) {
        if (level->isClientSide) return;

        super::tick(level, x, y, z, random);

        if (level->getRawBrightness(x, y + 1, z) >= Level::MAX_BRIGHTNESS - 6) {
            if (random->nextInt(7) == 0) {
                int data = level->getData(x, y, z);
                if ((data & AGE_BIT) == 0) {
					//@attn @fix : was setData, but it only works if using sendTileUpdate
                    level->setDataNoUpdate(x, y, z, data | AGE_BIT);
                } else {
                    growTree(level, x, y, z, random);
                }
            }
        }
    }

    /*@Override*/
    int getTexture(int face, int data) {
        int t = getTreeType(data);
        if (t == LeafTile::EVERGREEN_LEAF) {
            return 15 + 16 * 3;
        } else if (t == LeafTile::BIRCH_LEAF) {
            return 15 + 16 * 4;
        } else if (t == LeafTile::JUNGLE_LEAF) {
            return 39 | Tile::TEXTURE_ALT_FLAG;
        } else if (t == LeafTile::ACACIA_LEAF) {
            return 40 | Tile::TEXTURE_ALT_FLAG;
        } else if (t == LeafTile::DARK_OAK_LEAF) {
            return 54 | Tile::TEXTURE_ALT_FLAG;
        } else {
            return super::getTexture(face, data);
        }
    }

    void growTree(Level* level, int x, int y, int z, Random* random) {
        int data = level->getData(x, y, z);
        int t = getTreeType(data);

        Feature* f = NULL;

        int ox = 0, oz = 0;
        bool multiblock = false;

        if (t == LeafTile::EVERGREEN_LEAF) {
            f = new SpruceFeature(true);
        } else if (t == LeafTile::BIRCH_LEAF) {
            f = new BirchFeature(true);
        } else {
            f = new TreeFeature(true);
        }

        if (multiblock) {
            level->setTileNoUpdate(x + ox, y, z + oz, 0);
            level->setTileNoUpdate(x + ox + 1, y, z + oz, 0);
            level->setTileNoUpdate(x + ox, y, z + oz + 1, 0);
            level->setTileNoUpdate(x + ox + 1, y, z + oz + 1, 0);
        } else {
            level->setTileNoUpdate(x, y, z, 0);
        }

        if (!f->place(level, random, x + ox, y, z + oz)) {
            if (multiblock) {
                level->setTileAndDataNoUpdate(x + ox, y, z + oz, this->id, data);
                level->setTileAndDataNoUpdate(x + ox + 1, y, z + oz, this->id, data);
                level->setTileAndDataNoUpdate(x + ox, y, z + oz + 1, this->id, data);
                level->setTileAndDataNoUpdate(x + ox + 1, y, z + oz + 1, this->id, data);
            } else {
                level->setTileAndDataNoUpdate(x, y, z, this->id, data);
            }
        }

        if (f) delete f;
    }

    bool isSapling(Level* level, int x, int y, int z, int type) {
        return (level->getTile(x, y, z) == id) && ((level->getData(x, y, z) & TYPE_MASK) == type);
    }

protected:
    int getSpawnResourcesAuxValue(int data) {
        return data & TYPE_MASK;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__Sapling_H__*/
