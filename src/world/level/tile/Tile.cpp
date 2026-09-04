#include "TileInclude.h"
#include "ExtraTiles.h"
#include "FlowerTile.h"
#include "DeepslateTile.h"
#include "NetherQuartzOreTile.h"
#include "SoulSandTile.h"
// AncientDebrisTile removed
#include "NetherPortalTile.h"
#include "ClassicPortalTile.h"
#include "entity/NetherReactorTileEntity.h"
#include "../Level.h"
#include "../../entity/player/Player.h"
#include "../../entity/item/ItemEntity.h"
#include "../../item/Item.h"
#include "../../item/TileItem.h"

#include "../../../util/Random.h"
//#include "locale/Descriptive.h"
//#include "stats/Stats.h"
#include "../../entity/Entity.h"
#include "../LevelSource.h"
#include "../material/Material.h"
#include "../../phys/AABB.h"
#include "../../phys/HitResult.h"
#include "../../phys/Vec3.h"
#include "../../../locale/I18n.h"
#include "../../item/ClothTileItem.h"

#include "../../item/AuxDataTileItem.h"
#include "../../item/LeafTileItem.h"
#include "../../item/StoneSlabTileItem.h"
#include "../../item/SaplingTileItem.h"
#include "../../item/ItemCategory.h"
#include "../../../mods/ModRegistry.h"
#include <map>

const int Tile::RENDERLAYER_OPAQUE          = 0;
const int Tile::RENDERLAYER_ALPHATEST       = 1;
const int Tile::RENDERLAYER_BLEND           = 2;

const std::string Tile::TILE_DESCRIPTION_PREFIX("tile.");

const Tile::SoundType Tile::SOUND_NORMAL("stone", 1, 1);
const Tile::SoundType Tile::SOUND_WOOD("wood", 1, 1);
const Tile::SoundType Tile::SOUND_GRAVEL("gravel", 1, 1);
const Tile::SoundType Tile::SOUND_GRASS("grass", 0.5f, 1);
const Tile::SoundType Tile::SOUND_STONE("stone", 1, 1);
const Tile::SoundType Tile::SOUND_METAL("stone", 1, 1.5f);
const Tile::SoundType Tile::SOUND_GLASS("stone", "random.glass", 1, 1);
const Tile::SoundType Tile::SOUND_CLOTH("cloth", 1, 1);

#ifdef PRE_ANDROID23
	const Tile::SoundType Tile::SOUND_SAND("sand", 0.45f, 1);
#else
	const Tile::SoundType Tile::SOUND_SAND("sand", "step.gravel", 1, 1);
#endif

const Tile::SoundType Tile::SOUND_SILENT("", 0, 0);

Tile* Tile::tiles[NUM_BLOCK_TYPES] = {NULL};
int Tile::lightBlock[NUM_BLOCK_TYPES] = {0};
int Tile::lightEmission[NUM_BLOCK_TYPES] = {0};
bool Tile::solid[NUM_BLOCK_TYPES] = {false};
bool Tile::isEntityTile[NUM_BLOCK_TYPES] = {false};
bool Tile::translucent[NUM_BLOCK_TYPES] = {true}; 
bool Tile::shouldTick[NUM_BLOCK_TYPES] = {false};
bool Tile::sendTileData[NUM_BLOCK_TYPES] = {false};

Tile* Tile::sand        = NULL;
Tile* Tile::sandStone   = NULL;
const int SANDSTONE_TEXTURES[] = { 0 + 16 * 12, 5 + 16 * 14, 6 + 16 * 14 };
const int SANDSTONE_TEXTURE_COUNT = 3;

Tile* Tile::stoneBrick  = NULL;
Tile* Tile::redBrick    = NULL;
Tile* Tile::wood        = NULL;
Tile* Tile::birchPlanks = NULL;
Tile* Tile::sprucePlanks = NULL;
Tile* Tile::junglePlanks = NULL;
Tile* Tile::acaciaPlanks = NULL;
Tile* Tile::darkOakPlanks = NULL;
Tile* Tile::spruceSlab  = NULL;
Tile* Tile::spruceSlabHalf = NULL;
Tile* Tile::birchSlab   = NULL;
Tile* Tile::birchSlabHalf = NULL;
Tile* Tile::stairs_spruce = NULL;
Tile* Tile::stairs_birch = NULL;
Tile* Tile::fenceBirch    = NULL;
Tile* Tile::fenceSpruce   = NULL;
Tile* Tile::fenceGateBirch = NULL;
Tile* Tile::fenceGateSpruce = NULL;
Tile* Tile::stairs_jungle = NULL;
Tile* Tile::jungleSlab = NULL;
Tile* Tile::jungleSlabHalf = NULL;
Tile* Tile::fenceJungle = NULL;
Tile* Tile::fenceGateJungle = NULL;
Tile* Tile::stairs_acacia = NULL;
Tile* Tile::acaciaSlab = NULL;
Tile* Tile::acaciaSlabHalf = NULL;
Tile* Tile::fenceAcacia = NULL;
Tile* Tile::fenceGateAcacia = NULL;
Tile* Tile::stairs_darkOak = NULL;
Tile* Tile::darkOakSlab = NULL;
Tile* Tile::darkOakSlabHalf = NULL;
Tile* Tile::fenceDarkOak = NULL;
Tile* Tile::fenceGateDarkOak = NULL;
Tile* Tile::sandstoneSlab = NULL;
Tile* Tile::sandstoneSlabHalf = NULL;
Tile* Tile::oakSlab = NULL;
Tile* Tile::oakSlabHalf = NULL;
Tile* Tile::cobbleSlab = NULL;
Tile* Tile::cobbleSlabHalf = NULL;
Tile* Tile::brickSlab = NULL;
Tile* Tile::brickSlabHalf = NULL;
Tile* Tile::stoneBrickSlab = NULL;
Tile* Tile::stoneBrickSlabHalf = NULL;
Tile* Tile::netherBrickSlab = NULL;
Tile* Tile::netherBrickSlabHalf = NULL;
Tile* Tile::quartzSlab = NULL;
Tile* Tile::quartzSlabHalf = NULL;
Tile* Tile::sapling     = NULL;
Tile* Tile::spruceSapling = NULL;
Tile* Tile::birchSapling = NULL;
Tile* Tile::jungleSapling = NULL;
Tile* Tile::acaciaSapling = NULL;
Tile* Tile::darkOakSapling = NULL;
Tile* Tile::glass       = NULL;
Tile* Tile::web	        = NULL;
Tile* Tile::thinGlass   = NULL;
Tile* Tile::calmWater   = NULL;
Tile* Tile::calmLava    = NULL;
Tile* Tile::gravel      = NULL;
Tile* Tile::rock        = NULL;
Tile* Tile::unbreakable = NULL;
Tile* Tile::dirt        = NULL;
Tile* Tile::grass       = NULL;
Tile* Tile::ice         = NULL;
Tile* Tile::clay        = NULL;
Tile* Tile::farmland    = NULL;
Tile* Tile::stoneSlab   = NULL;
Tile* Tile::stoneSlabHalf=NULL;
Tile* Tile::cloth       = NULL;
Tile* Tile::flower      = NULL;
Tile* Tile::rose        = NULL;
Tile* Tile::dandelion   = nullptr;
Tile* Tile::cornflower  = nullptr;
Tile* Tile::mushroom1   = NULL;
Tile* Tile::mushroom2   = NULL;
Tile* Tile::topSnow     = NULL;
Tile* Tile::treeTrunk   = NULL;
Tile* Tile::birchTrunk  = NULL;
Tile* Tile::spruceTrunk = NULL;
Tile* Tile::jungleTrunk = NULL;
Tile* Tile::acaciaTrunk = NULL;
Tile* Tile::snow        = NULL;
LeafTile* Tile::leaves  = NULL;
LeafTile* Tile::spruceLeaves  = NULL;
LeafTile* Tile::birchLeaves   = NULL;
LeafTile* Tile::jungleLeaves  = NULL;
LeafTile* Tile::acaciaLeaves  = NULL;
LeafTile* Tile::darkOakLeaves = NULL;
Tile* Tile::emeraldOre  = NULL;
Tile* Tile::redStoneOre = NULL;
Tile* Tile::redStoneOre_lit = NULL;
Tile* Tile::goldOre     = NULL;
Tile* Tile::ironOre     = NULL;
Tile* Tile::coalOre     = NULL;
Tile* Tile::lapisOre    = NULL;
Tile* Tile::lapisBlock	= NULL;
Tile* Tile::reeds       = NULL;
Tile* Tile::ladder      = NULL;
Tile* Tile::obsidian    = NULL;
Tile* Tile::tnt         = NULL;
Tile* Tile::bookshelf	= NULL;
Tile* Tile::sign        = NULL;
Tile* Tile::wallSign    = NULL;
Tile* Tile::mossStone	= NULL;
Tile* Tile::torch       = NULL;
Tile* Tile::water       = NULL;
Tile* Tile::lava        = NULL;
FireTile* Tile::fire    = NULL;
Tile* Tile::invisible_bedrock = NULL;
Tile* Tile::goldBlock   = NULL;
Tile* Tile::ironBlock   = NULL;
Tile* Tile::emeraldBlock= NULL;
Tile* Tile::workBench   = NULL;
Tile* Tile::stonecutterBench = NULL;
Tile* Tile::crops		= NULL;
Tile* Tile::furnace     = NULL;
Tile* Tile::furnace_lit = NULL;
Tile* Tile::chest		= NULL;
Tile* Tile::lightGem    = NULL;
Tile* Tile::stairs_wood = NULL;
Tile* Tile::stairs_stone= NULL;
Tile* Tile::stairs_brick= NULL;
Tile* Tile::door_wood   = NULL;
Tile* Tile::door_iron   = NULL;
Tile* Tile::cactus      = NULL;

Tile* Tile::melon       = NULL;
Tile* Tile::melonStem   = NULL;

Tile* Tile::bed         = NULL;
Tile* Tile::tallgrass   = NULL;
Tile* Tile::trapdoor    = NULL;
Tile* Tile::stoneBrickSmooth = NULL;
const int STONE_BRICK_TEXTURES[] = { 6 + 16 * 3, 4 + 16 * 6, 5 + 16 * 6 };
const int STONE_BRICK_TEXTURE_COUNT = 3;

Tile* Tile::fence       = NULL;
Tile* Tile::fenceGate   = NULL;

Tile* Tile::info_updateGame1 = NULL;
Tile* Tile::info_updateGame2 = NULL;
Tile* Tile::info_reserved6   = NULL;
Tile* Tile::grass_carried    = NULL;
LeafTile* Tile::leaves_carried   = NULL;

Tile* Tile::glowingObsidian = NULL;
Tile* Tile::netherReactor = NULL;
Tile* Tile::netherPortal = NULL;
Tile* Tile::classicPortal = NULL;

Tile* Tile::stairs_stoneBrickSmooth   = NULL;
Tile* Tile::netherBrick   = NULL;
Tile* Tile::netherFence   = NULL;
Tile* Tile::netherrack   = NULL;
Tile* Tile::stairs_netherBricks   = NULL;
Tile* Tile::stairs_sandStone   = NULL;
Tile* Tile::quartzBlock   = NULL;
Tile* Tile::stairs_quartz   = NULL;

Tile* Tile::netherStalk = NULL;
Tile* Tile::netherWartBlock = NULL;
Tile* Tile::ancientDebris = NULL;
Tile* Tile::netherQuartzOre = NULL;
Tile* Tile::endStone = NULL;
Tile* Tile::soulSand = NULL;

Tile* Tile::deepslate = NULL;
Tile* Tile::cobbledDeepslate = NULL;
Tile* Tile::deepslateCoalOre = NULL;
Tile* Tile::deepslateIronOre = NULL;
Tile* Tile::deepslateGoldOre = NULL;
Tile* Tile::deepslateDiamondOre = NULL;
Tile* Tile::deepslateLapisOre = NULL;
Tile* Tile::deepslateRedstoneOre = NULL;
Tile* Tile::deepslateRedstoneOre_lit = NULL;
Tile* Tile::deepslatePolished = NULL;
Tile* Tile::deepslateTiles = NULL;
Tile* Tile::deepslateBricks = NULL;

Tile* Tile::granite = NULL;
Tile* Tile::graniteSmooth = NULL;
Tile* Tile::diorite = NULL;
Tile* Tile::dioriteSmooth = NULL;
Tile* Tile::andesite = NULL;
Tile* Tile::andesiteSmooth = NULL;

Tile* Tile::podzol = NULL;
Tile* Tile::mycelium = NULL;

Tile* Tile::packedIce = NULL;
Tile* Tile::blueIce = NULL;
Tile* Tile::frostedIce = NULL;

Tile* Tile::hardenedClay = NULL;
Tile* Tile::stainedClay_white = NULL;
Tile* Tile::stainedClay_orange = NULL;
Tile* Tile::stainedClay_magenta = NULL;
Tile* Tile::stainedClay_lightBlue = NULL;
Tile* Tile::stainedClay_yellow = NULL;
Tile* Tile::stainedClay_lime = NULL;
Tile* Tile::stainedClay_pink = NULL;
Tile* Tile::stainedClay_gray = NULL;
Tile* Tile::stainedClay_silver = NULL;
Tile* Tile::stainedClay_cyan = NULL;
Tile* Tile::stainedClay_purple = NULL;
Tile* Tile::stainedClay_blue = NULL;
Tile* Tile::stainedClay_brown = NULL;
Tile* Tile::stainedClay_green = NULL;
Tile* Tile::stainedClay_red = NULL;
Tile* Tile::stainedClay_black = NULL;

Tile* Tile::mushroomBlock_brown = NULL;
Tile* Tile::mushroomBlock_red = NULL;
Tile* Tile::mushroomBlock_stem = NULL;

Tile* Tile::waterlily = NULL;
Tile* Tile::vines = NULL;
Tile* Tile::cocoa = NULL;
Tile* Tile::sweetBerryBush = NULL;

Tile* Tile::flower_allium = NULL;
Tile* Tile::flower_blueOrchid = NULL;
Tile* Tile::flower_houstonia = NULL;
Tile* Tile::flower_tulipRed = NULL;
Tile* Tile::flower_tulipOrange = NULL;
Tile* Tile::flower_tulipWhite = NULL;
Tile* Tile::flower_tulipPink = NULL;
Tile* Tile::flower_oxeyeDaisy = NULL;
Tile* Tile::flower_paeonia = NULL;
Tile* Tile::flower_roseBlue = NULL;
Tile* Tile::flower_lilyOfTheValley = NULL;
Tile* Tile::flower_witherRose = NULL;

