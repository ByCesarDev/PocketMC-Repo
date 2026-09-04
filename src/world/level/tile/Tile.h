#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__Tile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__Tile_H__

//package net.minecraft.world.level.tile;

#include <string>
#include <map>
#include <set>
#include "../../phys/AABB.h"

class Entity;
class Mob;
class Player;
class Level;
class LevelSource;
class Material;
class Random;
class ItemInstance;

class Bush;
class GrassTile;
class LeafTile;
class FireTile;
class FlowerTile;

class ModRegistry; // Forward declaration for friend access

// @Note: Got a memory leak in initTiles? You probably didn't call
//        Tile::init after constructing the tile!
class Tile
{
    friend class ModRegistry; // Allow ModRegistry to access private/protected members

    static const std::string TILE_DESCRIPTION_PREFIX;

public:
    class SoundType
	{
	public:
        //const std::string name;
        const float volume;
        const float pitch;
		const std::string breakSound;
		const std::string stepSound;

        SoundType(const std::string& name, float volume, float pitch)
		:	volume(volume),
			pitch(pitch),
			breakSound("step." + name),
			stepSound("step." + name)
		{}
		SoundType(const std::string& name, const std::string& breakSound, float volume, float pitch)
		:	volume(volume),
			pitch(pitch),
			stepSound("step." + name),
			breakSound(breakSound)
		{}

        float getVolume() const {
            return volume;
        }

        float getPitch() const {
            return pitch;
        }

        const std::string& getBreakSound() const {
            return breakSound;
        }

        const std::string& getStepSound() const {
            return stepSound;
        }
	};

	static const SoundType SOUND_NORMAL;
	static const SoundType SOUND_WOOD;
	static const SoundType SOUND_GRAVEL;
	static const SoundType SOUND_GRASS;
	static const SoundType SOUND_STONE;
	static const SoundType SOUND_METAL;
	static const SoundType SOUND_GLASS;
	static const SoundType SOUND_CLOTH;
	static const SoundType SOUND_SAND;
	static const SoundType SOUND_SILENT;

    static const int SHAPE_INVISIBLE = -1;
    static const int SHAPE_BLOCK = 0;
    static const int SHAPE_CROSS_TEXTURE = 1;
    static const int SHAPE_TORCH = 2;
    static const int SHAPE_FIRE = 3;
    static const int SHAPE_WATER = 4;
    static const int SHAPE_RED_DUST = 5;
    static const int SHAPE_ROWS = 6;
    static const int SHAPE_DOOR = 7;
    static const int SHAPE_LADDER = 8;
    static const int SHAPE_RAIL = 9;
    static const int SHAPE_STAIRS = 10;
    static const int SHAPE_FENCE = 11;
    static const int SHAPE_LEVER = 12;
    static const int SHAPE_CACTUS = 13;
    static const int SHAPE_BED = 14;
    static const int SHAPE_DIODE = 15;
    static const int SHAPE_IRON_FENCE = 18;
	static const int SHAPE_STEM = 19;
    static const int SHAPE_FENCE_GATE = 21;
	static const int SHAPE_ENTITYTILE_ANIMATED = 22;

	// Enumeración de caras del bloque para material_instances
	enum BlockFace {
		FACE_DOWN = 0,
		FACE_UP = 1,
		FACE_NORTH = 2,
		FACE_SOUTH = 3,
		FACE_WEST = 4,
		FACE_EAST = 5
	};

	// Estructura para material_instance
	struct MaterialInstance {
		std::string textureName;  // Nombre de textura separada
		int textureIndex;         // Índice en atlas (fallback)
		
		MaterialInstance() : textureIndex(0) {}
		MaterialInstance(const std::string& name, int index = 0) 
			: textureName(name), textureIndex(index) {}
		
		bool usesSeparateTexture() const {
			return !textureName.empty();
		}
	};

	static const int NUM_BLOCK_TYPES = 2048;

    // Flag OR'ed into texture indices to indicate the texture is in the
    // secondary atlas (terrain2.png) instead of the main terrain.png atlas. (Index >= 4096)
    static const int TEXTURE_ALT_FLAG = 0x1000;

