#include "WoodlandMod.h"
#include "WoodlandAtlas.h"
#include "WoodlandData.h"
#include "WoodlandMob.h"
#include "WoodlandStructures.h"

#include "../../world/entity/player/Player.h"
#include "../../world/item/Item.h"
#include "../../world/item/ItemCategory.h"
#include "../../world/item/ItemInstance.h"
#include "../../world/level/Level.h"
#include "../../world/level/material/Material.h"
#include "../../world/level/tile/Tile.h"
#include "../../util/Random.h"

#ifndef STANDALONE_SERVER
#include "../../client/renderer/entity/MobRenderer.h"
#include "WoodlandModel.h"
#endif

#include <cmath>
#include <cstddef>
#include <cstring>

Tile* WoodlandMod::wood1 = NULL;       Tile* WoodlandMod::wood2 = NULL;
Tile* WoodlandMod::wood3 = NULL;       Tile* WoodlandMod::woodKeyBlock = NULL;
Tile* WoodlandMod::woodDoor = NULL;    Tile* WoodlandMod::woodPortal = NULL;
Item* WoodlandMod::woodKey = NULL;     Item* WoodlandMod::doorMap = NULL;
Item* WoodlandMod::spawnEgg = NULL;

// ---- wood_key_block: hardness 1.2, always drops a wood_key ----------------------------
class WoodKeyTile : public Tile {
public:
    WoodKeyTile(int id, int tex, const Material* m) : Tile(id, tex, m) {}
    virtual int getResource(int data, Random* random) { return 256 + WoodlandMod::WoodKeyItemId; }
};

// ---- spawn egg: aux value picks the mob ----------------------------------------------
class WoodlandSpawnEgg : public Item {
public:
    WoodlandSpawnEgg(int id) : Item(id) {
        this->setMaxStackSize(64);
    }

    virtual bool useOn(ItemInstance* item, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) override {
        if (!level || level->isClientSide) return true;
        
        static const int dx[6] = { 0, 0, 0, 0, -1, 1 };
        static const int dy[6] = { -1, 1, 0, 0, 0, 0 };
        static const int dz[6] = { 0, 0, -1, 1, 0, 0 };
        
        int idx = item->getAuxValue();
        if (idx < 0 || idx >= WL_MOB_COUNT) idx = 0;
        
        WoodlandMob* mob = new WoodlandMob(level, idx);
        mob->moveTo((float)x + dx[face] + 0.5f, (float)(y + dy[face]), (float)z + dz[face] + 0.5f, 0, 0);
        level->addEntity(mob);
        
        if (!player->abilities.instabuild) {
            item->count--;
        }
        return true;
    }
};

#define WL_TILE(var, cls, id, tex, hard, name)                                     \
    if (!var && !Tile::tiles[id]) {                                                \
        var = (new cls(id, tex, Material::wood))->init()                           \
            ->setDestroyTime(hard)->setSoundType(Tile::SOUND_WOOD)                 \
            ->setCategory(ItemCategory::Decorations)->setDescriptionId(name);      \
    }

void WoodlandMod::registerTiles()
{
    WL_TILE(wood1,        Tile,        Wood1TileId,      WL_TEX_WOOD1,     2.0f, "woodland.wood1");
    WL_TILE(wood2,        Tile,        Wood2TileId,      WL_TEX_WOOD2,     2.0f, "woodland.wood2");
    WL_TILE(wood3,        Tile,        Wood3TileId,      WL_TEX_WOOD3,     2.0f, "woodland.wood3");
    WL_TILE(woodKeyBlock, WoodKeyTile, WoodKeyTileId,    WL_TEX_KEY_BLOCK, 1.2f, "woodland.wood_key_block");
    WL_TILE(woodDoor,     Tile,        WoodDoorTileId,   WL_TEX_DOOR,      2.0f, "woodland.wood_door");
    WL_TILE(woodPortal,   Tile,        WoodPortalTileId, WL_TEX_PORTAL,    2.0f, "woodland.wood_portal");
}

void WoodlandMod::registerItems()
{
    if (!woodKey && !Item::items[256 + WoodKeyItemId])
        woodKey = (new Item(WoodKeyItemId))->setIcon(WL_ICON_KEY_X, WL_ICON_KEY_Y)
            ->setCategory(ItemCategory::Decorations)->setDescriptionId("woodland.wood_key");
    if (!doorMap && !Item::items[256 + DoorMapItemId])
        doorMap = (new Item(DoorMapItemId))->setIcon(WL_ICON_MAP_X, WL_ICON_MAP_Y)
            ->setCategory(ItemCategory::Decorations)->setDescriptionId("woodland.door_map");
    if (!spawnEgg && !Item::items[256 + SpawnEggItemId])
        spawnEgg = (new WoodlandSpawnEgg(SpawnEggItemId))->setIcon(WL_ICON_EGG_X, WL_ICON_EGG_Y)
            ->setCategory(ItemCategory::Decorations)->setDescriptionId("woodland.spawn_egg");
}

void WoodlandMod::registerEntities()
{
}

int WoodlandMod::findMob(const char* name)
{
    for (int i = 0; i < WL_MOB_COUNT; ++i)
        if (strcmp(WL_MOBS[i].name, name) == 0) return i;
    return -1;
}

bool WoodlandMod::spawnMob(Level* level, const char* name, float x, float y, float z)
{
    int idx = findMob(name);
    if (idx < 0) return false;
    WoodlandMob* mob = new WoodlandMob(level, idx);
    mob->moveTo(x, y, z, 0, 0);
    level->addEntity(mob);
    return true;
}

Entity* WoodlandMod::createEntity(int typeId, Level* level)
{
    int idx = typeId - EntityTypeBase;
    if (idx < 0 || idx >= WL_MOB_COUNT) return 0;
    return new WoodlandMob(level, idx);
}

EntityRenderer* WoodlandMod::createRenderer(int typeId)
{
#ifndef STANDALONE_SERVER
    int idx = typeId - EntityTypeBase;
    if (idx < 0 || idx >= WL_MOB_COUNT) return 0;
    return new MobRenderer(new WoodlandModel(&WL_MOBS[idx]), 0.5f);
#else
    return 0;
#endif
}

void WoodlandMod::postProcessChunk(Level* level, Random* random, int chunkX, int chunkZ)
{
    if (random->nextInt(150) != 0 || WL_STRUCTURE_COUNT == 0) return;
    int s = random->nextInt(WL_STRUCTURE_COUNT);
    int x = chunkX * 16 + 8, z = chunkZ * 16 + 8;
    int y = level->getTopSolidBlock(x, z);
    WoodlandStructures::place(level, s, x, y, z);
}