// ============================================
// Bedrock Generator / Structures Block Pointers (IDs 256..491)
// ============================================
Tile* Tile::acaciaDoor = NULL;
Tile* Tile::acaciaPressurePlate = NULL;
Tile* Tile::acaciaWood = NULL;
Tile* Tile::barrel = NULL;
Tile* Tile::basalt = NULL;
Tile* Tile::bell = NULL;
Tile* Tile::blackCarpet = NULL;
Tile* Tile::blackGlazedTerracotta = NULL;
Tile* Tile::blackStainedGlass = NULL;
Tile* Tile::blackstone = NULL;
Tile* Tile::blackstoneSlab = NULL;
Tile* Tile::blackstoneStairs = NULL;
Tile* Tile::blackstoneWall = NULL;
Tile* Tile::blastFurnace = NULL;
Tile* Tile::blueCarpet = NULL;
Tile* Tile::boneBlock = NULL;
Tile* Tile::brewingStand = NULL;
Tile* Tile::brickWall = NULL;
Tile* Tile::brownCarpet = NULL;
Tile* Tile::brownStainedGlass = NULL;
Tile* Tile::campfire = NULL;
Tile* Tile::candle = NULL;
Tile* Tile::cartographyTable = NULL;
Tile* Tile::carvedPumpkin = NULL;
Tile* Tile::cauldron = NULL;
Tile* Tile::chiseledDeepslate = NULL;
Tile* Tile::chiseledPolishedBlackstone = NULL;
Tile* Tile::chiseledSandstone = NULL;
Tile* Tile::chiseledStoneBricks = NULL;
Tile* Tile::chiseledTuff = NULL;
Tile* Tile::chiseledTuffBricks = NULL;
Tile* Tile::cinnabar = NULL;
Tile* Tile::coalBlock = NULL;
Tile* Tile::coarseDirt = NULL;
Tile* Tile::cobbledDeepslateSlab = NULL;
Tile* Tile::cobbledDeepslateStairs = NULL;
Tile* Tile::cobbledDeepslateWall = NULL;
Tile* Tile::cobblestoneWall = NULL;
Tile* Tile::composter = NULL;
Tile* Tile::copperBlock = NULL;
Tile* Tile::crackedDeepslateBricks = NULL;
Tile* Tile::crackedDeepslateTiles = NULL;
Tile* Tile::crackedPolishedBlackstoneBricks = NULL;
Tile* Tile::crackedStoneBricks = NULL;
Tile* Tile::cutSandstone = NULL;
Tile* Tile::cyanCarpet = NULL;
Tile* Tile::cyanGlazedTerracotta = NULL;
Tile* Tile::damagedAnvil = NULL;
Tile* Tile::darkOakDoor = NULL;
Tile* Tile::darkOakLog = NULL;
Tile* Tile::deadbush = NULL;
Tile* Tile::decoratedPot = NULL;
Tile* Tile::deepslateBrickSlab = NULL;
Tile* Tile::deepslateBrickStairs = NULL;
Tile* Tile::deepslateBrickWall = NULL;
Tile* Tile::deepslateTileDoubleSlab = NULL;
Tile* Tile::deepslateTileSlab = NULL;
Tile* Tile::deepslateTileStairs = NULL;
Tile* Tile::deepslateTileWall = NULL;
Tile* Tile::dioriteDoubleSlab = NULL;
Tile* Tile::dioriteStairs = NULL;
Tile* Tile::dioriteWall = NULL;
Tile* Tile::dragonHead = NULL;
Tile* Tile::endBricks = NULL;
Tile* Tile::endRod = NULL;
Tile* Tile::enderChest = NULL;
Tile* Tile::fern = NULL;
Tile* Tile::fletchingTable = NULL;
Tile* Tile::flowerPot = NULL;
Tile* Tile::gildedBlackstone = NULL;
Tile* Tile::graniteStairs = NULL;
Tile* Tile::graniteWall = NULL;
Tile* Tile::grassPath = NULL;
Tile* Tile::grayCarpet = NULL;
Tile* Tile::greenCarpet = NULL;
Tile* Tile::grindstone = NULL;
Tile* Tile::hayBlock = NULL;
Tile* Tile::hopper = NULL;
Tile* Tile::infestedChiseledStoneBricks = NULL;
Tile* Tile::infestedCobblestone = NULL;
Tile* Tile::infestedMossyStoneBricks = NULL;
Tile* Tile::infestedStoneBricks = NULL;
Tile* Tile::ironBars = NULL;
Tile* Tile::ironChain = NULL;
Tile* Tile::ironTrapdoor = NULL;
Tile* Tile::jigsaw = NULL;
Tile* Tile::jungleButton = NULL;
Tile* Tile::jungleDoor = NULL;
Tile* Tile::jungleTrapdoor = NULL;
Tile* Tile::lantern = NULL;
Tile* Tile::largeFern = NULL;
Tile* Tile::lectern = NULL;
Tile* Tile::lightBlueCarpet = NULL;
Tile* Tile::lightBlueGlazedTerracotta = NULL;
Tile* Tile::lightGrayCarpet = NULL;
Tile* Tile::lightGrayStainedGlass = NULL;
Tile* Tile::limeCarpet = NULL;
Tile* Tile::limeGlazedTerracotta = NULL;
Tile* Tile::litRedstoneLamp = NULL;
Tile* Tile::loom = NULL;
Tile* Tile::magentaCarpet = NULL;
Tile* Tile::magentaStainedGlass = NULL;
Tile* Tile::magma = NULL;
Tile* Tile::mangroveLeaves = NULL;
Tile* Tile::mangroveLog = NULL;
Tile* Tile::mangroveRoots = NULL;
Tile* Tile::mangroveWood = NULL;
Tile* Tile::mossBlock = NULL;
Tile* Tile::mossCarpet = NULL;
Tile* Tile::mossyCobblestoneSlab = NULL;
Tile* Tile::mossyCobblestoneStairs = NULL;
Tile* Tile::mossyCobblestoneWall = NULL;
Tile* Tile::mossyStoneBricks = NULL;
Tile* Tile::mud = NULL;
Tile* Tile::mudBrickSlab = NULL;
Tile* Tile::mudBrickStairs = NULL;
Tile* Tile::mudBrickWall = NULL;
Tile* Tile::mudBricks = NULL;
Tile* Tile::muddyMangroveRoots = NULL;
Tile* Tile::noteblock = NULL;
Tile* Tile::orangeCarpet = NULL;
Tile* Tile::orangeGlazedTerracotta = NULL;
Tile* Tile::orangeStainedGlassPane = NULL;
Tile* Tile::oxidizedCopperTrapdoor = NULL;
Tile* Tile::oxidizedCutCopper = NULL;
Tile* Tile::packedMud = NULL;
Tile* Tile::pinkCarpet = NULL;
Tile* Tile::pointedDripstone = NULL;
Tile* Tile::polishedBasalt = NULL;
Tile* Tile::polishedBlackstoneBrickStairs = NULL;
Tile* Tile::polishedBlackstoneBricks = NULL;
Tile* Tile::polishedDeepslateSlab = NULL;
Tile* Tile::polishedDeepslateStairs = NULL;
Tile* Tile::polishedDeepslateWall = NULL;
Tile* Tile::polishedTuff = NULL;
Tile* Tile::polishedTuffSlab = NULL;
Tile* Tile::potentSulfur = NULL;
Tile* Tile::powderSnow = NULL;
Tile* Tile::poweredComparator = NULL;
Tile* Tile::poweredRepeater = NULL;
Tile* Tile::prismarine = NULL;
Tile* Tile::purpleCarpet = NULL;
Tile* Tile::purpleGlazedTerracotta = NULL;
Tile* Tile::purpurBlock = NULL;
Tile* Tile::purpurPillar = NULL;
Tile* Tile::purpurSlab = NULL;
Tile* Tile::purpurStairs = NULL;
Tile* Tile::redCandle = NULL;
Tile* Tile::redCarpet = NULL;
Tile* Tile::redConcrete = NULL;
Tile* Tile::redGlazedTerracotta = NULL;
Tile* Tile::redstoneBlock = NULL;
Tile* Tile::redstoneLamp = NULL;
Tile* Tile::redstoneTorch = NULL;
Tile* Tile::redstoneWire = NULL;
Tile* Tile::reinforcedDeepslate = NULL;
Tile* Tile::sandstoneWall = NULL;
Tile* Tile::sculkSensor = NULL;
Tile* Tile::seaLantern = NULL;
Tile* Tile::seaPickle = NULL;
Tile* Tile::silverGlazedTerracotta = NULL;
Tile* Tile::skeletonSkull = NULL;
Tile* Tile::smithingTable = NULL;
Tile* Tile::smoker = NULL;
Tile* Tile::smoothBasalt = NULL;
Tile* Tile::smoothQuartz = NULL;
Tile* Tile::smoothQuartzSlab = NULL;
Tile* Tile::smoothSandstone = NULL;
Tile* Tile::smoothSandstoneDoubleSlab = NULL;
Tile* Tile::smoothSandstoneSlab = NULL;
Tile* Tile::smoothSandstoneStairs = NULL;
Tile* Tile::smoothStone = NULL;
Tile* Tile::smoothStoneDoubleSlab = NULL;
Tile* Tile::smoothStoneSlab = NULL;
Tile* Tile::soulFire = NULL;
Tile* Tile::soulLantern = NULL;
Tile* Tile::spruceDoor = NULL;
Tile* Tile::sprucePressurePlate = NULL;
Tile* Tile::spruceTrapdoor = NULL;
Tile* Tile::spruceWallSign = NULL;
Tile* Tile::spruceWood = NULL;
Tile* Tile::stickyPiston = NULL;
Tile* Tile::stickyPistonArmCollision = NULL;
Tile* Tile::stoneBrickWall = NULL;
Tile* Tile::stoneButton = NULL;
Tile* Tile::stonePressurePlate = NULL;
Tile* Tile::strippedAcaciaLog = NULL;
Tile* Tile::strippedOakLog = NULL;
Tile* Tile::strippedOakWood = NULL;
Tile* Tile::strippedSpruceLog = NULL;
Tile* Tile::strippedSpruceWood = NULL;
Tile* Tile::structureBlock = NULL;
Tile* Tile::sulfur = NULL;
Tile* Tile::sulfurSpike = NULL;
Tile* Tile::tallGrass = NULL;
Tile* Tile::target = NULL;
Tile* Tile::trappedChest = NULL;
Tile* Tile::trialSpawner = NULL;
Tile* Tile::tripWire = NULL;
Tile* Tile::tripwireHook = NULL;
Tile* Tile::tuff = NULL;
Tile* Tile::tuffBricks = NULL;
Tile* Tile::unlitRedstoneTorch = NULL;
Tile* Tile::unpoweredComparator = NULL;
Tile* Tile::unpoweredRepeater = NULL;
Tile* Tile::vault = NULL;
Tile* Tile::wallBanner = NULL;
Tile* Tile::waxedChiseledCopper = NULL;
Tile* Tile::waxedCopper = NULL;
Tile* Tile::waxedCopperBulb = NULL;
Tile* Tile::waxedCopperDoor = NULL;
Tile* Tile::waxedCopperGrate = NULL;
Tile* Tile::waxedCutCopper = NULL;
Tile* Tile::waxedCutCopperSlab = NULL;
Tile* Tile::waxedCutCopperStairs = NULL;
Tile* Tile::waxedOxidizedChiseledCopper = NULL;
Tile* Tile::waxedOxidizedCopper = NULL;
Tile* Tile::waxedOxidizedCopperDoor = NULL;
Tile* Tile::waxedOxidizedCopperGrate = NULL;
Tile* Tile::waxedOxidizedCopperTrapdoor = NULL;
Tile* Tile::waxedOxidizedCutCopper = NULL;
Tile* Tile::waxedOxidizedCutCopperSlab = NULL;
Tile* Tile::waxedOxidizedCutCopperStairs = NULL;
Tile* Tile::whiteCandle = NULL;
Tile* Tile::whiteCarpet = NULL;
Tile* Tile::whiteConcrete = NULL;
Tile* Tile::whiteGlazedTerracotta = NULL;
Tile* Tile::whiteStainedGlass = NULL;
Tile* Tile::whiteStainedGlassPane = NULL;
Tile* Tile::woodenButton = NULL;
Tile* Tile::woodenPressurePlate = NULL;
Tile* Tile::yellowCarpet = NULL;
Tile* Tile::yellowGlazedTerracotta = NULL;
Tile* Tile::yellowStainedGlassPane = NULL;