    static Tile* tiles[NUM_BLOCK_TYPES];

	static bool sendTileData[NUM_BLOCK_TYPES];
    static bool shouldTick[NUM_BLOCK_TYPES];
    static bool solid[NUM_BLOCK_TYPES];
    static bool isEntityTile[NUM_BLOCK_TYPES];
    static int lightBlock[NUM_BLOCK_TYPES];
    static bool translucent[NUM_BLOCK_TYPES];
    static int lightEmission[NUM_BLOCK_TYPES];

    static Tile* rock;
	static Tile* grass;
    static Tile* dirt;
    static Tile* stoneBrick;
    static Tile* wood;
    static Tile* birchPlanks;
    static Tile* sprucePlanks;
    static Tile* junglePlanks;
    static Tile* acaciaPlanks;
    static Tile* darkOakPlanks;
    static Tile* spruceSlab;
    static Tile* spruceSlabHalf;
    static Tile* birchSlab;
    static Tile* birchSlabHalf;
    static Tile* stairs_spruce;
    static Tile* stairs_birch;
    static Tile* fenceBirch;
    static Tile* fenceSpruce;
    static Tile* fenceGateBirch;
    static Tile* fenceGateSpruce;
    static Tile* stairs_jungle;
    static Tile* jungleSlab;
    static Tile* jungleSlabHalf;
    static Tile* fenceJungle;
    static Tile* fenceGateJungle;
    static Tile* stairs_acacia;
    static Tile* acaciaSlab;
    static Tile* acaciaSlabHalf;
    static Tile* fenceAcacia;
    static Tile* fenceGateAcacia;
    static Tile* stairs_darkOak;
    static Tile* darkOakSlab;
    static Tile* darkOakSlabHalf;
    static Tile* fenceDarkOak;
    static Tile* fenceGateDarkOak;
    static Tile* sandstoneSlab;
    static Tile* sandstoneSlabHalf;
    static Tile* oakSlab;
    static Tile* oakSlabHalf;
    static Tile* cobbleSlab;
    static Tile* cobbleSlabHalf;
    static Tile* brickSlab;
    static Tile* brickSlabHalf;
    static Tile* stoneBrickSlab;
    static Tile* stoneBrickSlabHalf;
    static Tile* netherBrickSlab;
    static Tile* netherBrickSlabHalf;
    static Tile* quartzSlab;
    static Tile* quartzSlabHalf;
    static Tile* sapling;
    static Tile* spruceSapling;
    static Tile* birchSapling;
    static Tile* jungleSapling;
    static Tile* acaciaSapling;
    static Tile* darkOakSapling;
    static Tile* unbreakable;
    static Tile* water;
    static Tile* calmWater;
    static Tile* lava;
    static Tile* calmLava;
    static Tile* sand;
    static Tile* gravel;
    static Tile* goldOre;
    static Tile* ironOre;
    static Tile* coalOre;
    static Tile* treeTrunk;
    static Tile* birchTrunk;
    static Tile* spruceTrunk;
    static Tile* jungleTrunk;
    static Tile* acaciaTrunk;
	static LeafTile* leaves;
	static LeafTile* spruceLeaves;
	static LeafTile* birchLeaves;
	static LeafTile* jungleLeaves;
	static LeafTile* acaciaLeaves;
	static LeafTile* darkOakLeaves;
    static Tile* sponge;
	static Tile* web;
    static Tile* glass;
	static Tile* thinGlass;
    static Tile* lapisOre;
    static Tile* lapisBlock;
    static Tile* dispenser;
    static Tile* sandStone;
    static Tile* musicBlock;
    static Tile* bed;
    static Tile* unused_27;
    static Tile* unused_28;
    static Tile* unused_29;
    static Tile* unused_30;
    static Tile* tallgrass;
    static Tile* unused_32;
    static Tile* unused_33;
    static Tile* unused_34;
    static Tile* unused_36;

