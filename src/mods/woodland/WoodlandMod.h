#ifndef POCKETMC_MODS_WOODLAND_WOODLANDMOD_H__
#define POCKETMC_MODS_WOODLAND_WOODLANDMOD_H__

class Tile;
class Item;
class Level;
class Entity;
class EntityRenderer;
class Random;

class WoodlandMod {
public:
    // Tile IDs (0..255)
    static const int Wood1TileId      = 202;
    static const int Wood2TileId      = 203;
    static const int Wood3TileId      = 204;
    static const int WoodKeyTileId    = 205;
    static const int WoodDoorTileId   = 206;
    static const int WoodPortalTileId = 207;

    // Item IDs (internal Item::items index is 256 + id)
    static const int WoodKeyItemId    = 221;
    static const int DoorMapItemId    = 222;
    static const int SpawnEggItemId   = 223;

    // Entity type IDs start at 10000
    static const int EntityTypeBase   = 10000;

    static const char* getId() { return "woodland"; }
    static const char* getName() { return "Woodland"; }
    static const char* getVersion() { return "1.0.0"; }

    static void registerTiles();
    static void registerItems();
    static void registerEntities();
    static void postProcessChunk(Level* level, Random* random, int chunkX, int chunkZ);
    static EntityRenderer* createRenderer(int entityTypeId);
    static Entity* createEntity(int entityTypeId, Level* level);

    static bool spawnMob(Level* level, const char* name, float x, float y, float z);
    static int  findMob(const char* name);

    static Tile* wood1;
    static Tile* wood2;
    static Tile* wood3;
    static Tile* woodKeyBlock;
    static Tile* woodDoor;
    static Tile* woodPortal;

    static Item* woodKey;
    static Item* doorMap;
    static Item* spawnEgg;
};

#endif