/*static*/
void Tile::initTiles() {
	rock        = (new StoneTile(1, 1))->init()->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("stone")->setAllFacesTexture("stone");
	grass       = (GrassTile*) (new GrassTile(2))->init()->setDestroyTime(0.6f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("grass");
	dirt        = (new DirtTile(3, 2))->init()->setDestroyTime(0.5f)->setSoundType(SOUND_GRAVEL)->setCategory(ItemCategory::Decorations)->setDescriptionId("dirt")->setAllFacesTexture("dirt");
	stoneBrick  = (new Tile(4, 16, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stonebrick")->setAllFacesTexture("stone_brick");
	wood        = (new Tile(5, 4, Material::wood))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("wood")->setAllFacesTexture("planks_oak");
	sapling     = (new Sapling(6, 15))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("sapling")->setAllFacesTexture("sapling");
	unbreakable = (new Tile(7, 17, Material::stone))->init()->setDestroyTime(-1)->setExplodeable(6000000)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("bedrock");
	water       = (new LiquidTileDynamic(8, Material::water))->init()->setDestroyTime(100.0f)->setLightBlock(2)->setCategory(ItemCategory::Decorations)->setDescriptionId("water");
	calmWater   = (new LiquidTileStatic(9, Material::water))->init()->setDestroyTime(100.0f)->setLightBlock(2)->setCategory(ItemCategory::Decorations)->setDescriptionId("water");
	lava        = (new LiquidTileDynamic(10, Material::lava))->init()->setDestroyTime(00.0f)->setLightEmission(1.0f)->setLightBlock(255)->setCategory(ItemCategory::Decorations)->setDescriptionId("lava");
	calmLava    = (new LiquidTileStatic(11, Material::lava))->init()->setDestroyTime(100.0f)->setLightEmission(1.0f)->setLightBlock(255)->setCategory(ItemCategory::Decorations)->setDescriptionId("lava");
	sand        = (new HeavyTile(12, 18))->init()->setDestroyTime(0.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("sand")->setAllFacesTexture("sand");
	gravel      = (new GravelTile(13, 19))->init()->setDestroyTime(0.6f)->setSoundType(SOUND_GRAVEL)->setCategory(ItemCategory::Decorations)->setDescriptionId("gravel")->setAllFacesTexture("gravel");
	goldOre     = (new OreTile(14, 32))->init()->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("oreGold")->setAllFacesTexture("gold_ore");
	ironOre     = (new OreTile(15, 33))->init()->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("oreIron")->setAllFacesTexture("iron_ore");
	coalOre     = (new OreTile(16, 34))->init()->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("oreCoal")->setAllFacesTexture("coal_ore");
	treeTrunk   = (new TreeTile(17))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("logOak");
	birchTrunk  = (new TreeTile(19))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("logBirch");
	spruceTrunk = (new TreeTile(23))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("logSpruce");

	leaves      = (LeafTile*) (new LeafTile(18, 4 + 3 * 16))->init()->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("leaves");

	glass       = (new GlassTile(20, 49, Material::glass, false))->init()->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Structures)->setDescriptionId("glass")->setAllFacesTexture("glass");
	lapisOre    = (new OreTile(21, 10 * 16))->init()->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("oreLapis")->setAllFacesTexture("lapis_ore");
	lapisBlock	= (new Tile(22, 9 * 16, Material::stone))->init()->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("blockLapis")->setAllFacesTexture("lapis_block");
	sandStone   = (new SandStoneTile(24, (const int*)&SANDSTONE_TEXTURES, SANDSTONE_TEXTURE_COUNT))->init()->setSoundType(SOUND_STONE)->setDestroyTime(0.8f)->setCategory(ItemCategory::Structures)->setDescriptionId("sandStone");
	bed         = (new BedTile(26))->init()->setDestroyTime(0.2f)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("bed");
	web	        = (new WebTile(30, 11))->init()->setLightBlock(1)->setDestroyTime(4.0f)->setCategory(ItemCategory::Decorations)->setDescriptionId("web")->setAllFacesTexture("cobweb");
	tallgrass   = (new TallGrass(31, 2 * 16 + 7))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("tallgrass")->setAllFacesTexture("tall_grass");
	cloth       = (new ClothTile(35))->init()->setDestroyTime(0.8f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Structures)->setDescriptionId("cloth");

	dandelion   = (new FlowerTile(179, 13 & ~Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flower")->setAllFacesTexture("flower_dandelion");
	cornflower  = (new FlowerTile(180, 12 & ~Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("rose")->setAllFacesTexture("flower_cornflower");
	flower = dandelion;
	rose = cornflower;

	mushroom1   = (new Mushroom(39, 13 + 16))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setLightEmission(2 / 16.0f)->setCategory(ItemCategory::Decorations)->setDescriptionId("mushroom")->setAllFacesTexture("mushroom_brown");
	mushroom2   = (new Mushroom(40, 12 + 16))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mushroom")->setAllFacesTexture("mushroom_red");
	goldBlock   = (new MetalTile(41, 39 - 16))->init()->setDestroyTime(3.0f)->setExplodeable(10)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("blockGold")->setAllFacesTexture("gold_block");
	ironBlock   = (new MetalTile(42, 38 - 16))->init()->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("blockIron")->setAllFacesTexture("iron_block");
	stoneSlab   = (new StoneSlabTile(43, true))->init()->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stoneSlab");
	stoneSlabHalf=(new StoneSlabTile(44, false))->init()->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stoneSlab");
	redBrick    = (new Tile(45, 7, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("brick")->setAllFacesTexture("brick");
	tnt         = (new TntTile(46, 8))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("tnt")->setSimplifiedTextures("tnt", "tnt_top", "tnt_bottom");
	bookshelf	= (new BookshelfTile(47, 35))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("bookshelf")->setSimplifiedTextures("bookshelf_side", "planks_oak", "planks_oak");
	mossStone	= (new Tile(48, 36, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stoneMoss")->setAllFacesTexture("mossy_cobblestone");
	obsidian    = (new ObsidianTile(49, 37, false))->init()->setDestroyTime(10.0f)->setExplodeable(2000)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("obsidian")->setAllFacesTexture("obsidian");
	torch       = (new TorchTile(50, 5 * 16))->init()->setDestroyTime(0.0f)->setLightEmission(15 / 16.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("torch")->setAllFacesTexture("torch");

	stairs_wood = (new StairTile(53, wood))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsWood");
	chest		= (new ChestTile(54))->init()->setCategory(ItemCategory::FoodArmor)->setDestroyTime(2.5f)->setSoundType(SOUND_WOOD)->setDescriptionId("chest");

	emeraldOre  = (new OreTile(56, 16 * 3 + 2))->init()->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("oreDiamond")->setAllFacesTexture("diamond_ore");
	emeraldBlock= (new MetalTile(57, 40 - 16))->init()->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("blockDiamond")->setAllFacesTexture("diamond_block");
	workBench   = (new WorkbenchTile(58))->init()->setDestroyTime(2.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("workbench")
					->setFaceTexture(FACE_UP, "crafting_table_top")
					->setFaceTexture(FACE_DOWN, "planks_oak")
					->setFaceTexture(FACE_NORTH, "crafting_table_front")
					->setFaceTexture(FACE_SOUTH, "crafting_table_side")
					->setFaceTexture(FACE_WEST, "crafting_table_side")
					->setFaceTexture(FACE_EAST, "crafting_table_back");
	crops		= (new CropTile(59, 8 + 5 * 16))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("crops");
	farmland    = (new FarmTile(60))->init()->setDestroyTime(0.6f)->setSoundType(SOUND_GRAVEL)->setCategory(ItemCategory::Decorations)->setDescriptionId("farmland");
	furnace		= (new FurnaceTile(61, false))->init()->setDestroyTime(3.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("furnace")
					->setFaceTexture(FACE_UP, "furnace_top")
					->setFaceTexture(FACE_DOWN, "furnace_bottom")
					->setFaceTexture(FACE_NORTH, "furnace_front")
					->setSideTexture("furnace_side");
	furnace_lit = (new FurnaceTile(62, true))->init()->setDestroyTime(3.5f)->setSoundType(SOUND_STONE)->setLightEmission(14 / 16.0f)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("furnace")
					->setFaceTexture(FACE_UP, "furnace_top")
					->setFaceTexture(FACE_DOWN, "furnace_bottom")
					->setFaceTexture(FACE_NORTH, "furnace_front_lit")
					->setSideTexture("furnace_side");
    sign        = (new SignTile(63, TileEntityType::Sign, true))->init()->setDestroyTime(1.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("sign");
	door_wood   = (new DoorTile(64, Material::wood))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("doorWood");
	ladder      = (new LadderTile(65, 3 + 5 * 16))->init()->setDestroyTime(0.4f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("ladder")->setAllFacesTexture("ladder");

	stairs_stone= (new StairTile(67, stoneBrick))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsStone");
    wallSign    = (new SignTile(68, TileEntityType::Sign, false))->init()->setDestroyTime(1.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("sign");

	door_iron   = (new DoorTile(71, Material::metal))->init()->setDestroyTime(5.0f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("doorIron");

	redStoneOre = (new RedStoneOreTile(73, 16 * 3 + 3, false))->init()->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("oreRedstone")->setAllFacesTexture("redstone_ore");
	redStoneOre_lit = (new RedStoneOreTile(74, 16 * 3 + 3, true))->init()->setDestroyTime(3.0f)->setLightEmission(10 / 16.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("oreRedstone")->setAllFacesTexture("redstone_ore_lit");

	topSnow     = (new TopSnowTile(78, 16 * 4 + 2))->init()->setDestroyTime(0.1f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("snow")->setAllFacesTexture("snow");
	ice         = (new IceTile(79, 16 * 4 + 3))->init()->setDestroyTime(0.5f)->setLightBlock(3)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("ice")->setAllFacesTexture("ice");
	snow		= (new SnowTile(80, 16 * 4 + 2))->init()->setDestroyTime(0.2f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("snow")->setAllFacesTexture("snow");
    cactus      = (new CactusTile(81, 16 * 4 + 6))->init()->setDestroyTime(0.4f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("cactus")->setSimplifiedTextures("cactus_side", "cactus_top", "cactus_bottom");
	clay        = (new ClayTile(82, 16 * 4 + 8))->init()->setDestroyTime(0.6f)->setSoundType(SOUND_GRAVEL)->setCategory(ItemCategory::Decorations)->setDescriptionId("clay")->setAllFacesTexture("clay");
	reeds       = (new ReedTile(83, 16 * 4 + 9))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("reeds")->setAllFacesTexture("sugar_cane");

	fence		= (new FenceTile(85, 4))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fence")->setAllFacesTexture("fence_oak");

	netherrack = (new Tile(87, 7 + 6 * 16, Material::stone))->init()->setDestroyTime(0.4f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("hellrock")->setAllFacesTexture("netherrack");

	lightGem	= (new LightGemTile(89, 9 + 16 * 6, Material::glass))->init()->setDestroyTime(0.3f)->setCategory(ItemCategory::Decorations)->setSoundType(SOUND_GLASS)->setLightEmission(1.0f)->setDescriptionId("lightgem")->setAllFacesTexture("glowstone");
	invisible_bedrock = (new InvisibleTile(95, 0, Material::stone))->init()->setDestroyTime(-1)->setExplodeable(6000000)->setCategory(ItemCategory::Structures);
	trapdoor = (new TrapDoorTile(96, Material::wood))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("trapdoor");
 
	stoneBrickSmooth = (new MultiTextureTile(98, (const int*)&STONE_BRICK_TEXTURES, STONE_BRICK_TEXTURE_COUNT, Material::stone))->init()->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stonebricksmooth");

	thinGlass = (new ThinFenceTile(102, 1 + 3 * 16, 4 + 9 * 16, Material::glass, false))->init()->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Structures)->setDescriptionId("thinGlass")->setAllFacesTexture("glass_pane");

	netherPortal = (new NetherPortalTile(90, 250))->init()->setDestroyTime(-1.0f)->setLightEmission(11 / 16.0f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("netherPortal")->setAllFacesTexture("portal");
	classicPortal = (new ClassicPortalTile(250, 14, Material::portal))->init()->setDestroyTime(-1.0f)->setLightEmission(15 / 16.0f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("classicPortal");
	classicPortal->setTicking(true);

	melon = (new MelonTile(103))->init()->setDestroyTime(1.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("melon")->setSimplifiedTextures("melon_side", "melon_top", "melon_top");
	melonStem = (new StemTile(105, Tile::melon))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("pumpkinStem");
	fenceGate = (new FenceGateTile(107, 4))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceGate")->setAllFacesTexture("fence_oak");
	stairs_brick = (new StairTile(108, Tile::redBrick))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsBrick");

	stairs_stoneBrickSmooth = (new StairTile(109, Tile::stoneBrickSmooth))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsStoneBrickSmooth");
	netherBrick    = (new Tile(112, 0 + 14 * 16, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("netherBrick")->setAllFacesTexture("nether_brick");
	netherFence    = (new FenceTile(113, 0 + 14 * 16, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("netherFence")->setAllFacesTexture("nether_brick");
	stairs_netherBricks = (new StairTile(114, Tile::netherBrick))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsNetherBrick");
	netherStalk    = (new NetherStalkTile(115))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("netherStalk");
	stairs_sandStone = (new StairTile(128, Tile::sandStone))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsSandStone");

	quartzBlock   = (new QuartzBlockTile(155))->init()->setSoundType(SOUND_STONE)->setDestroyTime(0.8f)->setCategory(ItemCategory::Structures)->setDescriptionId("quartzBlock");
	stairs_quartz = (new StairTile(156, Tile::quartzBlock))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsQuartz");

	netherQuartzOre = (new NetherQuartzOreTile(117, 7 + 16 | TEXTURE_ALT_FLAG))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("netherQuartzOre")->setAllFacesTexture("nether_quartz_ore", 23 | Tile::TEXTURE_ALT_FLAG);
	endStone = (new Tile(118, 8 + 16 | TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("endStone")->setAllFacesTexture("end_stone", 24 | Tile::TEXTURE_ALT_FLAG);
	soulSand = (new SoulSandTile(119, 9 + 16 | TEXTURE_ALT_FLAG))->init()->setDestroyTime(0.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("soulSand")->setAllFacesTexture("soul_sand", 25 | Tile::TEXTURE_ALT_FLAG);

	netherWartBlock = (new Tile(214, 29 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.0f)->setExplodeable(1.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Structures)->setDescriptionId("netherWartBlock")->setAllFacesTexture("nether_wart_block", 29 | Tile::TEXTURE_ALT_FLAG);
	ancientDebris  = (new AncientDebrisTile(185))->init();

	stonecutterBench= (new StonecutterTile(245))->init()->setDestroyTime(2.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("stonecutter")
						->setFaceTexture(FACE_UP, "stonecutter_top")
						->setFaceTexture(FACE_DOWN, "stonecutter_bottom")
						->setFaceTexture(FACE_NORTH, "stonecutter_front")
						->setSideTexture("stonecutter_side");
	glowingObsidian = (new ObsidianTile(246, 10 + 16 * 13, true))->init()->setDestroyTime(10.0f)->setLightEmission(14 / 16.0f)->setExplodeable(2000)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("glowingobsidian")->setAllFacesTexture("glowing_obsidian");
	netherReactor	= (new NetherReactor(247, 10  + 14 * 16, Material::metal))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("netherreactor");
	info_updateGame1= (new Tile(248, 252, Material::dirt))->init()->setDestroyTime(1.0f)->setSoundType(SOUND_GRAVEL)->setCategory(ItemCategory::Structures)->setDescriptionId("info_update");
	info_updateGame2= (new Tile(249, 253, Material::dirt))->init()->setDestroyTime(1.0f)->setSoundType(SOUND_GRAVEL)->setCategory(ItemCategory::Structures)->setDescriptionId("info_update");
	grass_carried   = (new CarriedTile(253, 3, 12*16 + 12))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("grass");
	leaves_carried  = (LeafTile*) (new LeafTile(254, 11 + 14 * 16))->init()->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("leaves");
	info_reserved6  = (new Tile(255, Material::dirt))->init()->setCategory(ItemCategory::Structures);

    //
    // Stuff that need to be inited in a specific order (i.e. after the other tiles have been created)
    //
    fire     = (FireTile*) (new FireTile(51, 1 * 16 + 15))->init()->setDestroyTime(0.0f)->setLightEmission(1.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("fire");

	//
	// Initialize extra/community tiles (planks, deepslate, etc.)
	Tile::initExtraTiles();
	ModRegistry::registerBuiltInTiles();

    // Special case for certain items since they can have different icons
	// @note: Make sure those different items are handled in ItemInHandRenderer::renderItem
	//
	Item::items[cloth->id] = (new ClothTileItem(cloth->id - 256))->setCategory(ItemCategory::Structures)->setDescriptionId("cloth");
	Item::items[treeTrunk->id] = (new AuxDataTileItem(treeTrunk->id - 256, treeTrunk))->setCategory(ItemCategory::Decorations)->setDescriptionId("log");
	// Register separate item entries for birch and spruce trunks
	Item::items[birchTrunk->id] = (new TileItem(birchTrunk->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("logBirch");
	Item::items[spruceTrunk->id] = (new TileItem(spruceTrunk->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("logSpruce");
	Item::items[jungleTrunk->id] = (new TileItem(jungleTrunk->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("logJungle");
	Item::items[acaciaTrunk->id] = (new TileItem(acaciaTrunk->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("logAcacia");

	Item::items[stoneBrickSmooth->id] = (new AuxDataTileItem(stoneBrickSmooth->id - 256, stoneBrickSmooth))->setCategory(ItemCategory::Structures)->setDescriptionId("stonebricksmooth");
	Item::items[stoneSlabHalf->id] = (new TileItem(stoneSlabHalf->id - 256))->setCategory(ItemCategory::Structures)->setDescriptionId("stoneSlab");
	Item::items[sapling->id] = (new SaplingTileItem(sapling->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("sapling");
	Item::items[spruceSapling->id]  = (new SaplingTileItem(spruceSapling->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingSpruce");
	Item::items[birchSapling->id]   = (new SaplingTileItem(birchSapling->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingBirch");
	Item::items[jungleSapling->id]  = (new SaplingTileItem(jungleSapling->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingJungle");
	Item::items[acaciaSapling->id]  = (new SaplingTileItem(acaciaSapling->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingAcacia");
	Item::items[darkOakSapling->id] = (new SaplingTileItem(darkOakSapling->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingDarkOak");

	Item::items[leaves->id] = (new LeafTileItem(leaves->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("leaves");
	Item::items[spruceLeaves->id]  = (new LeafTileItem(spruceLeaves->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesSpruce");
	Item::items[birchLeaves->id]   = (new LeafTileItem(birchLeaves->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesBirch");
	Item::items[jungleLeaves->id]  = (new LeafTileItem(jungleLeaves->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesJungle");
	Item::items[acaciaLeaves->id]  = (new LeafTileItem(acaciaLeaves->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesAcacia");
	Item::items[darkOakLeaves->id] = (new LeafTileItem(darkOakLeaves->id - 256))->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesDarkOak");

	Item::items[sandStone->id] = (new AuxDataTileItem(sandStone->id - 256, sandStone))->setCategory(ItemCategory::Structures)->setDescriptionId("sandStone");

	Item::items[quartzBlock->id] = (new AuxDataTileItem(quartzBlock->id - 256, quartzBlock))->setCategory(ItemCategory::Structures)->setDescriptionId("quartzBlock");

	// ============================================
	// EJEMPLOS DE SISTEMA MATERIAL_INSTANCES
	// ============================================
	
	// FORMATO DIRECCIONAL ESPECÍFICO (similar a Minecraft Bedrock)
	// compassBlock = (new Tile(250, 0, Material::stone))
	// 	->setFaceTexture(FACE_DOWN, "compass_block_down")
	// 	->setFaceTexture(FACE_UP, "compass_block_up")
	// 	->setFaceTexture(FACE_NORTH, "compass_block_north")
	// 	->setFaceTexture(FACE_EAST, "compass_block_east")
	// 	->setFaceTexture(FACE_SOUTH, "compass_block_south")
	// 	->setFaceTexture(FACE_WEST, "compass_block_west")
	// 	->init()->setDestroyTime(2.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("compassBlock");
	
	// FORMATO SIMPLIFICADO (side, top, bottom)
	// customStone = (new Tile(251, 0, Material::stone))
	// 	->setSimplifiedTextures("stone_side", "stone_top", "stone_bottom")
	// 	->init()->setDestroyTime(2.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("customStone");
	
	// MIXTO: texturas separadas + índices de atlas
	// customFurnace = (new Tile(252, 0, Material::stone))
	// 	->setFaceTexture(FACE_DOWN, "furnace_bottom", 2 + 16 * 14)
	// 	->setFaceTexture(FACE_UP, "furnace_top", 1 + 16 * 14)
	// 	->setFaceTexture(FACE_NORTH, "furnace_front")
	// 	->setFaceTexture(FACE_SOUTH, 3 + 16 * 14)  // solo atlas
	// 	->setFaceTexture(FACE_WEST, 3 + 16 * 14)
	// 	->setFaceTexture(FACE_EAST, 3 + 16 * 14)
	// 	->init()->setDestroyTime(3.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("customFurnace");
	
	// TODAS LAS CARAS IGUALES CON TEXTURA SEPARADA
	// uniformBlock = (new Tile(253, 0, Material::stone))
	// 	->setAllFacesTexture("uniform_texture")
	// 	->init()->setDestroyTime(2.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("uniformBlock");
	
	// ============================================
	// FIN EJEMPLOS MATERIAL_INSTANCES
	// ============================================

    for (size_t i = 0; i < NUM_BLOCK_TYPES; i++) {
        if (Tile::tiles[i] != NULL) {
			if (Item::items[i] == NULL) {
				Item::items[i] = new TileItem(i - 256);
				Item::items[i]->category = Tile::tiles[i]->category;
			}
			// Check for missing category
			if (Item::items[i]->category == -1)
				LOGE("Error: Missing category for tile %d: %s\n", tiles[i]->id, tiles[i]->getDescriptionId().c_str());
        }
    }
}

/*static*/
void Tile::teardownTiles() {
	for (size_t i = 0; i < NUM_BLOCK_TYPES; ++i)
		if (Tile::tiles[i]) {
			delete Tile::tiles[i];
			Tile::tiles[i] = NULL;
		}
}

// Initialize extra/community tiles (previously in ExtraTiles.cpp)
void Tile::initExtraTiles()
{
	// New plank blocks using textures from the secondary atlas (terrain2.png)
	// Spruce planks texture at (4,0) -> index 4, Birch planks at (5,0) -> index 5
	sprucePlanks  = (new Tile(150, 4 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("planksSpruce")->setAllFacesTexture("planks_spruce", 4 | Tile::TEXTURE_ALT_FLAG);
	birchPlanks   = (new Tile(151, 5 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("planksBirch")->setAllFacesTexture("planks_birch", 5 | Tile::TEXTURE_ALT_FLAG);
	junglePlanks  = (new Tile(152, 36 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("planksJungle")->setAllFacesTexture("planks_jungle", 36 | Tile::TEXTURE_ALT_FLAG);
	acaciaPlanks  = (new Tile(153, 37 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("planksAcacia")->setAllFacesTexture("planks_acacia", 37 | Tile::TEXTURE_ALT_FLAG);
	darkOakPlanks = (new Tile(154, 38 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("planksDarkOak")->setAllFacesTexture("planks_big_oak", 38 | Tile::TEXTURE_ALT_FLAG);

	jungleTrunk = (new TreeTile(186))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("logJungle");
	acaciaTrunk = (new TreeTile(187))->init()->setDestroyTime(2.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("logAcacia");

	// Spruce Slabs
	spruceSlab = (new Tile(157, 4 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("spruceSlab")->setAllFacesTexture("planks_spruce", 4 | Tile::TEXTURE_ALT_FLAG);

	spruceSlabHalf = (new Tile(158, 4 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init();
	spruceSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	spruceSlabHalf->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("spruceSlab")->setAllFacesTexture("planks_spruce", 4 | Tile::TEXTURE_ALT_FLAG);

	// Birch Slabs
	birchSlab = (new Tile(159, 5 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("birchSlab")->setAllFacesTexture("planks_birch", 5 | Tile::TEXTURE_ALT_FLAG);

	birchSlabHalf = (new Tile(160, 5 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init();
	birchSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	birchSlabHalf->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("birchSlab")->setAllFacesTexture("planks_birch", 5 | Tile::TEXTURE_ALT_FLAG);

	// Stairs
	stairs_spruce  = (new StairTile(161, sprucePlanks))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsSpruce");
	stairs_birch   = (new StairTile(162, birchPlanks))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsBirch");

	// Fences
	fenceSpruce = (new FenceTile(164, 4 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceSpruce")->setAllFacesTexture("fence_spruce", 4 | Tile::TEXTURE_ALT_FLAG);
	fenceBirch  = (new FenceTile(163, 5 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceBirch")->setAllFacesTexture("fence_birch", 5 | Tile::TEXTURE_ALT_FLAG);

	// Fence Gates
	fenceGateSpruce = (new FenceGateTile(166, 4 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceGateSpruce")->setAllFacesTexture("fence_spruce", 4 | Tile::TEXTURE_ALT_FLAG);
	fenceGateBirch  = (new FenceGateTile(165, 5 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceGateBirch")->setAllFacesTexture("fence_birch", 5 | Tile::TEXTURE_ALT_FLAG);

	// Deepslate blocks (IDs 170-177)
	deepslate            = (new DeepslateTile(170, 6 | Tile::TEXTURE_ALT_FLAG, 171))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslate")->setAllFacesTexture("deepslate", 6 | Tile::TEXTURE_ALT_FLAG);
	cobbledDeepslate     = (new DeepslateTile(171, 7 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslateCobbled")->setAllFacesTexture("cobbled_deepslate", 7 | Tile::TEXTURE_ALT_FLAG);
	deepslateCoalOre     = (new OreTile(172, 8 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslateOreCoal")->setAllFacesTexture("deepslate_coal_ore", 8 | Tile::TEXTURE_ALT_FLAG);
	deepslateDiamondOre  = (new OreTile(173, 9 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslateOreDiamond")->setAllFacesTexture("deepslate_diamond_ore", 9 | Tile::TEXTURE_ALT_FLAG);
	deepslateGoldOre     = (new OreTile(174, 10 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslateOreGold")->setAllFacesTexture("deepslate_gold_ore", 10 | Tile::TEXTURE_ALT_FLAG);
	deepslateIronOre     = (new OreTile(175, 11 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslateOreIron")->setAllFacesTexture("deepslate_iron_ore", 11 | Tile::TEXTURE_ALT_FLAG);
	deepslateLapisOre    = (new OreTile(176, 12 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslateOreLapis")->setAllFacesTexture("deepslate_lapis_ore", 12 | Tile::TEXTURE_ALT_FLAG);
	deepslateRedstoneOre = (new RedStoneOreTile(177, 13 | Tile::TEXTURE_ALT_FLAG, false))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslateOreRedstone")->setAllFacesTexture("deepslate_redstone_ore", 13 | Tile::TEXTURE_ALT_FLAG);
	deepslateRedstoneOre_lit = (new RedStoneOreTile(178, 13 | Tile::TEXTURE_ALT_FLAG, true))->init()->setDestroyTime(3.0f)->setLightEmission(10 / 16.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("deepslateOreRedstone")->setAllFacesTexture("deepslate_redstone_ore_lit", 13 | Tile::TEXTURE_ALT_FLAG);

	// New Deepslate variants (IDs 181-183)
	deepslatePolished    = (new PolishedDeepslateTile(181))->init()->setDestroyTime(1.5f)->setExplodeable(6.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslatePolished")->setAllFacesTexture("polished_deepslate", 18 | Tile::TEXTURE_ALT_FLAG);
	deepslateTiles       = (new DeepslateTilesTile(182))->init()->setDestroyTime(1.5f)->setExplodeable(6.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateTiles")->setAllFacesTexture("deepslate_tiles", 19 | Tile::TEXTURE_ALT_FLAG);
	deepslateBricks      = (new DeepslateBricksTile(183))->init()->setDestroyTime(1.5f)->setExplodeable(6.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateBricks")->setAllFacesTexture("deepslate_bricks", 20 | Tile::TEXTURE_ALT_FLAG);

	// Leaves (IDs 188-192)
	spruceLeaves  = (LeafTile*) (new LeafTile(188, 132, LeafTile::EVERGREEN_LEAF))->init()->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesSpruce");
	birchLeaves   = (LeafTile*) (new LeafTile(189, 41 | Tile::TEXTURE_ALT_FLAG, LeafTile::BIRCH_LEAF))->init()->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesBirch");
	jungleLeaves  = (LeafTile*) (new LeafTile(190, 43 | Tile::TEXTURE_ALT_FLAG, LeafTile::JUNGLE_LEAF))->init()->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesJungle");
	acaciaLeaves  = (LeafTile*) (new LeafTile(191, 45 | Tile::TEXTURE_ALT_FLAG, LeafTile::ACACIA_LEAF))->init()->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesAcacia");
	darkOakLeaves = (LeafTile*) (new LeafTile(192, 47 | Tile::TEXTURE_ALT_FLAG, LeafTile::DARK_OAK_LEAF))->init()->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("leavesDarkOak");

	// Saplings (IDs 193-197)
	spruceSapling  = (new Sapling(193, 15 + 16 * 3, LeafTile::EVERGREEN_LEAF))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingSpruce")->setAllFacesTexture("sapling_spruce");
	birchSapling   = (new Sapling(194, 15 + 16 * 4, LeafTile::BIRCH_LEAF))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingBirch")->setAllFacesTexture("sapling_birch");
	jungleSapling  = (new Sapling(195, 39 | Tile::TEXTURE_ALT_FLAG, LeafTile::JUNGLE_LEAF))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingJungle")->setAllFacesTexture("sapling_jungle", 39 | Tile::TEXTURE_ALT_FLAG);
	acaciaSapling  = (new Sapling(196, 40 | Tile::TEXTURE_ALT_FLAG, LeafTile::ACACIA_LEAF))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingAcacia")->setAllFacesTexture("sapling_acacia", 40 | Tile::TEXTURE_ALT_FLAG);
	darkOakSapling = (new Sapling(197, 54 | Tile::TEXTURE_ALT_FLAG, LeafTile::DARK_OAK_LEAF))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("saplingDarkOak")->setAllFacesTexture("sapling_big_oak", 54 | Tile::TEXTURE_ALT_FLAG);

	// Jungle wood structures (IDs 198-202)
	stairs_jungle    = (new StairTile(198, junglePlanks))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsJungle");
	jungleSlab       = (new Tile(199, 36 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("jungleSlab")->setAllFacesTexture("planks_jungle", 36 | Tile::TEXTURE_ALT_FLAG);
	jungleSlabHalf   = (new Tile(200, 36 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init();
	jungleSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	jungleSlabHalf->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("jungleSlab")->setAllFacesTexture("planks_jungle", 36 | Tile::TEXTURE_ALT_FLAG);
	fenceJungle      = (new FenceTile(201, 36 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceJungle")->setAllFacesTexture("planks_jungle", 36 | Tile::TEXTURE_ALT_FLAG);
	fenceGateJungle  = (new FenceGateTile(202, 36 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceGateJungle")->setAllFacesTexture("planks_jungle", 36 | Tile::TEXTURE_ALT_FLAG);

	// Acacia wood structures (IDs 203-207)
	stairs_acacia    = (new StairTile(203, acaciaPlanks))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsAcacia");
	acaciaSlab       = (new Tile(204, 37 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("acaciaSlab")->setAllFacesTexture("planks_acacia", 37 | Tile::TEXTURE_ALT_FLAG);
	acaciaSlabHalf   = (new Tile(205, 37 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init();
	acaciaSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	acaciaSlabHalf->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("acaciaSlab")->setAllFacesTexture("planks_acacia", 37 | Tile::TEXTURE_ALT_FLAG);
	fenceAcacia      = (new FenceTile(206, 37 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceAcacia")->setAllFacesTexture("planks_acacia", 37 | Tile::TEXTURE_ALT_FLAG);
	fenceGateAcacia  = (new FenceGateTile(207, 37 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceGateAcacia")->setAllFacesTexture("planks_acacia", 37 | Tile::TEXTURE_ALT_FLAG);

	// Dark Oak wood structures (IDs 208-212)
	stairs_darkOak   = (new StairTile(208, darkOakPlanks))->init()->setCategory(ItemCategory::Structures)->setDescriptionId("stairsDarkOak");
	darkOakSlab      = (new Tile(209, 38 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("darkOakSlab")->setAllFacesTexture("planks_big_oak", 38 | Tile::TEXTURE_ALT_FLAG);
	darkOakSlabHalf  = (new Tile(210, 38 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init();
	darkOakSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	darkOakSlabHalf->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("darkOakSlab")->setAllFacesTexture("planks_big_oak", 38 | Tile::TEXTURE_ALT_FLAG);
	fenceDarkOak     = (new FenceTile(211, 38 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceDarkOak")->setAllFacesTexture("planks_big_oak", 38 | Tile::TEXTURE_ALT_FLAG);
	fenceGateDarkOak = (new FenceGateTile(212, 38 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("fenceGateDarkOak")->setAllFacesTexture("planks_big_oak", 38 | Tile::TEXTURE_ALT_FLAG);

	// Sandstone Slabs (IDs 215, 216)
	sandstoneSlab       = (new Tile(215, 192, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("sandstoneSlab");
	sandstoneSlabHalf   = (new Tile(216, 192, Material::stone))->init();
	sandstoneSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	sandstoneSlabHalf->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("sandstoneSlab");

	// Oak Slabs (IDs 217, 218)
	oakSlab       = (new Tile(217, 4, Material::wood))->init()->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("oakSlab");
	oakSlabHalf   = (new Tile(218, 4, Material::wood))->init();
	oakSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	oakSlabHalf->setDestroyTime(2.0f)->setExplodeable(5.0f / 3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("oakSlab");

	// Cobblestone Slabs (IDs 219, 220)
	cobbleSlab       = (new Tile(219, 16, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("cobbleSlab");
	cobbleSlabHalf   = (new Tile(220, 16, Material::stone))->init();
	cobbleSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	cobbleSlabHalf->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("cobbleSlab");

	// Brick Slabs (IDs 221, 222)
	brickSlab       = (new Tile(221, 7, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("brickSlab");
	brickSlabHalf   = (new Tile(222, 7, Material::stone))->init();
	brickSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	brickSlabHalf->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("brickSlab");

	// Stone Brick Slabs (IDs 223, 224)
	stoneBrickSlab       = (new Tile(223, 54, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stoneBrickSlab");
	stoneBrickSlabHalf   = (new Tile(224, 54, Material::stone))->init();
	stoneBrickSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	stoneBrickSlabHalf->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stoneBrickSlab");

	// Nether Brick Slabs (IDs 225, 226)
	netherBrickSlab       = (new Tile(225, 224, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("netherBrickSlab");
	netherBrickSlabHalf   = (new Tile(226, 224, Material::stone))->init();
	netherBrickSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	netherBrickSlabHalf->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("netherBrickSlab");

	// Quartz Slabs (IDs 227, 228)
	quartzSlab       = (new Tile(227, 233, Material::stone))->init()->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("quartzSlab");
	quartzSlabHalf   = (new Tile(228, 233, Material::stone))->init();
	quartzSlabHalf->setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	quartzSlabHalf->setDestroyTime(2.0f)->setExplodeable(10.0f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("quartzSlab");

	// Granite, Diorite, Andesite (IDs 131..136)
	granite        = (new Tile(131, 56 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("stoneGranite")->setAllFacesTexture("stone_granite", 56 | Tile::TEXTURE_ALT_FLAG);
	graniteSmooth  = (new Tile(132, 57 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("stoneGraniteSmooth")->setAllFacesTexture("stone_granite_smooth", 57 | Tile::TEXTURE_ALT_FLAG);
	diorite        = (new Tile(133, 58 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("stoneDiorite")->setAllFacesTexture("stone_diorite", 58 | Tile::TEXTURE_ALT_FLAG);
	dioriteSmooth  = (new Tile(134, 59 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("stoneDioriteSmooth")->setAllFacesTexture("stone_diorite_smooth", 59 | Tile::TEXTURE_ALT_FLAG);
	andesite       = (new Tile(135, 60 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("stoneAndesite")->setAllFacesTexture("stone_andesite", 60 | Tile::TEXTURE_ALT_FLAG);
	andesiteSmooth = (new Tile(136, 61 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("stoneAndesiteSmooth")->setAllFacesTexture("stone_andesite_smooth", 61 | Tile::TEXTURE_ALT_FLAG);

	// Podzol & Mycelium (IDs 137, 110)
	podzol   = (new Tile(137, 63 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(0.5f)->setSoundType(SOUND_GRAVEL)->setCategory(ItemCategory::Decorations)->setDescriptionId("dirtPodzol")
				->setSideTexture("dirt_podzol_side", 63 | Tile::TEXTURE_ALT_FLAG)
				->setTopTexture("dirt_podzol_top", 62 | Tile::TEXTURE_ALT_FLAG)
				->setBottomTexture("dirt", 2);
	mycelium = (new Tile(110, 65 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(0.6f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mycelium")
				->setSideTexture("mycelium_side", 65 | Tile::TEXTURE_ALT_FLAG)
				->setTopTexture("mycelium_top", 64 | Tile::TEXTURE_ALT_FLAG)
				->setBottomTexture("dirt", 2);

	// Ices (IDs 138..140)
	packedIce  = (new Tile(138, 66 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(0.5f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("icePacked")->setAllFacesTexture("ice_packed", 66 | Tile::TEXTURE_ALT_FLAG);
	blueIce    = (new Tile(139, 67 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(2.8f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("iceBlue")->setAllFacesTexture("blue_ice", 67 | Tile::TEXTURE_ALT_FLAG);
	frostedIce = (new Tile(140, 68 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(0.5f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("iceFrosted")->setAllFacesTexture("frosted_ice_0", 68 | Tile::TEXTURE_ALT_FLAG);

	// Hardened Clay / Terracotta (ID 130)
	hardenedClay = (new Tile(130, 69 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardened")->setAllFacesTexture("hardened_clay", 69 | Tile::TEXTURE_ALT_FLAG);

	// 16 Stained Clays (IDs 229..244)
	stainedClay_white     = (new Tile(229, 70 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedWhite")->setAllFacesTexture("hardened_clay_stained_white", 70 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_orange    = (new Tile(230, 71 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedOrange")->setAllFacesTexture("hardened_clay_stained_orange", 71 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_magenta   = (new Tile(231, 72 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedMagenta")->setAllFacesTexture("hardened_clay_stained_magenta", 72 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_lightBlue = (new Tile(232, 73 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedLightBlue")->setAllFacesTexture("hardened_clay_stained_light_blue", 73 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_yellow    = (new Tile(233, 74 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedYellow")->setAllFacesTexture("hardened_clay_stained_yellow", 74 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_lime      = (new Tile(234, 75 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedLime")->setAllFacesTexture("hardened_clay_stained_lime", 75 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_pink      = (new Tile(235, 76 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedPink")->setAllFacesTexture("hardened_clay_stained_pink", 76 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_gray      = (new Tile(236, 77 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedGray")->setAllFacesTexture("hardened_clay_stained_gray", 77 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_silver    = (new Tile(237, 78 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedSilver")->setAllFacesTexture("hardened_clay_stained_silver", 78 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_cyan      = (new Tile(238, 79 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedCyan")->setAllFacesTexture("hardened_clay_stained_cyan", 79 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_purple    = (new Tile(239, 80 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedPurple")->setAllFacesTexture("hardened_clay_stained_purple", 80 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_blue      = (new Tile(240, 81 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedBlue")->setAllFacesTexture("hardened_clay_stained_blue", 81 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_brown     = (new Tile(241, 82 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedBrown")->setAllFacesTexture("hardened_clay_stained_brown", 82 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_green     = (new Tile(242, 83 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedGreen")->setAllFacesTexture("hardened_clay_stained_green", 83 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_red       = (new Tile(243, 84 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedRed")->setAllFacesTexture("hardened_clay_stained_red", 84 | Tile::TEXTURE_ALT_FLAG);
	stainedClay_black     = (new Tile(244, 85 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.25f)->setExplodeable(4.2f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Decorations)->setDescriptionId("clayHardenedBlack")->setAllFacesTexture("hardened_clay_stained_black", 85 | Tile::TEXTURE_ALT_FLAG);

	// Huge Mushroom blocks (IDs 99, 100, 141)
	mushroomBlock_brown = (new Tile(99, 86 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("mushroomBlockBrown")
							->setSideTexture("mushroom_block_skin_brown", 86 | Tile::TEXTURE_ALT_FLAG)
							->setTopTexture("mushroom_block_skin_brown", 86 | Tile::TEXTURE_ALT_FLAG)
							->setBottomTexture("mushroom_block_inside", 89 | Tile::TEXTURE_ALT_FLAG);
	mushroomBlock_red   = (new Tile(100, 87 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("mushroomBlockRed")
							->setSideTexture("mushroom_block_skin_red", 87 | Tile::TEXTURE_ALT_FLAG)
							->setTopTexture("mushroom_block_skin_red", 87 | Tile::TEXTURE_ALT_FLAG)
							->setBottomTexture("mushroom_block_inside", 89 | Tile::TEXTURE_ALT_FLAG);
	mushroomBlock_stem  = (new Tile(141, 88 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("mushroomBlockStem")
							->setSideTexture("mushroom_block_skin_stem", 88 | Tile::TEXTURE_ALT_FLAG)
							->setTopTexture("mushroom_block_inside", 89 | Tile::TEXTURE_ALT_FLAG)
							->setBottomTexture("mushroom_block_inside", 89 | Tile::TEXTURE_ALT_FLAG);

	// Flora & Special (IDs 111, 106, 127, 142)
	waterlily       = (new FlowerTile(111, 90 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("waterlily")->setAllFacesTexture("waterlily", 90 | Tile::TEXTURE_ALT_FLAG);
	vines           = (new FlowerTile(106, 91 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(0.2f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("vine")->setAllFacesTexture("vine", 91 | Tile::TEXTURE_ALT_FLAG);
	cocoa           = (new Tile(127, 92 | Tile::TEXTURE_ALT_FLAG, Material::plant))->init()->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("cocoa")->setAllFacesTexture("cocoa_stage_2", 92 | Tile::TEXTURE_ALT_FLAG);
	sweetBerryBush  = (new FlowerTile(142, 93 | Tile::TEXTURE_ALT_FLAG))->init()->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("sweetBerryBush")->setAllFacesTexture("sweet_berry_bush_stage3", 93 | Tile::TEXTURE_ALT_FLAG);

	// 1.7+ Flowers (IDs 143..149, 167..169, 184, 213)
	flower_allium           = (new FlowerTile(143, 94 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerAllium")->setAllFacesTexture("flower_allium", 94 | Tile::TEXTURE_ALT_FLAG);
	flower_blueOrchid       = (new FlowerTile(144, 95 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerBlueOrchid")->setAllFacesTexture("flower_blue_orchid", 95 | Tile::TEXTURE_ALT_FLAG);
	flower_houstonia        = (new FlowerTile(145, 96 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerHoustonia")->setAllFacesTexture("flower_houstonia", 96 | Tile::TEXTURE_ALT_FLAG);
	flower_tulipRed         = (new FlowerTile(146, 97 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerTulipRed")->setAllFacesTexture("flower_tulip_red", 97 | Tile::TEXTURE_ALT_FLAG);
	flower_tulipOrange      = (new FlowerTile(147, 98 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerTulipOrange")->setAllFacesTexture("flower_tulip_orange", 98 | Tile::TEXTURE_ALT_FLAG);
	flower_tulipWhite       = (new FlowerTile(148, 99 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerTulipWhite")->setAllFacesTexture("flower_tulip_white", 99 | Tile::TEXTURE_ALT_FLAG);
	flower_tulipPink        = (new FlowerTile(149, 100 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerTulipPink")->setAllFacesTexture("flower_tulip_pink", 100 | Tile::TEXTURE_ALT_FLAG);
	flower_oxeyeDaisy       = (new FlowerTile(167, 101 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerOxeyeDaisy")->setAllFacesTexture("flower_oxeye_daisy", 101 | Tile::TEXTURE_ALT_FLAG);
	flower_paeonia          = (new FlowerTile(168, 102 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerPaeonia")->setAllFacesTexture("flower_paeonia", 102 | Tile::TEXTURE_ALT_FLAG);
	flower_roseBlue         = (new FlowerTile(169, 103 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerRoseBlue")->setAllFacesTexture("flower_rose_blue", 103 | Tile::TEXTURE_ALT_FLAG);
	flower_lilyOfTheValley  = (new FlowerTile(184, 104 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerLilyOfTheValley")->setAllFacesTexture("flower_lily_of_the_valley", 104 | Tile::TEXTURE_ALT_FLAG);
	flower_witherRose       = (new FlowerTile(213, 105 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerWitherRose")->setAllFacesTexture("flower_wither_rose", 105 | Tile::TEXTURE_ALT_FLAG);

	// ============================================
	// Bedrock Generator / Structures Block Initializations (IDs 512..745)
	// ============================================
	acaciaDoor = (new DoorTile(512, 106 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("acaciaDoor")->setAllFacesTexture("door_acacia_lower", 106 | Tile::TEXTURE_ALT_FLAG);
	acaciaPressurePlate = (new Tile(513, 37 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("acaciaPressurePlate")->setAllFacesTexture("planks_acacia", 37 | Tile::TEXTURE_ALT_FLAG);
	acaciaWood = (new Tile(514, 34 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("acaciaWood")->setAllFacesTexture("log_acacia", 34 | Tile::TEXTURE_ALT_FLAG);
	barrel = (new Tile(515, 107 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("barrel")->setAllFacesTexture("barrel_side", 107 | Tile::TEXTURE_ALT_FLAG);
	basalt = (new Tile(516, 108 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("basalt")->setAllFacesTexture("basalt_side", 108 | Tile::TEXTURE_ALT_FLAG);
	bell = (new Tile(517, 109 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("bell")->setAllFacesTexture("bell_bottom", 109 | Tile::TEXTURE_ALT_FLAG);
	blackCarpet = (new Tile(518, 110 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("blackCarpet")->setAllFacesTexture("wool_colored_black", 110 | Tile::TEXTURE_ALT_FLAG);
	blackGlazedTerracotta = (new Tile(519, 111 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("blackGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_black", 111 | Tile::TEXTURE_ALT_FLAG);
	blackStainedGlass = (new Tile(520, 49, Material::glass))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("blackStainedGlass")->setAllFacesTexture("glass", 49);
	blackstone = (new Tile(521, 112 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("blackstone")->setAllFacesTexture("blackstone", 112 | Tile::TEXTURE_ALT_FLAG);
	blackstoneSlab = (new SlabTile(522, 112 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("blackstoneSlab")->setAllFacesTexture("blackstone", 112 | Tile::TEXTURE_ALT_FLAG);
	blackstoneStairs = (new StairTile(523, 112 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("blackstoneStairs")->setAllFacesTexture("blackstone", 112 | Tile::TEXTURE_ALT_FLAG);
	blackstoneWall = (new FenceTile(524, 112 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("blackstoneWall")->setAllFacesTexture("blackstone", 112 | Tile::TEXTURE_ALT_FLAG);
	blastFurnace = (new Tile(525, 113 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("blastFurnace")->setAllFacesTexture("blast_furnace_front_off", 113 | Tile::TEXTURE_ALT_FLAG);
	blueCarpet = (new Tile(526, 114 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("blueCarpet")->setAllFacesTexture("wool_colored_blue", 114 | Tile::TEXTURE_ALT_FLAG);
	boneBlock = (new Tile(527, 115 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("boneBlock")->setAllFacesTexture("bone_block_side", 115 | Tile::TEXTURE_ALT_FLAG);
	brewingStand = (new Tile(528, 116 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("brewingStand")->setAllFacesTexture("brewing_stand", 116 | Tile::TEXTURE_ALT_FLAG);
	brickWall = (new FenceTile(529, 7, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("brickWall")->setAllFacesTexture("brick", 7);
	brownCarpet = (new Tile(530, 117 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("brownCarpet")->setAllFacesTexture("wool_colored_brown", 117 | Tile::TEXTURE_ALT_FLAG);
	brownStainedGlass = (new Tile(531, 49, Material::glass))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("brownStainedGlass")->setAllFacesTexture("glass", 49);
	campfire = (new Tile(532, 118 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("campfire")->setLightEmission(1.0f)->setAllFacesTexture("campfire_log", 118 | Tile::TEXTURE_ALT_FLAG);
	candle = (new Tile(533, 119 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("candle")->setLightEmission(3 / 16.0f)->setAllFacesTexture("candle", 119 | Tile::TEXTURE_ALT_FLAG);
	cartographyTable = (new Tile(534, 120 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("cartographyTable")->setAllFacesTexture("cartography_table_side3", 120 | Tile::TEXTURE_ALT_FLAG);
	carvedPumpkin = (new Tile(535, 121 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("carvedPumpkin")->setAllFacesTexture("pumpkin_face_off", 121 | Tile::TEXTURE_ALT_FLAG);
	cauldron = (new Tile(536, 122 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("cauldron")->setAllFacesTexture("cauldron_side", 122 | Tile::TEXTURE_ALT_FLAG);
	chiseledDeepslate = (new Tile(537, 123 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("chiseledDeepslate")->setAllFacesTexture("chiseled_deepslate", 123 | Tile::TEXTURE_ALT_FLAG);
	chiseledPolishedBlackstone = (new Tile(538, 124 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("chiseledPolishedBlackstone")->setAllFacesTexture("chiseled_polished_blackstone", 124 | Tile::TEXTURE_ALT_FLAG);
	chiseledSandstone = (new Tile(539, 229, Material::sand))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("chiseledSandstone")->setAllFacesTexture("sandstone_chiseled", 229);
	chiseledStoneBricks = (new Tile(540, 16, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("chiseledStoneBricks")->setAllFacesTexture("stone_brick", 16);
	chiseledTuff = (new Tile(541, 125 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("chiseledTuff")->setAllFacesTexture("tuff", 125 | Tile::TEXTURE_ALT_FLAG);
	chiseledTuffBricks = (new Tile(542, 126 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("chiseledTuffBricks")->setAllFacesTexture("tuff_bricks", 126 | Tile::TEXTURE_ALT_FLAG);
	cinnabar = (new Tile(543, 127 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("cinnabar")->setAllFacesTexture("cinnabar", 127 | Tile::TEXTURE_ALT_FLAG);
	coalBlock = (new Tile(544, 128 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("coalBlock")->setAllFacesTexture("coal_block", 128 | Tile::TEXTURE_ALT_FLAG);
	coarseDirt = (new Tile(545, 129 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRAVEL)->setCategory(ItemCategory::Decorations)->setDescriptionId("coarseDirt")->setAllFacesTexture("coarse_dirt", 129 | Tile::TEXTURE_ALT_FLAG);
	cobbledDeepslateSlab = (new SlabTile(546, 7 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("cobbledDeepslateSlab")->setAllFacesTexture("cobbled_deepslate", 7 | Tile::TEXTURE_ALT_FLAG);
	cobbledDeepslateStairs = (new StairTile(547, 7 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("cobbledDeepslateStairs")->setAllFacesTexture("cobbled_deepslate", 7 | Tile::TEXTURE_ALT_FLAG);
	cobbledDeepslateWall = (new FenceTile(548, 7 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("cobbledDeepslateWall")->setAllFacesTexture("cobbled_deepslate", 7 | Tile::TEXTURE_ALT_FLAG);
	cobblestoneWall = (new FenceTile(549, 130 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("cobblestoneWall")->setAllFacesTexture("cobblestone", 130 | Tile::TEXTURE_ALT_FLAG);
	composter = (new Tile(550, 131 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("composter")->setAllFacesTexture("compost", 131 | Tile::TEXTURE_ALT_FLAG);
	copperBlock = (new Tile(551, 132 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("copperBlock")->setAllFacesTexture("copper_block", 132 | Tile::TEXTURE_ALT_FLAG);
	crackedDeepslateBricks = (new Tile(552, 20 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("crackedDeepslateBricks")->setAllFacesTexture("deepslate_bricks", 20 | Tile::TEXTURE_ALT_FLAG);
	crackedDeepslateTiles = (new Tile(553, 19 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("crackedDeepslateTiles")->setAllFacesTexture("deepslate_tiles", 19 | Tile::TEXTURE_ALT_FLAG);
	crackedPolishedBlackstoneBricks = (new Tile(554, 112 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("crackedPolishedBlackstoneBricks")->setAllFacesTexture("blackstone", 112 | Tile::TEXTURE_ALT_FLAG);
	crackedStoneBricks = (new Tile(555, 101, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("crackedStoneBricks")->setAllFacesTexture("stonebrick_cracked", 101);
	cutSandstone = (new Tile(556, 192, Material::sand))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("cutSandstone")->setAllFacesTexture("sandstone_normal", 192);
	cyanCarpet = (new Tile(557, 133 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("cyanCarpet")->setAllFacesTexture("wool_colored_cyan", 133 | Tile::TEXTURE_ALT_FLAG);
	cyanGlazedTerracotta = (new Tile(558, 134 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("cyanGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_cyan", 134 | Tile::TEXTURE_ALT_FLAG);
	damagedAnvil = (new Tile(559, 135 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("damagedAnvil")->setAllFacesTexture("anvil_top_damaged_1", 135 | Tile::TEXTURE_ALT_FLAG);
	darkOakDoor = (new DoorTile(560, 136 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("darkOakDoor")->setAllFacesTexture("door_dark_oak_lower", 136 | Tile::TEXTURE_ALT_FLAG);
	darkOakLog = (new Tile(561, 137 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("darkOakLog")->setAllFacesTexture("stripped_dark_oak_log", 137 | Tile::TEXTURE_ALT_FLAG);
	deadbush = (new FlowerTile(562, 138 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("deadbush")->setAllFacesTexture("deadbush", 138 | Tile::TEXTURE_ALT_FLAG);
	decoratedPot = (new Tile(563, 139 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("decoratedPot")->setAllFacesTexture("decorated_pot_side", 139 | Tile::TEXTURE_ALT_FLAG);
	deepslateBrickSlab = (new SlabTile(564, 20 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateBrickSlab")->setAllFacesTexture("deepslate_bricks", 20 | Tile::TEXTURE_ALT_FLAG);
	deepslateBrickStairs = (new StairTile(565, 20 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateBrickStairs")->setAllFacesTexture("deepslate_bricks", 20 | Tile::TEXTURE_ALT_FLAG);
	deepslateBrickWall = (new FenceTile(566, 20 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateBrickWall")->setAllFacesTexture("deepslate_bricks", 20 | Tile::TEXTURE_ALT_FLAG);
	deepslateTileDoubleSlab = (new Tile(567, 19 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateTileDoubleSlab")->setAllFacesTexture("deepslate_tiles", 19 | Tile::TEXTURE_ALT_FLAG);
	deepslateTileSlab = (new SlabTile(568, 19 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateTileSlab")->setAllFacesTexture("deepslate_tiles", 19 | Tile::TEXTURE_ALT_FLAG);
	deepslateTileStairs = (new StairTile(569, 19 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateTileStairs")->setAllFacesTexture("deepslate_tiles", 19 | Tile::TEXTURE_ALT_FLAG);
	deepslateTileWall = (new FenceTile(570, 19 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("deepslateTileWall")->setAllFacesTexture("deepslate_tiles", 19 | Tile::TEXTURE_ALT_FLAG);
	dioriteDoubleSlab = (new Tile(571, 58 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("dioriteDoubleSlab")->setAllFacesTexture("stone_diorite", 58 | Tile::TEXTURE_ALT_FLAG);
	dioriteStairs = (new StairTile(572, 58 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("dioriteStairs")->setAllFacesTexture("stone_diorite", 58 | Tile::TEXTURE_ALT_FLAG);
	dioriteWall = (new FenceTile(573, 58 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("dioriteWall")->setAllFacesTexture("stone_diorite", 58 | Tile::TEXTURE_ALT_FLAG);
	dragonHead = (new Tile(574, 140 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("dragonHead")->setAllFacesTexture("dragon_egg", 140 | Tile::TEXTURE_ALT_FLAG);
	endBricks = (new Tile(575, 141 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("endBricks")->setAllFacesTexture("end_bricks", 141 | Tile::TEXTURE_ALT_FLAG);
	endRod = (new Tile(576, 142 | Tile::TEXTURE_ALT_FLAG, Material::glass))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("endRod")->setLightEmission(14 / 16.0f)->setAllFacesTexture("end_rod", 142 | Tile::TEXTURE_ALT_FLAG);
	enderChest = (new Tile(577, 143 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("enderChest")->setAllFacesTexture("ender_chest_front", 143 | Tile::TEXTURE_ALT_FLAG);
	fern = (new FlowerTile(578, 144 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("fern")->setAllFacesTexture("fern", 144 | Tile::TEXTURE_ALT_FLAG);
	fletchingTable = (new Tile(579, 145 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("fletchingTable")->setAllFacesTexture("fletching_table_front", 145 | Tile::TEXTURE_ALT_FLAG);
	flowerPot = (new FlowerTile(580, 146 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("flowerPot")->setAllFacesTexture("flower_pot", 146 | Tile::TEXTURE_ALT_FLAG);
	gildedBlackstone = (new Tile(581, 147 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("gildedBlackstone")->setAllFacesTexture("gilded_blackstone", 147 | Tile::TEXTURE_ALT_FLAG);
	graniteStairs = (new StairTile(582, 56 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("graniteStairs")->setAllFacesTexture("stone_granite", 56 | Tile::TEXTURE_ALT_FLAG);
	graniteWall = (new FenceTile(583, 56 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("graniteWall")->setAllFacesTexture("stone_granite", 56 | Tile::TEXTURE_ALT_FLAG);
	grassPath = (new Tile(584, 148 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("grassPath")->setAllFacesTexture("grass_path_side", 148 | Tile::TEXTURE_ALT_FLAG);
	grayCarpet = (new Tile(585, 149 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("grayCarpet")->setAllFacesTexture("wool_colored_gray", 149 | Tile::TEXTURE_ALT_FLAG);
	greenCarpet = (new Tile(586, 150 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("greenCarpet")->setAllFacesTexture("wool_colored_green", 150 | Tile::TEXTURE_ALT_FLAG);
	grindstone = (new Tile(587, 151 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("grindstone")->setAllFacesTexture("grindstone_pivot", 151 | Tile::TEXTURE_ALT_FLAG);
	hayBlock = (new Tile(588, 152 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("hayBlock")->setAllFacesTexture("hay_block_side", 152 | Tile::TEXTURE_ALT_FLAG);
	hopper = (new Tile(589, 153 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::FoodArmor)->setDescriptionId("hopper")->setAllFacesTexture("hopper_top", 153 | Tile::TEXTURE_ALT_FLAG);
	infestedChiseledStoneBricks = (new Tile(590, 16, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("infestedChiseledStoneBricks")->setAllFacesTexture("stone_brick", 16);
	infestedCobblestone = (new Tile(591, 130 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("infestedCobblestone")->setAllFacesTexture("cobblestone", 130 | Tile::TEXTURE_ALT_FLAG);
	infestedMossyStoneBricks = (new Tile(592, 100, Material::plant))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("infestedMossyStoneBricks")->setAllFacesTexture("stonebrick_mossy", 100);
	infestedStoneBricks = (new Tile(593, 16, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("infestedStoneBricks")->setAllFacesTexture("stone_brick", 16);
	ironBars = (new ThinFenceTile(594, 154 | Tile::TEXTURE_ALT_FLAG, 154 | Tile::TEXTURE_ALT_FLAG, Material::metal, true))->init()->setDestroyTime(5.0f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("ironBars")->setAllFacesTexture("iron_bars", 154 | Tile::TEXTURE_ALT_FLAG);
	ironChain = (new Tile(595, 155 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("ironChain")->setAllFacesTexture("chain", 155 | Tile::TEXTURE_ALT_FLAG);
	ironTrapdoor = (new TrapDoorTile(596, 156 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(5.0f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("ironTrapdoor")->setAllFacesTexture("iron_trapdoor", 156 | Tile::TEXTURE_ALT_FLAG);
	jigsaw = (new Tile(597, 157 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("jigsaw")->setAllFacesTexture("jigsaw_side", 157 | Tile::TEXTURE_ALT_FLAG);
	jungleButton = (new Tile(598, 36 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("jungleButton")->setAllFacesTexture("planks_jungle", 36 | Tile::TEXTURE_ALT_FLAG);
	jungleDoor = (new DoorTile(599, 158 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("jungleDoor")->setAllFacesTexture("door_jungle_lower", 158 | Tile::TEXTURE_ALT_FLAG);
	jungleTrapdoor = (new TrapDoorTile(600, 159 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("jungleTrapdoor")->setAllFacesTexture("jungle_trapdoor", 159 | Tile::TEXTURE_ALT_FLAG);
	lantern = (new Tile(601, 160 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("lantern")->setLightEmission(1.0f)->setAllFacesTexture("lantern", 160 | Tile::TEXTURE_ALT_FLAG);
	largeFern = (new FlowerTile(602, 161 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("largeFern")->setAllFacesTexture("double_plant_fern_top", 161 | Tile::TEXTURE_ALT_FLAG);
	lectern = (new Tile(603, 162 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("lectern")->setAllFacesTexture("lectern_base", 162 | Tile::TEXTURE_ALT_FLAG);
	lightBlueCarpet = (new Tile(604, 163 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("lightBlueCarpet")->setAllFacesTexture("wool_colored_light_blue", 163 | Tile::TEXTURE_ALT_FLAG);
	lightBlueGlazedTerracotta = (new Tile(605, 164 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("lightBlueGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_light_blue", 164 | Tile::TEXTURE_ALT_FLAG);
	lightGrayCarpet = (new Tile(606, 165 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("lightGrayCarpet")->setAllFacesTexture("wool_colored_silver", 165 | Tile::TEXTURE_ALT_FLAG);
	lightGrayStainedGlass = (new Tile(607, 49, Material::glass))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("lightGrayStainedGlass")->setAllFacesTexture("glass", 49);
	limeCarpet = (new Tile(608, 166 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("limeCarpet")->setAllFacesTexture("wool_colored_lime", 166 | Tile::TEXTURE_ALT_FLAG);
	limeGlazedTerracotta = (new Tile(609, 167 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("limeGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_lime", 167 | Tile::TEXTURE_ALT_FLAG);
	litRedstoneLamp = (new Tile(610, 168 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("litRedstoneLamp")->setLightEmission(1.0f)->setAllFacesTexture("redstone_lamp_on", 168 | Tile::TEXTURE_ALT_FLAG);
	loom = (new Tile(611, 169 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("loom")->setAllFacesTexture("eyeblossom_blooming", 169 | Tile::TEXTURE_ALT_FLAG);
	magentaCarpet = (new Tile(612, 170 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("magentaCarpet")->setAllFacesTexture("wool_colored_magenta", 170 | Tile::TEXTURE_ALT_FLAG);
	magentaStainedGlass = (new Tile(613, 49, Material::glass))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("magentaStainedGlass")->setAllFacesTexture("glass", 49);
	magma = (new Tile(614, 171 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("magma")->setLightEmission(3 / 16.0f)->setAllFacesTexture("magma", 171 | Tile::TEXTURE_ALT_FLAG);
	mangroveLeaves = (new Tile(615, 172 | Tile::TEXTURE_ALT_FLAG, Material::leaves))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mangroveLeaves")->setAllFacesTexture("mangrove_leaves", 172 | Tile::TEXTURE_ALT_FLAG);
	mangroveLog = (new Tile(616, 173 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("mangroveLog")->setAllFacesTexture("mangrove_log_side", 173 | Tile::TEXTURE_ALT_FLAG);
	mangroveRoots = (new FlowerTile(617, 174 | Tile::TEXTURE_ALT_FLAG))->init()->setCategory(ItemCategory::Decorations)->setDescriptionId("mangroveRoots")->setAllFacesTexture("mangrove_roots_side", 174 | Tile::TEXTURE_ALT_FLAG);
	mangroveWood = (new Tile(618, 173 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("mangroveWood")->setAllFacesTexture("mangrove_log_side", 173 | Tile::TEXTURE_ALT_FLAG);
	mossBlock = (new Tile(619, 175 | Tile::TEXTURE_ALT_FLAG, Material::plant))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mossBlock")->setAllFacesTexture("moss_block", 175 | Tile::TEXTURE_ALT_FLAG);
	mossCarpet = (new Tile(620, 176 | Tile::TEXTURE_ALT_FLAG, Material::plant))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mossCarpet")->setAllFacesTexture("pale_moss_carpet_side_base", 176 | Tile::TEXTURE_ALT_FLAG);
	mossyCobblestoneSlab = (new SlabTile(621, 36, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("mossyCobblestoneSlab")->setAllFacesTexture("mossy_cobblestone", 36);
	mossyCobblestoneStairs = (new StairTile(622, 36, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("mossyCobblestoneStairs")->setAllFacesTexture("mossy_cobblestone", 36);
	mossyCobblestoneWall = (new FenceTile(623, 36, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("mossyCobblestoneWall")->setAllFacesTexture("mossy_cobblestone", 36);
	mossyStoneBricks = (new Tile(624, 100, Material::plant))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mossyStoneBricks")->setAllFacesTexture("stonebrick_mossy", 100);
	mud = (new Tile(625, 177 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mud")->setAllFacesTexture("mud", 177 | Tile::TEXTURE_ALT_FLAG);
	mudBrickSlab = (new SlabTile(626, 178 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mudBrickSlab")->setAllFacesTexture("mud_bricks", 178 | Tile::TEXTURE_ALT_FLAG);
	mudBrickStairs = (new StairTile(627, 178 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mudBrickStairs")->setAllFacesTexture("mud_bricks", 178 | Tile::TEXTURE_ALT_FLAG);
	mudBrickWall = (new FenceTile(628, 178 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mudBrickWall")->setAllFacesTexture("mud_bricks", 178 | Tile::TEXTURE_ALT_FLAG);
	mudBricks = (new Tile(629, 178 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("mudBricks")->setAllFacesTexture("mud_bricks", 178 | Tile::TEXTURE_ALT_FLAG);
	muddyMangroveRoots = (new Tile(630, 177 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("muddyMangroveRoots")->setAllFacesTexture("mud", 177 | Tile::TEXTURE_ALT_FLAG);
	noteblock = (new Tile(631, 179 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("noteblock")->setAllFacesTexture("noteblock", 179 | Tile::TEXTURE_ALT_FLAG);
	orangeCarpet = (new Tile(632, 180 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("orangeCarpet")->setAllFacesTexture("wool_colored_orange", 180 | Tile::TEXTURE_ALT_FLAG);
	orangeGlazedTerracotta = (new Tile(633, 181 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("orangeGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_orange", 181 | Tile::TEXTURE_ALT_FLAG);
	orangeStainedGlassPane = (new ThinFenceTile(634, 49, 49, Material::glass, false))->init()->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Structures)->setDescriptionId("orangeStainedGlassPane")->setAllFacesTexture("glass", 49);
	oxidizedCopperTrapdoor = (new TrapDoorTile(635, 182 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(5.0f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("oxidizedCopperTrapdoor")->setAllFacesTexture("oxidized_copper_trapdoor", 182 | Tile::TEXTURE_ALT_FLAG);
	oxidizedCutCopper = (new Tile(636, 183 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("oxidizedCutCopper")->setAllFacesTexture("oxidized_cut_copper", 183 | Tile::TEXTURE_ALT_FLAG);
	packedMud = (new Tile(637, 184 | Tile::TEXTURE_ALT_FLAG, Material::dirt))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("packedMud")->setAllFacesTexture("packed_mud", 184 | Tile::TEXTURE_ALT_FLAG);
	pinkCarpet = (new Tile(638, 185 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("pinkCarpet")->setAllFacesTexture("wool_colored_pink", 185 | Tile::TEXTURE_ALT_FLAG);
	pointedDripstone = (new Tile(639, 186 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("pointedDripstone")->setAllFacesTexture("pointed_dripstone_down_tip", 186 | Tile::TEXTURE_ALT_FLAG);
	polishedBasalt = (new Tile(640, 187 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("polishedBasalt")->setAllFacesTexture("polished_basalt_side", 187 | Tile::TEXTURE_ALT_FLAG);
	polishedBlackstoneBrickStairs = (new StairTile(641, 188 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("polishedBlackstoneBrickStairs")->setAllFacesTexture("polished_blackstone_bricks", 188 | Tile::TEXTURE_ALT_FLAG);
	polishedBlackstoneBricks = (new Tile(642, 188 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("polishedBlackstoneBricks")->setAllFacesTexture("polished_blackstone_bricks", 188 | Tile::TEXTURE_ALT_FLAG);
	polishedDeepslateSlab = (new SlabTile(643, 18 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("polishedDeepslateSlab")->setAllFacesTexture("polished_deepslate", 18 | Tile::TEXTURE_ALT_FLAG);
	polishedDeepslateStairs = (new StairTile(644, 18 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("polishedDeepslateStairs")->setAllFacesTexture("polished_deepslate", 18 | Tile::TEXTURE_ALT_FLAG);
	polishedDeepslateWall = (new FenceTile(645, 18 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("polishedDeepslateWall")->setAllFacesTexture("polished_deepslate", 18 | Tile::TEXTURE_ALT_FLAG);
	polishedTuff = (new Tile(646, 189 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("polishedTuff")->setAllFacesTexture("polished_tuff", 189 | Tile::TEXTURE_ALT_FLAG);
	polishedTuffSlab = (new SlabTile(647, 189 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("polishedTuffSlab")->setAllFacesTexture("polished_tuff", 189 | Tile::TEXTURE_ALT_FLAG);
	potentSulfur = (new Tile(648, 190 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("potentSulfur")->setAllFacesTexture("potent_sulfur", 190 | Tile::TEXTURE_ALT_FLAG);
	powderSnow = (new Tile(649, 191 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("powderSnow")->setAllFacesTexture("powder_snow", 191 | Tile::TEXTURE_ALT_FLAG);
	poweredComparator = (new Tile(650, 192 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("poweredComparator")->setAllFacesTexture("comparator_on", 192 | Tile::TEXTURE_ALT_FLAG);
	poweredRepeater = (new Tile(651, 193 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("poweredRepeater")->setAllFacesTexture("repeater_on", 193 | Tile::TEXTURE_ALT_FLAG);
	prismarine = (new Tile(652, 194 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("prismarine")->setAllFacesTexture("prismarine_bricks", 194 | Tile::TEXTURE_ALT_FLAG);
	purpleCarpet = (new Tile(653, 195 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("purpleCarpet")->setAllFacesTexture("wool_colored_purple", 195 | Tile::TEXTURE_ALT_FLAG);
	purpleGlazedTerracotta = (new Tile(654, 196 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("purpleGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_purple", 196 | Tile::TEXTURE_ALT_FLAG);
	purpurBlock = (new Tile(655, 197 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("purpurBlock")->setAllFacesTexture("purpur_block", 197 | Tile::TEXTURE_ALT_FLAG);
	purpurPillar = (new Tile(656, 198 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("purpurPillar")->setAllFacesTexture("purpur_pillar", 198 | Tile::TEXTURE_ALT_FLAG);
	purpurSlab = (new SlabTile(657, 197 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("purpurSlab")->setAllFacesTexture("purpur_block", 197 | Tile::TEXTURE_ALT_FLAG);
	purpurStairs = (new StairTile(658, 197 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("purpurStairs")->setAllFacesTexture("purpur_block", 197 | Tile::TEXTURE_ALT_FLAG);
	redCandle = (new Tile(659, 199 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("redCandle")->setLightEmission(3 / 16.0f)->setAllFacesTexture("red_candle", 199 | Tile::TEXTURE_ALT_FLAG);
	redCarpet = (new Tile(660, 200 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("redCarpet")->setAllFacesTexture("wool_colored_red", 200 | Tile::TEXTURE_ALT_FLAG);
	redConcrete = (new Tile(661, 201 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("redConcrete")->setAllFacesTexture("concrete_red", 201 | Tile::TEXTURE_ALT_FLAG);
	redGlazedTerracotta = (new Tile(662, 202 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("redGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_red", 202 | Tile::TEXTURE_ALT_FLAG);
	redstoneBlock = (new Tile(663, 203 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("redstoneBlock")->setAllFacesTexture("redstone_block", 203 | Tile::TEXTURE_ALT_FLAG);
	redstoneLamp = (new Tile(664, 204 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("redstoneLamp")->setAllFacesTexture("redstone_lamp_off", 204 | Tile::TEXTURE_ALT_FLAG);
	redstoneTorch = (new Tile(665, 205 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("redstoneTorch")->setAllFacesTexture("redstone_torch_on", 205 | Tile::TEXTURE_ALT_FLAG);
	redstoneWire = (new Tile(666, 206 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("redstoneWire")->setAllFacesTexture("redstone_dust_cross", 206 | Tile::TEXTURE_ALT_FLAG);
	reinforcedDeepslate = (new Tile(667, 6 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("reinforcedDeepslate")->setAllFacesTexture("deepslate", 6 | Tile::TEXTURE_ALT_FLAG);
	sandstoneWall = (new FenceTile(668, 192, Material::sand))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("sandstoneWall")->setAllFacesTexture("sandstone_normal", 192);
	sculkSensor = (new Tile(669, 207 | Tile::TEXTURE_ALT_FLAG, Material::plant))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("sculkSensor")->setLightEmission(1 / 16.0f)->setAllFacesTexture("calibrated_sculk_sensor_amethyst", 207 | Tile::TEXTURE_ALT_FLAG);
	seaLantern = (new Tile(670, 208 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("seaLantern")->setLightEmission(1.0f)->setAllFacesTexture("sea_lantern", 208 | Tile::TEXTURE_ALT_FLAG);
	seaPickle = (new Tile(671, 209 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("seaPickle")->setLightEmission(6 / 16.0f)->setAllFacesTexture("sea_pickle", 209 | Tile::TEXTURE_ALT_FLAG);
	silverGlazedTerracotta = (new Tile(672, 210 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("silverGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_silver", 210 | Tile::TEXTURE_ALT_FLAG);
	skeletonSkull = (new Tile(673, 25 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("skeletonSkull")->setAllFacesTexture("soul_sand", 25 | Tile::TEXTURE_ALT_FLAG);
	smithingTable = (new Tile(674, 211 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("smithingTable")->setAllFacesTexture("smithing_table_bottom", 211 | Tile::TEXTURE_ALT_FLAG);
	smoker = (new Tile(675, 212 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("smoker")->setAllFacesTexture("smoker_bottom", 212 | Tile::TEXTURE_ALT_FLAG);
	smoothBasalt = (new Tile(676, 213 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("smoothBasalt")->setAllFacesTexture("smooth_basalt", 213 | Tile::TEXTURE_ALT_FLAG);
	smoothQuartz = (new Tile(677, 214 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("smoothQuartz")->setAllFacesTexture("quartz_block_bottom", 214 | Tile::TEXTURE_ALT_FLAG);
	smoothQuartzSlab = (new SlabTile(678, 214 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("smoothQuartzSlab")->setAllFacesTexture("quartz_block_bottom", 214 | Tile::TEXTURE_ALT_FLAG);
	smoothSandstone = (new Tile(679, 230, Material::sand))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("smoothSandstone")->setAllFacesTexture("sandstone_smooth", 230);
	smoothSandstoneDoubleSlab = (new Tile(680, 176, Material::sand))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("smoothSandstoneDoubleSlab")->setAllFacesTexture("sandstone_top", 176);
	smoothSandstoneSlab = (new SlabTile(681, 176, Material::sand))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("smoothSandstoneSlab")->setAllFacesTexture("sandstone_top", 176);
	smoothSandstoneStairs = (new StairTile(682, 176, Material::sand))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_SAND)->setCategory(ItemCategory::Decorations)->setDescriptionId("smoothSandstoneStairs")->setAllFacesTexture("sandstone_top", 176);
	smoothStone = (new Tile(683, 215 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("smoothStone")->setAllFacesTexture("smooth_stone", 215 | Tile::TEXTURE_ALT_FLAG);
	smoothStoneDoubleSlab = (new Tile(684, 215 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("smoothStoneDoubleSlab")->setAllFacesTexture("smooth_stone", 215 | Tile::TEXTURE_ALT_FLAG);
	smoothStoneSlab = (new SlabTile(685, 215 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("smoothStoneSlab")->setAllFacesTexture("smooth_stone", 215 | Tile::TEXTURE_ALT_FLAG);
	soulFire = (new Tile(686, 216 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("soulFire")->setAllFacesTexture("soul_fire_0", 216 | Tile::TEXTURE_ALT_FLAG);
	soulLantern = (new Tile(687, 217 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("soulLantern")->setLightEmission(10 / 16.0f)->setAllFacesTexture("soul_lantern", 217 | Tile::TEXTURE_ALT_FLAG);
	spruceDoor = (new DoorTile(688, 218 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("spruceDoor")->setAllFacesTexture("door_spruce_lower", 218 | Tile::TEXTURE_ALT_FLAG);
	sprucePressurePlate = (new Tile(689, 4 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("sprucePressurePlate")->setAllFacesTexture("planks_spruce", 4 | Tile::TEXTURE_ALT_FLAG);
	spruceTrapdoor = (new TrapDoorTile(690, 219 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(3.0f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("spruceTrapdoor")->setAllFacesTexture("spruce_trapdoor", 219 | Tile::TEXTURE_ALT_FLAG);
	spruceWallSign = (new Tile(691, 4 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Structures)->setDescriptionId("spruceWallSign")->setAllFacesTexture("planks_spruce", 4 | Tile::TEXTURE_ALT_FLAG);
	spruceWood = (new Tile(692, 0 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("spruceWood")->setAllFacesTexture("log_spruce_side", 0 | Tile::TEXTURE_ALT_FLAG);
	stickyPiston = (new Tile(693, 220 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stickyPiston")->setAllFacesTexture("piston_top_sticky", 220 | Tile::TEXTURE_ALT_FLAG);
	stickyPistonArmCollision = (new Tile(694, 220 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stickyPistonArmCollision")->setAllFacesTexture("piston_top_sticky", 220 | Tile::TEXTURE_ALT_FLAG);
	stoneBrickWall = (new FenceTile(695, 16, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stoneBrickWall")->setAllFacesTexture("stone_brick", 16);
	stoneButton = (new Tile(696, 1, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stoneButton")->setAllFacesTexture("stone", 1);
	stonePressurePlate = (new Tile(697, 1, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("stonePressurePlate")->setAllFacesTexture("stone", 1);
	strippedAcaciaLog = (new Tile(698, 221 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("strippedAcaciaLog")->setAllFacesTexture("stripped_acacia_log", 221 | Tile::TEXTURE_ALT_FLAG);
	strippedOakLog = (new Tile(699, 222 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("strippedOakLog")->setAllFacesTexture("stripped_oak_log", 222 | Tile::TEXTURE_ALT_FLAG);
	strippedOakWood = (new Tile(700, 222 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("strippedOakWood")->setAllFacesTexture("stripped_oak_log", 222 | Tile::TEXTURE_ALT_FLAG);
	strippedSpruceLog = (new Tile(701, 223 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("strippedSpruceLog")->setAllFacesTexture("stripped_spruce_log", 223 | Tile::TEXTURE_ALT_FLAG);
	strippedSpruceWood = (new Tile(702, 223 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("strippedSpruceWood")->setAllFacesTexture("stripped_spruce_log", 223 | Tile::TEXTURE_ALT_FLAG);
	structureBlock = (new Tile(703, 224 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("structureBlock")->setAllFacesTexture("structure_block_corner", 224 | Tile::TEXTURE_ALT_FLAG);
	sulfur = (new Tile(704, 18, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("sulfur")->setAllFacesTexture("sand", 18);
	sulfurSpike = (new Tile(705, 176, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("sulfurSpike")->setAllFacesTexture("sandstone_top", 176);
	tallGrass = (new Tile(706, 225 | Tile::TEXTURE_ALT_FLAG, Material::plant))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GRASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("tallGrass")->setAllFacesTexture("double_plant_grass_top", 225 | Tile::TEXTURE_ALT_FLAG);
	target = (new Tile(707, 226 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("target")->setAllFacesTexture("target_side", 226 | Tile::TEXTURE_ALT_FLAG);
	trappedChest = (new Tile(708, 227 | Tile::TEXTURE_ALT_FLAG, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("trappedChest")->setAllFacesTexture("trapped_chest_front", 227 | Tile::TEXTURE_ALT_FLAG);
	trialSpawner = (new Tile(709, 228 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("trialSpawner")->setAllFacesTexture("trial_spawner_bottom", 228 | Tile::TEXTURE_ALT_FLAG);
	tripWire = (new Tile(710, 229 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("tripWire")->setAllFacesTexture("trip_wire", 229 | Tile::TEXTURE_ALT_FLAG);
	tripwireHook = (new Tile(711, 230 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("tripwireHook")->setAllFacesTexture("trip_wire_source", 230 | Tile::TEXTURE_ALT_FLAG);
	tuff = (new Tile(712, 125 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("tuff")->setAllFacesTexture("tuff", 125 | Tile::TEXTURE_ALT_FLAG);
	tuffBricks = (new Tile(713, 126 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("tuffBricks")->setAllFacesTexture("tuff_bricks", 126 | Tile::TEXTURE_ALT_FLAG);
	unlitRedstoneTorch = (new Tile(714, 231 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("unlitRedstoneTorch")->setLightEmission(1.0f)->setAllFacesTexture("redstone_torch_off", 231 | Tile::TEXTURE_ALT_FLAG);
	unpoweredComparator = (new Tile(715, 232 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("unpoweredComparator")->setAllFacesTexture("comparator_off", 232 | Tile::TEXTURE_ALT_FLAG);
	unpoweredRepeater = (new Tile(716, 233 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("unpoweredRepeater")->setAllFacesTexture("repeater_off", 233 | Tile::TEXTURE_ALT_FLAG);
	vault = (new Tile(717, 234 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Decorations)->setDescriptionId("vault")->setAllFacesTexture("vault_bottom", 234 | Tile::TEXTURE_ALT_FLAG);
	wallBanner = (new Tile(718, 235 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("wallBanner")->setAllFacesTexture("wool_colored_white", 235 | Tile::TEXTURE_ALT_FLAG);
	waxedChiseledCopper = (new Tile(719, 236 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedChiseledCopper")->setAllFacesTexture("chiseled_copper", 236 | Tile::TEXTURE_ALT_FLAG);
	waxedCopper = (new Tile(720, 132 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedCopper")->setAllFacesTexture("copper_block", 132 | Tile::TEXTURE_ALT_FLAG);
	waxedCopperBulb = (new Tile(721, 237 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedCopperBulb")->setLightEmission(1.0f)->setAllFacesTexture("copper_bulb", 237 | Tile::TEXTURE_ALT_FLAG);
	waxedCopperDoor = (new DoorTile(722, 238 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(5.0f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedCopperDoor")->setAllFacesTexture("copper_door_bottom", 238 | Tile::TEXTURE_ALT_FLAG);
	waxedCopperGrate = (new Tile(723, 239 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedCopperGrate")->setAllFacesTexture("copper_grate", 239 | Tile::TEXTURE_ALT_FLAG);
	waxedCutCopper = (new Tile(724, 240 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedCutCopper")->setAllFacesTexture("cut_copper", 240 | Tile::TEXTURE_ALT_FLAG);
	waxedCutCopperSlab = (new SlabTile(725, 240 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedCutCopperSlab")->setAllFacesTexture("cut_copper", 240 | Tile::TEXTURE_ALT_FLAG);
	waxedCutCopperStairs = (new StairTile(726, 240 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedCutCopperStairs")->setAllFacesTexture("cut_copper", 240 | Tile::TEXTURE_ALT_FLAG);
	waxedOxidizedChiseledCopper = (new Tile(727, 241 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedOxidizedChiseledCopper")->setAllFacesTexture("oxidized_chiseled_copper", 241 | Tile::TEXTURE_ALT_FLAG);
	waxedOxidizedCopper = (new Tile(728, 242 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedOxidizedCopper")->setAllFacesTexture("oxidized_copper", 242 | Tile::TEXTURE_ALT_FLAG);
	waxedOxidizedCopperDoor = (new DoorTile(729, 243 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(5.0f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedOxidizedCopperDoor")->setAllFacesTexture("oxidized_copper_door_bottom", 243 | Tile::TEXTURE_ALT_FLAG);
	waxedOxidizedCopperGrate = (new Tile(730, 244 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedOxidizedCopperGrate")->setAllFacesTexture("oxidized_copper_grate", 244 | Tile::TEXTURE_ALT_FLAG);
	waxedOxidizedCopperTrapdoor = (new TrapDoorTile(731, 182 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(5.0f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedOxidizedCopperTrapdoor")->setAllFacesTexture("oxidized_copper_trapdoor", 182 | Tile::TEXTURE_ALT_FLAG);
	waxedOxidizedCutCopper = (new Tile(732, 183 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedOxidizedCutCopper")->setAllFacesTexture("oxidized_cut_copper", 183 | Tile::TEXTURE_ALT_FLAG);
	waxedOxidizedCutCopperSlab = (new SlabTile(733, 183 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedOxidizedCutCopperSlab")->setAllFacesTexture("oxidized_cut_copper", 183 | Tile::TEXTURE_ALT_FLAG);
	waxedOxidizedCutCopperStairs = (new StairTile(734, 183 | Tile::TEXTURE_ALT_FLAG, Material::metal))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_METAL)->setCategory(ItemCategory::Structures)->setDescriptionId("waxedOxidizedCutCopperStairs")->setAllFacesTexture("oxidized_cut_copper", 183 | Tile::TEXTURE_ALT_FLAG);
	whiteCandle = (new Tile(735, 245 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("whiteCandle")->setLightEmission(3 / 16.0f)->setAllFacesTexture("white_candle", 245 | Tile::TEXTURE_ALT_FLAG);
	whiteCarpet = (new Tile(736, 235 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("whiteCarpet")->setAllFacesTexture("wool_colored_white", 235 | Tile::TEXTURE_ALT_FLAG);
	whiteConcrete = (new Tile(737, 246 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("whiteConcrete")->setAllFacesTexture("concrete_white", 246 | Tile::TEXTURE_ALT_FLAG);
	whiteGlazedTerracotta = (new Tile(738, 247 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("whiteGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_white", 247 | Tile::TEXTURE_ALT_FLAG);
	whiteStainedGlass = (new Tile(739, 49, Material::glass))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Decorations)->setDescriptionId("whiteStainedGlass")->setAllFacesTexture("glass", 49);
	whiteStainedGlassPane = (new ThinFenceTile(740, 49, 49, Material::glass, false))->init()->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Structures)->setDescriptionId("whiteStainedGlassPane")->setAllFacesTexture("glass", 49);
	woodenButton = (new Tile(741, 4, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("woodenButton")->setAllFacesTexture("planks_oak", 4);
	woodenPressurePlate = (new Tile(742, 4, Material::wood))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_WOOD)->setCategory(ItemCategory::Decorations)->setDescriptionId("woodenPressurePlate")->setAllFacesTexture("planks_oak", 4);
	yellowCarpet = (new Tile(743, 248 | Tile::TEXTURE_ALT_FLAG, Material::cloth))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_CLOTH)->setCategory(ItemCategory::Decorations)->setDescriptionId("yellowCarpet")->setAllFacesTexture("wool_colored_yellow", 248 | Tile::TEXTURE_ALT_FLAG);
	yellowGlazedTerracotta = (new Tile(744, 249 | Tile::TEXTURE_ALT_FLAG, Material::stone))->init()->setDestroyTime(1.5f)->setSoundType(SOUND_STONE)->setCategory(ItemCategory::Structures)->setDescriptionId("yellowGlazedTerracotta")->setAllFacesTexture("glazed_terracotta_yellow", 249 | Tile::TEXTURE_ALT_FLAG);
	yellowStainedGlassPane = (new ThinFenceTile(745, 49, 49, Material::glass, false))->init()->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setCategory(ItemCategory::Structures)->setDescriptionId("yellowStainedGlassPane")->setAllFacesTexture("glass", 49);
}

bool Tile::isTileAllowedInCreative(int id) {
	if (id <= 0 || id >= NUM_BLOCK_TYPES || Tile::tiles[id] == NULL)
		return false;

	switch (id) {
	// Liquids & Fire (use buckets / flint and steel)
	case 8:   // water
	case 9:   // calmWater
	case 10:  // lava
	case 11:  // calmLava
	case 51:  // fire

	// Blocks with dedicated placing items
	case 26:  // bed (placed by Item::bed)
	case 63:  // sign (placed by Item::sign)
	case 64:  // door_wood (placed by Item::door_wood)
	case 68:  // wallSign (placed by Item::sign)
	case 71:  // door_iron (placed by Item::door_iron)
	case 83:  // reeds (placed by Item::reeds)

	// Double slabs (use half-slabs in creative inventory)
	case 43:  // stoneSlab
	case 157: // spruceSlab
	case 159: // birchSlab
	case 199: // jungleSlab
	case 204: // acaciaSlab
	case 209: // darkOakSlab
	case 213: // sandstoneSlab
	case 215: // oakSlab
	case 217: // cobbleSlab
	case 219: // brickSlab
	case 221: // stoneBrickSlab
	case 223: // netherBrickSlab
	case 225: // quartzSlab

	// Crops / Growth stages / Farmland (use seeds)
	case 59:  // crops (placed by Item::seeds_wheat)
	case 60:  // farmland
	case 105: // melonStem (placed by Item::seeds_melon)
	case 115: // netherStalk (placed by Item::netherWart)

	// Lit / Active variants
	case 62:  // furnace_lit (use furnace)
	case 74:  // redStoneOre_lit (use redStoneOre)
	case 178: // deepslateRedstoneOre_lit (use deepslateRedstoneOre)

	// Portals, invisible barrier & internal engine blocks
	case 95:  // invisible_bedrock
	case 246: // classicPortal
	case 248: // info_updateGame1
	case 249: // info_updateGame2
	case 253: // grass_carried
	case 254: // leaves_carried
	case 255: // info_reserved6
		return false;

	default:
		return true;
	}
}

int Tile::transformToValidBlockId( int blockId ) {
    return transformToValidBlockId(blockId, 0, 0, 0);
}
int Tile::transformToValidBlockId( int blockId, int x, int y, int z ) {
	if (blockId < 0 || blockId >= NUM_BLOCK_TYPES)
		return 0;
	if ((Tile::info_updateGame1 && blockId == Tile::info_updateGame1->id) ||
		(Tile::info_updateGame2 && blockId == Tile::info_updateGame2->id))
		return 0;
	if (blockId != 0 && Tile::tiles[blockId] == NULL)
		return 0;
	return blockId;
}

int Tile::getOreVariant(int oreTileId, int replacedTileId) {
	if (replacedTileId != Tile::deepslate->id) {
		return oreTileId;
	}
    
    // Si estamos reemplazando Deepslate, devolvemos la variante de Deepslate del mineral
	if (oreTileId == Tile::coalOre->id) return Tile::deepslateCoalOre->id;
	if (oreTileId == Tile::ironOre->id) return Tile::deepslateIronOre->id;
	if (oreTileId == Tile::goldOre->id) return Tile::deepslateGoldOre->id;
	if (oreTileId == Tile::emeraldOre->id) return Tile::deepslateDiamondOre->id; // En este motor EmeraldOre se usa para Diamond
	if (oreTileId == Tile::lapisOre->id) return Tile::deepslateLapisOre->id;
	if (oreTileId == Tile::redStoneOre->id) return Tile::deepslateRedstoneOre->id;

	return oreTileId;
}

Tile::Tile(int id, const Material* material)
:	id(id),
	material(material),
	tex(1),
	category(-1),
	gravity(1.0f),
	friction(0.6f),
	soundType(&Tile::SOUND_NORMAL),
	tmpBB(0,0,0,1,1,1),
	xx0(0),yy0(0),zz0(0),
	xx1(1),yy1(1),zz1(1),
	useMaterialInstances(false)
{
	if (Tile::tiles[id]) {
		printf("Slot %d is already occupied by %p when adding %p\n", id, &Tile::tiles[id], this);
	}
}

Tile::Tile( int id, int tex, const Material* material )
:	id(id),
	tex(tex),
	material(material),
	category(-1),
	gravity(1.0f),
	friction(0.6f),
	soundType(&Tile::SOUND_NORMAL),
	tmpBB(0,0,0,1,1,1),
	xx0(0),yy0(0),zz0(0),
	xx1(1),yy1(1),zz1(1),
	useMaterialInstances(false)
{
	if (Tile::tiles[id]) {
		printf("Slot %d is already occupied by %p when adding %p\n", id, &Tile::tiles[id], this);
	}
}

//Tile* sendTileData() {
//    Tile::sendTileData[id] = true;
//    return this;
//}

/*protected*/
Tile* Tile::setLightEmission(float f) {
    Tile::lightEmission[id] = (int) std::round(Level::MAX_BRIGHTNESS * f);
    return this;
}

/*public static*/
bool Tile::isFaceVisible(Level* level, int x, int y, int z, int f) {
	switch (f) {
		case Facing::DOWN : y--; break;
		case Facing::UP   : y++; break;
		case Facing::NORTH: z--; break;
		case Facing::SOUTH: z++; break;
		case Facing::WEST : x--; break;
		case Facing::EAST : x++; break;
	}
	return !level->isSolidRenderTile(x, y, z);
}

/* private */
Tile* Tile::init() {
    Tile::tiles[id] = this;
	setShape(xx0, yy0, zz0, xx1, yy1, zz1); // @attn
	// Forzamos que las plantas no sean sólidas en el array global para evitar fondos opacos
	bool isPlant = (id == 179 || id == 180 || id == 6 || id == 31 || id == 39 || id == 40);
	solid[id] = isPlant ? false : isSolidRender();
	lightBlock[id] = isSolidRender() ? 255 : 0;
	translucent[id] = !material->blocksLight();
	return this;
}

/*public virtual*/
float Tile::getDestroyProgress(Player* player) {
    if (destroySpeed < 0) return 0;
    if (!player->canDestroy(this)) return 1 / destroySpeed / 100.0f;
    return (player->getDestroySpeed(this) / destroySpeed) / 30.0f;
}

/*public virtual*/
HitResult Tile::clip(Level* level, int xt, int yt, int zt, const Vec3& A, const Vec3& B) {
    updateShape(level, xt, yt, zt);

	//Stopwatch sw;
	//sw.start();

	Vec3 sub((float)xt, (float)yt, (float)zt);
	Vec3 a = A - sub;//a.add((float)-xt, (float)-yt, (float)-zt);
    Vec3 b = B - sub;//b.add((float)-xt, (float)-yt, (float)-zt);

    Vec3 xh0, xh1, yh0, yh1, zh0, zh1;

	bool bxh0 = a.clipX(b, xx0, xh0);
    bool bxh1 = a.clipX(b, xx1, xh1);

    bool byh0 = a.clipY(b, yy0, yh0);
    bool byh1 = a.clipY(b, yy1, yh1);

    bool bzh0 = a.clipZ(b, zz0, zh0);
    bool bzh1 = a.clipZ(b, zz1, zh1);

    //if (!containsX(xh0)) xh0 = NULL;
    if (!bxh0 || !containsX(xh0)) bxh0 = false;
    if (!bxh1 || !containsX(xh1)) bxh1 = false;
    if (!byh0 || !containsY(yh0)) byh0 = false;
    if (!byh1 || !containsY(yh1)) byh1 = false;
    if (!bzh0 || !containsZ(zh0)) bzh0 = false;
    if (!bzh1 || !containsZ(zh1)) bzh1 = false;
    Vec3* closest = NULL;

    //if (xh0 != NULL && (closest == NULL || a.distanceToSqr(xh0) < a.distanceToSqr(closest))) closest = xh0;
    if (bxh0 && (closest == NULL || a.distanceToSqr(xh0) < a.distanceToSqr(*closest))) closest = &xh0;
    if (bxh1 && (closest == NULL || a.distanceToSqr(xh1) < a.distanceToSqr(*closest))) closest = &xh1;
    if (byh0 && (closest == NULL || a.distanceToSqr(yh0) < a.distanceToSqr(*closest))) closest = &yh0;
    if (byh1 && (closest == NULL || a.distanceToSqr(yh1) < a.distanceToSqr(*closest))) closest = &yh1;
    if (bzh0 && (closest == NULL || a.distanceToSqr(zh0) < a.distanceToSqr(*closest))) closest = &zh0;
    if (bzh1 && (closest == NULL || a.distanceToSqr(zh1) < a.distanceToSqr(*closest))) closest = &zh1;

    if (closest == NULL)
		return HitResult();

    int face = -1;

    if (closest == &xh0) face = 4;
    if (closest == &xh1) face = 5;
    if (closest == &yh0) face = 0;
    if (closest == &yh1) face = 1;
    if (closest == &zh0) face = 2;
    if (closest == &zh1) face = 3;

	//sw.stop();
	//sw.printEvery(5, ">>> ");

    return HitResult(xt, yt, zt, face, closest->add((float)xt, (float)yt, (float)zt));
}

/*virtual*/
void Tile::spawnResources(Level* level, int x, int y, int z, int data, float odds) {
    if (level->isClientSide) return;

    int count = getResourceCount(&level->random);
    for (int i = 0; i < count; i++) {
        if (level->random.nextFloat() > odds) continue;
        int type = getResource(data, &level->random);
        if (type <= 0) continue;
        const float s = 0.7f;
        float xo = level->random.nextFloat() * s + (1 - s) * 0.5f;
        float yo = level->random.nextFloat() * s + (1 - s) * 0.5f;
        float zo = level->random.nextFloat() * s + (1 - s) * 0.5f;
		ItemEntity* item = new ItemEntity(level, x + xo, y + yo, z + zo, ItemInstance(type, 1, getSpawnResourcesAuxValue(data)));
        item->throwTime = 10;
        level->addEntity(item);
    }
}

void Tile::spawnResources( Level* level, int x, int y, int z, int data )
{
	spawnResources(level, x, y, z, data, 1);
}

void Tile::popResource(Level* level, int x, int y, int z, const ItemInstance& itemInstance) {
	if (level->isClientSide || level->getLevelData()->getGameType() == GameType::Creative) return;

	float s = 0.7f;
	float xo = level->random.nextFloat() * s + (1 - s) * 0.5f;
	float yo = level->random.nextFloat() * s + (1 - s) * 0.5f;
	float zo = level->random.nextFloat() * s + (1 - s) * 0.5f;

	ItemEntity* item = new ItemEntity(level, x + xo, y + yo, z + zo, itemInstance);
	item->throwTime = 10;
	level->addEntity(item);
}


void Tile::destroy( Level* level, int x, int y, int z, int data )
{
}

bool Tile::isCubeShaped()
{
	// Las flores y plantas no deben ser tratadas como cubos para evitar problemas de oclusión
	if (id == 179 || id == 180 || id == 6 || id == 31 || id == 39 || id == 40) return false;
	return true;
}

int Tile::getRenderShape()
{
	// Forzamos el renderizado en forma de cruz para las flores
	if (id == 179 || id == 180 || id == 6 || id == 31 || id == 39 || id == 40) return SHAPE_CROSS_TEXTURE;
	return SHAPE_BLOCK;
}

float Tile::getBrightness( LevelSource* level, int x, int y, int z )
{
	return level->getBrightness(x, y, z);
}

bool Tile::shouldRenderFace( LevelSource* level, int x, int y, int z, int face )
{
	if (face == 0 && y == -1) return false;
	// For fixed size worlds //@todo: external constants rather than magic numbers
	if (face == 2 && z == -1)  return true;
	if (face == 3 && z == 256) return true;
	if (face == 4 && x == -1)  return true;
	if (face == 5 && x == 256) return true;
	// Common
	if (face == 0 && yy0 > 0) return true;
	if (face == 1 && yy1 < 1) return true;
	if (face == 2 && zz0 > 0) return true;
	if (face == 3 && zz1 < 1) return true;
	if (face == 4 && xx0 > 0) return true;
	if (face == 5 && xx1 < 1) return true;
	Tile* t = Tile::tiles[level->getTile(x, y, z)];
	if (!t) return true;
	if (face == 1 && t->id == topSnow->id) return false;
	return !t->isSolidRender();
	//return (!level->isSolidRenderTile(x, y, z));
}

int Tile::getTexture( LevelSource* level, int x, int y, int z, int face )
{
	return getTexture(face, level ? level->getData(x, y, z) : 0);
}

int Tile::getTexture( int face, int data )
{
	return getTexture(face);
}

int Tile::getTexture( int face )
{
	if (useMaterialInstances) {
		Tile::BlockFace bf = Tile::renderFaceToBlockFace(face);
		const MaterialInstance* mat = getMaterialInstance(bf);
		if (mat && mat->textureIndex >= 0) {
			return mat->textureIndex;
		}
	}
	return tex;
}

void Tile::addAABBs( Level* level, int x, int y, int z, const AABB* box, std::vector<AABB>& boxes )
{
	AABB* aabb = getAABB(level, x, y, z);
	if (aabb != NULL && box->intersects(*aabb)) {
		boxes.push_back(*aabb);
	}
}

static inline bool isPlantOrCutoutTile(int id) {
	return (id == 179 || id == 180 || id == 6 || id == 31 || id == 39 || id == 40 ||
	        id == 106 || id == 111 || id == 142 ||
	        (id >= 143 && id <= 149) || (id >= 167 && id <= 169) ||
	        id == 184 || id == 213 || (id >= 193 && id <= 197));
}

AABB* Tile::getAABB( Level* level, int x, int y, int z )
{
	// Waterlily (111) has a thin surface collision box
	if (id == 111) {
		tmpBB.x0 = x + 0.0f;
		tmpBB.y0 = y + 0.0f;
		tmpBB.z0 = z + 0.0f;
		tmpBB.x1 = x + 1.0f;
		tmpBB.y1 = y + 0.015625f;
		tmpBB.z1 = z + 1.0f;
		return &tmpBB;
	}

	// Flowers, tall grass, vines, saplings have no collision box
	if (isPlantOrCutoutTile(id)) {
		return NULL;
	}

	tmpBB.x0 = x + xx0;
	tmpBB.y0 = y + yy0;
	tmpBB.z0 = z + zz0;
	tmpBB.x1 = x + xx1;
	tmpBB.y1 = y + yy1;
	tmpBB.z1 = z + zz1;
	return &tmpBB;
}

bool Tile::isSolidRender()
{
	if (isPlantOrCutoutTile(id)) return false;
	return xx0 <= 0.0f && yy0 <= 0.0f && zz0 <= 0.0f && xx1 >= 1.0f && yy1 >= 1.0f && zz1 >= 1.0f;
}

bool Tile::mayPick( int data, bool liquid )
{
	return mayPick();
}

bool Tile::mayPick()
{
	return true;
}

int Tile::getTickDelay()
{
	return 10;
}

int Tile::getResourceCount( Random* random )
{
	return 1;
}

int Tile::getResource( int data, Random* random )
{
	return id;
}

float Tile::getExplosionResistance( Entity* source )
{
	return explosionResistance / 5.0f;
}

int Tile::getRenderLayer()
{
	if (isPlantOrCutoutTile(id)) return Tile::RENDERLAYER_ALPHATEST;
	return Tile::RENDERLAYER_OPAQUE;
}

bool Tile::use( Level* level, int x, int y, int z, Player* player )
{
	return false;
}

bool Tile::spawnBurnResources( Level* level, float x, float y, float z )
{
	return false;
}

int Tile::getColor( LevelSource* level, int x, int y, int z )
{
	return 0xffffff;
}

bool Tile::getSignal( LevelSource* level, int x, int y, int z )
{
	return false;
}

bool Tile::getSignal( LevelSource* level, int x, int y, int z, int dir )
{
	return false;
}

bool Tile::isSignalSource()
{
	return false;
}

bool Tile::getDirectSignal( Level* level, int x, int y, int z, int dir )
{
	return false;
}

void Tile::playerDestroy( Level* level, Player* player, int x, int y, int z, int data )
{
	//player.awardStat(Stats.blockMined[id], 1);
	spawnResources(level, x, y, z, data);
}

bool Tile::canSurvive( Level* level, int x, int y, int z )
{
	return true;
}

Tile* Tile::setDescriptionId( const std::string& id )
{
	descriptionId = TILE_DESCRIPTION_PREFIX + id;
	return this;
}

Tile* Tile::setCategory(int category)
{
	this->category = category;
	return this;
}

std::string Tile::getName() const
{
	return I18n::get(getDescriptionId() + ".name");
}

std::string Tile::getDescriptionId() const
{
	return descriptionId;
}

Tile* Tile::setSoundType( const SoundType& soundType )
{
	this->soundType = &soundType;
	return this;
}

Tile* Tile::setLightBlock( int i )
{
	lightBlock[id] = i;
	return this;
}

Tile* Tile::setExplodeable( float explosionResistance )
{
	this->explosionResistance = explosionResistance * 3;
	return this;
}

Tile* Tile::setDestroyTime( float destroySpeed )
{
	this->destroySpeed = destroySpeed;
	if (explosionResistance < destroySpeed * 5) explosionResistance = destroySpeed * 5;
	return this;
}

void Tile::setTicking( bool tick )
{
	shouldTick[id] = tick;
}

int Tile::getSpawnResourcesAuxValue( int data )
{
	return 0;
}

bool Tile::containsX( const Vec3& v )
{
	return v.y >= yy0 && v.y <= yy1 && v.z >= zz0 && v.z <= zz1;
}

bool Tile::containsY( const Vec3& v )
{
	return v.x >= xx0 && v.x <= xx1 && v.z >= zz0 && v.z <= zz1;
}

bool Tile::containsZ( const Vec3& v )
{
	return v.x >= xx0 && v.x <= xx1 && v.y >= yy0 && v.y <= yy1;
}

/*public*/
AABB Tile::getTileAABB(Level* level, int x, int y, int z) {
	return AABB(x + xx0, y + yy0, z + zz0, x + xx1, y + yy1, z + zz1);
}

/*public*/
void Tile::setShape(float x0, float y0, float z0, float x1, float y1, float z1) {
	this->xx0 = x0;
	this->yy0 = y0;
	this->zz0 = z0;
	this->xx1 = x1;
	this->yy1 = y1;
	this->zz1 = z1;
	// Mantener consistencia con el array de solidez
	bool isPlant = (id == 179 || id == 180 || id == 6 || id == 31 || id == 39 || id == 40);
	solid[id] = isPlant ? false : isSolidRender();
}

/*public*/
bool Tile::mayPlace(Level* level, int x, int y, int z, unsigned char face) {
	return mayPlace(level, x, y, z);
}

bool Tile::mayPlace( Level* level, int x, int y, int z ) {
	int t = level->getTile(x, y, z);
	bool replaceable = (t == 0 || !Tile::solid[t]);
	if (id == 179 || id == 180) return replaceable && canSurvive(level, x, y, z);
	return replaceable;
}