	static Tile* cloth;
	static Tile* flower;
	static Tile* rose;
	static Tile* dandelion;
	static Tile* cornflower;
	static Tile* mushroom1;
	static Tile* mushroom2;
    static Tile* goldBlock;
    static Tile* ironBlock;
    static Tile* stoneSlab;
    static Tile* stoneSlabHalf;
    static Tile* redBrick;
    static Tile* tnt;
    static Tile* bookshelf;
    static Tile* mossStone;
    static Tile* obsidian;
    static Tile* torch;
    static FireTile* fire;
    static Tile* mobSpawner;
    static Tile* stairs_wood;
    static Tile* chest;
    static Tile* redStoneDust;
    static Tile* emeraldOre;
    static Tile* emeraldBlock;
    static Tile* workBench;
	static Tile* stonecutterBench;
    static Tile* crops;
    static Tile* farmland;
    static Tile* furnace;
    static Tile* furnace_lit;
    static Tile* sign;
    static Tile* door_wood;
    static Tile* ladder;
    static Tile* rail;
    static Tile* stairs_stone;
    static Tile* wallSign;
    static Tile* lever;
    static Tile* pressurePlate_stone;
    static Tile* door_iron;
    static Tile* pressurePlate_wood;
    static Tile* redStoneOre;
    static Tile* redStoneOre_lit;
    static Tile* notGate_off;
    static Tile* notGate_on;
    static Tile* button;
    static Tile* topSnow;
    static Tile* ice;
    static Tile* snow;
    static Tile* cactus;
    static Tile* clay;
    static Tile* reeds;
    static Tile* recordPlayer;
    static Tile* fence;
	static Tile* stairs_brick;
	static Tile* fenceGate;
    static Tile* pumpkin;
    static Tile* hellRock;
    static Tile* hellSand;
    static Tile* lightGem;
	static Tile* portalTile;
    static Tile* litPumpkin;
    static Tile* cake;
    static Tile* diode_off;
    static Tile* diode_on;
	static Tile* trapdoor;
	static Tile* stoneBrickSmooth;
	static Tile* grass_carried;
	static LeafTile* leaves_carried;
	static Tile* melon;
	static Tile* melonStem;
	static Tile* netherReactor;
	static Tile* glowingObsidian;
	static Tile* netherPortal;
	static Tile* classicPortal;

	static Tile* stairs_stoneBrickSmooth;
	static Tile* netherBrick;
	static Tile* netherFence;
	static Tile* netherrack;
	static Tile* stairs_netherBricks;
	static Tile* stairs_sandStone;
	static Tile* quartzBlock;
	static Tile* stairs_quartz;

	static Tile* netherStalk;
	static Tile* netherWartBlock;
	static Tile* ancientDebris;
	static Tile* netherQuartzOre;
	static Tile* endStone;
	static Tile* soulSand;

	static Tile* deepslate;
	static Tile* cobbledDeepslate;
	static Tile* deepslateCoalOre;
	static Tile* deepslateIronOre;
	static Tile* deepslateGoldOre;
	static Tile* deepslateDiamondOre;
	static Tile* deepslateLapisOre;
	static Tile* deepslateRedstoneOre;
	static Tile* deepslateRedstoneOre_lit;
	static Tile* deepslatePolished;
	static Tile* deepslateTiles;
	static Tile* deepslateBricks;

	// Stones & Polished
	static Tile* granite;
	static Tile* graniteSmooth;
	static Tile* diorite;
	static Tile* dioriteSmooth;
	static Tile* andesite;
	static Tile* andesiteSmooth;

	// Soils & Organic
	static Tile* podzol;
	static Tile* mycelium;

	// Ices
	static Tile* packedIce;
	static Tile* blueIce;
	static Tile* frostedIce;

