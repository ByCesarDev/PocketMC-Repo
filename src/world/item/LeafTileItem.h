#ifndef NET_MINECRAFT_WORLD_ITEM__LeafTileItem_H__
#define NET_MINECRAFT_WORLD_ITEM__LeafTileItem_H__

//package net.minecraft.world.item;

#include "TileItem.h"
#include "../level/tile/LeafTile.h"
#include "../level/FoliageColor.h"

class LeafTileItem: public TileItem
{
    typedef TileItem super;
public:
    LeafTileItem(int id)
    :   super(id)
    {
        setMaxDamage(0);
        setStackedByData(true);
    }

    int getLevelDataForAuxValue(int auxValue) {
        return auxValue | LeafTile::PERSISTENT_LEAF_BIT;
    }

    int getIcon(int itemAuxValue) {
        return Tile::tiles[tileId] ? Tile::tiles[tileId]->getTexture(0, itemAuxValue) : 0;
    }

    int getColor(int data) {
        LeafTile* leaf = Tile::tiles[tileId] ? (LeafTile*)Tile::tiles[tileId] : NULL;
        int type = leaf ? leaf->getLeafType(data) : (data & LeafTile::LEAF_TYPE_MASK);
        if (type == LeafTile::EVERGREEN_LEAF) {
            return FoliageColor::getEvergreenColor();
        }
        if (type == LeafTile::BIRCH_LEAF) {
            return FoliageColor::getBirchColor();
        }
        if (type == LeafTile::JUNGLE_LEAF) {
            return 0x30bb0b;
        }
        if (type == LeafTile::ACACIA_LEAF) {
            return 0xaea42a;
        }
        if (type == LeafTile::DARK_OAK_LEAF) {
            return 0x3b5919;
        }
        return FoliageColor::getDefaultColor();
    }
};

#endif /*NET_MINECRAFT_WORLD_ITEM__LeafTileItem_H__*/