	// Terracotta & 16 Stained Clays
	static Tile* hardenedClay;
	static Tile* stainedClay_white;
	static Tile* stainedClay_orange;
	static Tile* stainedClay_magenta;
	static Tile* stainedClay_lightBlue;
	static Tile* stainedClay_yellow;
	static Tile* stainedClay_lime;
	static Tile* stainedClay_pink;
	static Tile* stainedClay_gray;
	static Tile* stainedClay_silver;
	static Tile* stainedClay_cyan;
	static Tile* stainedClay_purple;
	static Tile* stainedClay_blue;
	static Tile* stainedClay_brown;
	static Tile* stainedClay_green;
	static Tile* stainedClay_red;
	static Tile* stainedClay_black;

	// Huge Mushrooms
	static Tile* mushroomBlock_brown;
	static Tile* mushroomBlock_red;
	static Tile* mushroomBlock_stem;

	// Flora & Special
	static Tile* waterlily;
	static Tile* vines;
	static Tile* cocoa;
	static Tile* sweetBerryBush;

	// 1.7+ Flowers
	static Tile* flower_allium;
	static Tile* flower_blueOrchid;
	static Tile* flower_houstonia;
	static Tile* flower_tulipRed;
	static Tile* flower_tulipOrange;
	static Tile* flower_tulipWhite;
	static Tile* flower_tulipPink;
	static Tile* flower_oxeyeDaisy;
	static Tile* flower_paeonia;
	static Tile* flower_roseBlue;
	static Tile* flower_lilyOfTheValley;
	static Tile* flower_witherRose;

	// invisible bedrock is used to block off empty chunks (i.e. prevent player movement)
	static Tile* invisible_bedrock;
	static Tile* info_updateGame1;
	static Tile* info_updateGame2;
	static Tile* info_reserved6;

	// ============================================
	// Bedrock Generator / Structures Blocks (IDs 256..491)
	// ============================================
	static Tile* acaciaDoor;
	static Tile* acaciaPressurePlate;
	static Tile* acaciaWood;
	static Tile* barrel;
	static Tile* basalt;
	static Tile* bell;
	static Tile* blackCarpet;
	static Tile* blackGlazedTerracotta;
	static Tile* blackStainedGlass;
	static Tile* blackstone;
	static Tile* blackstoneSlab;
	static Tile* blackstoneStairs;
	static Tile* blackstoneWall;
	static Tile* blastFurnace;
	static Tile* blueCarpet;
	static Tile* boneBlock;
	static Tile* brewingStand;
	static Tile* brickWall;
	static Tile* brownCarpet;
	static Tile* brownStainedGlass;
	static Tile* campfire;
	static Tile* candle;
	static Tile* cartographyTable;
	static Tile* carvedPumpkin;
	static Tile* cauldron;
	static Tile* chiseledDeepslate;
	static Tile* chiseledPolishedBlackstone;
	static Tile* chiseledSandstone;
	static Tile* chiseledStoneBricks;
	static Tile* chiseledTuff;
	static Tile* chiseledTuffBricks;
	static Tile* cinnabar;
	static Tile* coalBlock;
	static Tile* coarseDirt;
	static Tile* cobbledDeepslateSlab;
	static Tile* cobbledDeepslateStairs;
	static Tile* cobbledDeepslateWall;
	static Tile* cobblestoneWall;
	static Tile* composter;
	static Tile* copperBlock;
	static Tile* crackedDeepslateBricks;
	static Tile* crackedDeepslateTiles;
	static Tile* crackedPolishedBlackstoneBricks;
	static Tile* crackedStoneBricks;
	static Tile* cutSandstone;
	static Tile* cyanCarpet;
	static Tile* cyanGlazedTerracotta;
	static Tile* damagedAnvil;
	static Tile* darkOakDoor;
	static Tile* darkOakLog;
	static Tile* deadbush;
	static Tile* decoratedPot;
	static Tile* deepslateBrickSlab;
	static Tile* deepslateBrickStairs;
	static Tile* deepslateBrickWall;
	static Tile* deepslateTileDoubleSlab;
	static Tile* deepslateTileSlab;
	static Tile* deepslateTileStairs;
	static Tile* deepslateTileWall;
	static Tile* dioriteDoubleSlab;
	static Tile* dioriteStairs;
	static Tile* dioriteWall;
	static Tile* dragonHead;
	static Tile* endBricks;
	static Tile* endRod;
	static Tile* enderChest;
	static Tile* fern;
	static Tile* fletchingTable;
	static Tile* flowerPot;
	static Tile* gildedBlackstone;
	static Tile* graniteStairs;
	static Tile* graniteWall;
	static Tile* grassPath;
	static Tile* grayCarpet;
	static Tile* greenCarpet;
	static Tile* grindstone;
	static Tile* hayBlock;
	static Tile* hopper;
	static Tile* infestedChiseledStoneBricks;
	static Tile* infestedCobblestone;
	static Tile* infestedMossyStoneBricks;
	static Tile* infestedStoneBricks;
	static Tile* ironBars;
	static Tile* ironChain;
	static Tile* ironTrapdoor;
	static Tile* jigsaw;
	static Tile* jungleButton;
	static Tile* jungleDoor;
	static Tile* jungleTrapdoor;
	static Tile* lantern;
	static Tile* largeFern;
	static Tile* lectern;
	static Tile* lightBlueCarpet;
	static Tile* lightBlueGlazedTerracotta;
	static Tile* lightGrayCarpet;
	static Tile* lightGrayStainedGlass;
	static Tile* limeCarpet;
	static Tile* limeGlazedTerracotta;
	static Tile* litRedstoneLamp;
	static Tile* loom;
	static Tile* magentaCarpet;
	static Tile* magentaStainedGlass;
	static Tile* magma;
	static Tile* mangroveLeaves;
	static Tile* mangroveLog;
	static Tile* mangroveRoots;
	static Tile* mangroveWood;
	static Tile* mossBlock;
	static Tile* mossCarpet;
	static Tile* mossyCobblestoneSlab;
	static Tile* mossyCobblestoneStairs;
	static Tile* mossyCobblestoneWall;
	static Tile* mossyStoneBricks;
	static Tile* mud;
	static Tile* mudBrickSlab;
	static Tile* mudBrickStairs;
	static Tile* mudBrickWall;
	static Tile* mudBricks;
	static Tile* muddyMangroveRoots;
	static Tile* noteblock;
	static Tile* orangeCarpet;
	static Tile* orangeGlazedTerracotta;
	static Tile* orangeStainedGlassPane;
	static Tile* oxidizedCopperTrapdoor;
	static Tile* oxidizedCutCopper;
	static Tile* packedMud;
	static Tile* pinkCarpet;
	static Tile* pointedDripstone;
	static Tile* polishedBasalt;
	static Tile* polishedBlackstoneBrickStairs;
	static Tile* polishedBlackstoneBricks;
	static Tile* polishedDeepslateSlab;
	static Tile* polishedDeepslateStairs;
	static Tile* polishedDeepslateWall;
	static Tile* polishedTuff;
	static Tile* polishedTuffSlab;
	static Tile* potentSulfur;
	static Tile* powderSnow;
	static Tile* poweredComparator;
	static Tile* poweredRepeater;
	static Tile* prismarine;
	static Tile* purpleCarpet;
	static Tile* purpleGlazedTerracotta;
	static Tile* purpurBlock;
	static Tile* purpurPillar;
	static Tile* purpurSlab;
	static Tile* purpurStairs;
	static Tile* redCandle;
	static Tile* redCarpet;
	static Tile* redConcrete;
	static Tile* redGlazedTerracotta;
	static Tile* redstoneBlock;
	static Tile* redstoneLamp;
	static Tile* redstoneTorch;
	static Tile* redstoneWire;
	static Tile* reinforcedDeepslate;
	static Tile* sandstoneWall;
	static Tile* sculkSensor;
	static Tile* seaLantern;
	static Tile* seaPickle;
	static Tile* silverGlazedTerracotta;
	static Tile* skeletonSkull;
	static Tile* smithingTable;
	static Tile* smoker;
	static Tile* smoothBasalt;
	static Tile* smoothQuartz;
	static Tile* smoothQuartzSlab;
	static Tile* smoothSandstone;
	static Tile* smoothSandstoneDoubleSlab;
	static Tile* smoothSandstoneSlab;
	static Tile* smoothSandstoneStairs;
	static Tile* smoothStone;
	static Tile* smoothStoneDoubleSlab;
	static Tile* smoothStoneSlab;
	static Tile* soulFire;
	static Tile* soulLantern;
	static Tile* spruceDoor;
	static Tile* sprucePressurePlate;
	static Tile* spruceTrapdoor;
	static Tile* spruceWallSign;
	static Tile* spruceWood;
	static Tile* stickyPiston;
	static Tile* stickyPistonArmCollision;
	static Tile* stoneBrickWall;
	static Tile* stoneButton;
	static Tile* stonePressurePlate;
	static Tile* strippedAcaciaLog;
	static Tile* strippedOakLog;
	static Tile* strippedOakWood;
	static Tile* strippedSpruceLog;
	static Tile* strippedSpruceWood;
	static Tile* structureBlock;
	static Tile* sulfur;
	static Tile* sulfurSpike;
	static Tile* tallGrass;
	static Tile* target;
	static Tile* trappedChest;
	static Tile* trialSpawner;
	static Tile* tripWire;
	static Tile* tripwireHook;
	static Tile* tuff;
	static Tile* tuffBricks;
	static Tile* unlitRedstoneTorch;
	static Tile* unpoweredComparator;
	static Tile* unpoweredRepeater;
	static Tile* vault;
	static Tile* wallBanner;
	static Tile* waxedChiseledCopper;
	static Tile* waxedCopper;
	static Tile* waxedCopperBulb;
	static Tile* waxedCopperDoor;
	static Tile* waxedCopperGrate;
	static Tile* waxedCutCopper;
	static Tile* waxedCutCopperSlab;
	static Tile* waxedCutCopperStairs;
	static Tile* waxedOxidizedChiseledCopper;
	static Tile* waxedOxidizedCopper;
	static Tile* waxedOxidizedCopperDoor;
	static Tile* waxedOxidizedCopperGrate;
	static Tile* waxedOxidizedCopperTrapdoor;
	static Tile* waxedOxidizedCutCopper;
	static Tile* waxedOxidizedCutCopperSlab;
	static Tile* waxedOxidizedCutCopperStairs;
	static Tile* whiteCandle;
	static Tile* whiteCarpet;
	static Tile* whiteConcrete;
	static Tile* whiteGlazedTerracotta;
	static Tile* whiteStainedGlass;
	static Tile* whiteStainedGlassPane;
	static Tile* woodenButton;
	static Tile* woodenPressurePlate;
	static Tile* yellowCarpet;
	static Tile* yellowGlazedTerracotta;
	static Tile* yellowStainedGlassPane;

	static void initTiles();
	static void teardownTiles();
    static void initExtraTiles();

	static bool isTileAllowedInCreative(int id);

	static int getOreVariant(int oreTileId, int replacedTileId);
	static int transformToValidBlockId(int blockId);
	static int transformToValidBlockId(int blockId, int x, int y, int z);

	Tile(int id, const Material* material);
    Tile(int id, int tex, const Material* material);
	virtual ~Tile() {}

    virtual bool isCubeShaped();
    virtual int getRenderShape();
    virtual void setShape(float x0, float y0, float z0, float x1, float y1, float z1);
	virtual void updateShape(LevelSource* level, int x, int y, int z) {}
	virtual void updateDefaultShape() {}

	virtual void addLights(Level* level, int x, int y, int z) {}

	virtual float getBrightness(LevelSource* level, int x, int y, int z);

    static bool isFaceVisible(Level* level, int x, int y, int z, int f);
    virtual bool shouldRenderFace(LevelSource* level, int x, int y, int z, int face);

	virtual int getTexture(int face);
	virtual int getTexture(int face, int data);
    virtual int getTexture(LevelSource* level, int x, int y, int z, int face);

	// @attn Not threadsafe (ADDON: nor safe to _save_ this returned AABB*.
	// Make a copy if you need to save this AABB (rather then using as a temp)
    virtual AABB* getAABB(Level* level, int x, int y, int z);
	virtual void addAABBs(Level* level, int x, int y, int z, const AABB* box, std::vector<AABB>& boxes);
	virtual AABB getTileAABB(Level* level, int x, int y, int z);

    virtual bool isSolidRender();

	virtual bool mayPick();
    virtual bool mayPick(int data, bool liquid);
	virtual bool mayPlace(Level* level, int x, int y, int z, unsigned char face);
	virtual bool mayPlace(Level* level, int x, int y, int z);

	virtual int getTickDelay();
    virtual void tick(Level* level, int x, int y, int z, Random* random) {}
    virtual void animateTick(Level* level, int x, int y, int z, Random* random) {}

    virtual void destroy(Level* level, int x, int y, int z, int data);

    virtual void neighborChanged(Level* level, int x, int y, int z, int type) {}

    virtual void onPlace(Level* level, int x, int y, int z) {}
    virtual void onRemove(Level* level, int x, int y, int z) {}

	virtual int getResource(int data, Random* random);
    virtual int getResourceCount(Random* random);

    virtual float getDestroyProgress(Player* player);

    virtual void spawnResources(Level* level, int x, int y, int z, int data);
    virtual void spawnResources(Level* level, int x, int y, int z, int data, float odds);
	virtual bool spawnBurnResources(Level* level, float x, float y, float z);
	void popResource(Level* level, int x, int y, int z, const ItemInstance& itemInstance);

    virtual float getExplosionResistance(Entity* source);

    virtual HitResult clip(Level* level, int xt, int yt, int zt, const Vec3& a, const Vec3& b);

	virtual void wasExploded(Level* level, int x, int y, int z) {}

    virtual int getRenderLayer();

    virtual bool use(Level* level, int x, int y, int z, Player* player);

    virtual void stepOn(Level* level, int x, int y, int z, Entity* entity) {}

	virtual void fallOn( Level* level, int x, int y, int z, Entity* entity, float fallDistance ) {}

	virtual int getPlacedOnFaceDataValue(Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue) { return itemValue; }
	virtual void setPlacedBy(Level* level, int x, int y, int z, Mob* by) {}

    virtual void prepareRender(Level* level, int x, int y, int z) {}

    virtual void attack(Level* level, int x, int y, int z, Player* player) {}

    virtual void handleEntityInside(Level* level, int x, int y, int z, Entity* e, Vec3& current) {}

    virtual int getColor(LevelSource* level, int x, int y, int z);

	virtual bool isSignalSource();
    virtual bool getSignal(LevelSource* level, int x, int y, int z);
    virtual bool getSignal(LevelSource* level, int x, int y, int z, int dir);
	virtual bool getDirectSignal(Level* level, int x, int y, int z, int dir);

    virtual void entityInside(Level* level, int x, int y, int z, Entity* entity) {}

    virtual void playerDestroy(Level* level, Player* player, int x, int y, int z, int data);

    virtual bool canSurvive(Level* level, int x, int y, int z);


    virtual std::string getName() const;
    virtual std::string getDescriptionId() const;
	virtual Tile* setDescriptionId(const std::string& id);

    virtual void triggerEvent(Level* level, int x, int y, int z, int b0, int b1) {}

	// Métodos para configuración de material_instances
	Tile* setFaceTexture(BlockFace face, const std::string& textureName) {
		int defaultIndex = this->getTexture((int)face, 0);
		materialInstances[face] = MaterialInstance(textureName, defaultIndex);
		useMaterialInstances = true;
		return this;
	}
	
	Tile* setFaceTexture(BlockFace face, const std::string& textureName, int atlasIndex) {
		materialInstances[face] = MaterialInstance(textureName, atlasIndex);
		useMaterialInstances = true;
		return this;
	}
	
	Tile* setFaceTexture(BlockFace face, int atlasIndex) {
		materialInstances[face] = MaterialInstance("", atlasIndex);
		useMaterialInstances = true;
		return this;
	}
	
	Tile* setSideTexture(const std::string& textureName, int atlasIndex = -1) {
		if (atlasIndex >= 0) {
			setFaceTexture(FACE_NORTH, textureName, atlasIndex);
			setFaceTexture(FACE_SOUTH, textureName, atlasIndex);
			setFaceTexture(FACE_WEST, textureName, atlasIndex);
			setFaceTexture(FACE_EAST, textureName, atlasIndex);
		} else {
			setFaceTexture(FACE_NORTH, textureName);
			setFaceTexture(FACE_SOUTH, textureName);
			setFaceTexture(FACE_WEST, textureName);
			setFaceTexture(FACE_EAST, textureName);
		}
		return this;
	}
	
	Tile* setTopTexture(const std::string& textureName, int atlasIndex = -1) {
		return atlasIndex >= 0 ? setFaceTexture(FACE_UP, textureName, atlasIndex) : setFaceTexture(FACE_UP, textureName);
	}
	
	Tile* setBottomTexture(const std::string& textureName, int atlasIndex = -1) {
		return atlasIndex >= 0 ? setFaceTexture(FACE_DOWN, textureName, atlasIndex) : setFaceTexture(FACE_DOWN, textureName);
	}
	
	Tile* setSimplifiedTextures(const std::string& side, const std::string& top, const std::string& bottom) {
		setSideTexture(side);
		setTopTexture(top);
		setBottomTexture(bottom);
		return this;
	}
	
	Tile* setAllFacesTexture(const std::string& textureName, int atlasIndex = -1) {
		for (int i = 0; i <= 5; i++) {
			if (atlasIndex >= 0) {
				setFaceTexture((BlockFace)i, textureName, atlasIndex);
			} else {
				setFaceTexture((BlockFace)i, textureName);
			}
		}
		return this;
	}
	
	Tile* setAllFacesTexture(int atlasIndex) {
		for (int i = 0; i <= 5; i++) {
			setFaceTexture((BlockFace)i, atlasIndex);
		}
		return this;
	}
	
	const MaterialInstance* getMaterialInstance(BlockFace face) const {
		auto it = materialInstances.find(face);
		if (it != materialInstances.end()) {
			return &it->second;
		}
		return nullptr;
	}
	
	bool hasMaterialInstances() const {
		return useMaterialInstances;
	}
	
	static BlockFace renderFaceToBlockFace(int renderFace) {
		switch(renderFace) {
			case 0: return FACE_DOWN;
			case 1: return FACE_UP;
			case 2: return FACE_NORTH;
			case 3: return FACE_SOUTH;
			case 4: return FACE_WEST;
			case 5: return FACE_EAST;
			default: return FACE_UP;
		}
	}

protected:
	std::map<BlockFace, MaterialInstance> materialInstances;
	bool useMaterialInstances;

public:
	Tile* init();
	Tile* setCategory(int category);
    virtual Tile* setSoundType(const SoundType& soundType);
    virtual Tile* setLightBlock(int i);
    virtual Tile* setLightEmission(float f);
    virtual Tile* setExplodeable(float explosionResistance);
    virtual Tile* setDestroyTime(float destroySpeed);
    virtual void setTicking(bool tick);

protected:
	/*** Returns the item instance's auxValue when a TileItem is spawned from this Tile. */
	virtual int getSpawnResourcesAuxValue(int data);

private:
	bool containsX(const Vec3& v);
	bool containsY(const Vec3& v);
	bool containsZ(const Vec3& v);
public:
	int tex;
	const int id;

	float xx0, yy0, zz0, xx1, yy1, zz1;
	const SoundType* soundType;

	float gravity;
	const Material* const material;
	float friction;

//protected:
	float destroySpeed;
	float explosionResistance;

	int category;
protected:
	AABB tmpBB;

    static const int RENDERLAYER_OPAQUE;
    static const int RENDERLAYER_ALPHATEST;
    static const int RENDERLAYER_BLEND;
private:
	std::string descriptionId;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__Tile_H__*/
