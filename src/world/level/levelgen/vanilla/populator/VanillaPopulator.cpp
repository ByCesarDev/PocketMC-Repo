#include "VanillaPopulator.h"
#include "../../../LevelConstants.h"
#include "../biome/VanillaBiomeIds.h"
#include "../object/tree/VanillaTrees.h"
#include "../../feature/Feature.h"
#include "../../feature/TreeFeature.h"
#include "../../feature/BirchFeature.h"
#include "../../feature/PineFeature.h"
#include "../../feature/SpruceFeature.h"
#include "../../feature/OreFeature.h"
#include "../../feature/LakeFeature.h"
#include "../../feature/FlowerFeature.h"
#include "../../feature/TallgrassFeature.h"
#include "../../feature/CactusFeature.h"
#include "../../feature/ReedsFeature.h"
#include "../../../tile/Tile.h"

VanillaPopulator::VanillaPopulator() {}

void VanillaPopulator::populate(Level* level, int chunkX, int chunkZ, Random& rand, int biomeId)
{
	int bx = chunkX << 4;
	int bz = chunkZ << 4;

	// 1. Water & Lava Lakes
	if (rand.nextInt(4) == 0) {
		int lx = bx + rand.nextInt(16) + 8;
		int lz = bz + rand.nextInt(16) + 8;
		int ly = rand.nextInt(LEVEL_HEIGHT);
		LakeFeature waterLake(Tile::calmWater->id);
		waterLake.place(level, &rand, lx, ly, lz);
	}

	if (rand.nextInt(8) == 0) {
		int lx = bx + rand.nextInt(16) + 8;
		int lz = bz + rand.nextInt(16) + 8;
		int ly = rand.nextInt(rand.nextInt(LEVEL_HEIGHT - 16) + 8);
		if (ly < 64 || rand.nextInt(10) == 0) {
			LakeFeature lavaLake(Tile::calmLava->id);
			lavaLake.place(level, &rand, lx, ly, lz);
		}
	}

	// 2. Ores & Stones
	auto genOre = [&](int count, int tileId, int size, int minY, int maxY) {
		OreFeature ore(tileId, size);
		for (int i = 0; i < count; ++i) {
			int ox = bx + rand.nextInt(16);
			int oy = rand.nextInt(maxY - minY) + minY;
			int oz = bz + rand.nextInt(16);
			ore.place(level, &rand, ox, oy, oz);
		}
	};

	genOre(20, Tile::dirt->id, 32, 0, LEVEL_HEIGHT);
	genOre(10, Tile::gravel->id, 32, 0, LEVEL_HEIGHT);
	if (Tile::granite)  genOre(10, Tile::granite->id, 32, 0, 80);
	if (Tile::diorite)  genOre(10, Tile::diorite->id, 32, 0, 80);
	if (Tile::andesite) genOre(10, Tile::andesite->id, 32, 0, 80);
	genOre(20, Tile::coalOre->id, 16, 0, LEVEL_HEIGHT);
	genOre(20, Tile::ironOre->id, 8, 0, LEVEL_HEIGHT / 2);
	genOre(2,  Tile::goldOre->id, 8, 0, 32);
	genOre(8,  Tile::redStoneOre->id, 7, 0, 16);
	genOre(1,  Tile::emeraldOre->id, 7, 0, 32);
	genOre(1,  Tile::lapisOre->id, 6, 0, 32);

	if (biomeId == VanillaBiomeIds::MESA || biomeId == VanillaBiomeIds::MESA_PLATEAU || biomeId == VanillaBiomeIds::MESA_BRYCE) {
		genOre(20, Tile::goldOre->id, 8, 32, 80); // Extra gold in Mesa
	}

	// 3. Trees
	int treeCount = 0;
	if (biomeId == VanillaBiomeIds::FOREST || biomeId == VanillaBiomeIds::FOREST_HILLS) treeCount = 10;
	else if (biomeId == VanillaBiomeIds::BIRCH_FOREST || biomeId == VanillaBiomeIds::BIRCH_FOREST_MUTATED) treeCount = 10;
	else if (biomeId == VanillaBiomeIds::TAIGA || biomeId == VanillaBiomeIds::COLD_TAIGA) treeCount = 10;
	else if (biomeId == VanillaBiomeIds::ROOFED_FOREST) treeCount = 20;
	else if (biomeId == VanillaBiomeIds::MEGA_TAIGA) treeCount = 10;
	else if (biomeId == VanillaBiomeIds::JUNGLE || biomeId == VanillaBiomeIds::JUNGLE_MUTATED) treeCount = 50;
	else if (biomeId == VanillaBiomeIds::SAVANNA || biomeId == VanillaBiomeIds::SAVANNA_MUTATED) treeCount = 1;
	else if (biomeId == VanillaBiomeIds::SWAMPLAND) treeCount = 2;
	else if (biomeId == VanillaBiomeIds::PLAINS) {
		if (rand.nextInt(3) == 0) treeCount = 1;
	}

	for (int i = 0; i < treeCount; ++i) {
		int tx = bx + rand.nextInt(16) + 8;
		int tz = bz + rand.nextInt(16) + 8;
		int ty = level->getHeightmap(tx, tz);

		if (biomeId == VanillaBiomeIds::SAVANNA || biomeId == VanillaBiomeIds::SAVANNA_MUTATED) {
			AcaciaTreeFeature acacia;
			acacia.place(level, &rand, tx, ty, tz);
		} else if (biomeId == VanillaBiomeIds::ROOFED_FOREST) {
			if (rand.nextInt(3) == 0) {
				HugeMushroomFeature mushroom(rand.nextInt(2));
				mushroom.place(level, &rand, tx, ty, tz);
			} else {
				DarkOakTreeFeature darkOak;
				darkOak.place(level, &rand, tx, ty, tz);
			}
		} else if (biomeId == VanillaBiomeIds::MEGA_TAIGA) {
			if (rand.nextInt(3) == 0) {
				MegaPineTreeFeature megaPine(true);
				megaPine.place(level, &rand, tx, ty, tz);
			} else {
				SpruceFeature spruce(false);
				spruce.place(level, &rand, tx, ty, tz);
			}
		} else if (biomeId == VanillaBiomeIds::TAIGA || biomeId == VanillaBiomeIds::COLD_TAIGA) {
			if (rand.nextInt(3) == 0) {
				PineFeature pine(false);
				pine.place(level, &rand, tx, ty, tz);
			} else {
				SpruceFeature spruce(false);
				spruce.place(level, &rand, tx, ty, tz);
			}
		} else if (biomeId == VanillaBiomeIds::BIRCH_FOREST || biomeId == VanillaBiomeIds::BIRCH_FOREST_MUTATED) {
			BirchFeature birch(false);
			birch.place(level, &rand, tx, ty, tz);
		} else if (biomeId == VanillaBiomeIds::SWAMPLAND) {
			SwampTreeFeature swamp;
			swamp.place(level, &rand, tx, ty, tz);
		} else if (biomeId == VanillaBiomeIds::JUNGLE || biomeId == VanillaBiomeIds::JUNGLE_MUTATED) {
			if (rand.nextInt(3) == 0) {
				MegaJungleTreeFeature megaJungle;
				megaJungle.place(level, &rand, tx, ty, tz);
			} else {
				TreeFeature tree(false, TreeTile::JUNGLE_TRUNK);
				tree.place(level, &rand, tx, ty, tz);
			}
		} else {
			if (rand.nextInt(5) == 0) {
				BigOakTreeFeature bigOak;
				bigOak.place(level, &rand, tx, ty, tz);
			} else {
				TreeFeature tree(false);
				tree.place(level, &rand, tx, ty, tz);
			}
		}
	}

	// 4. Flowers and Grass
	int flowerCount = 2;
	if (biomeId == VanillaBiomeIds::FLOWER_FOREST || biomeId == VanillaBiomeIds::SUNFLOWER_PLAINS) flowerCount = 12;

	std::vector<Tile*> flowerPalette;
	if (Tile::flower_allium) flowerPalette.push_back(Tile::flower_allium);
	if (Tile::flower_blueOrchid) flowerPalette.push_back(Tile::flower_blueOrchid);
	if (Tile::flower_houstonia) flowerPalette.push_back(Tile::flower_houstonia);
	if (Tile::flower_tulipRed) flowerPalette.push_back(Tile::flower_tulipRed);
	if (Tile::flower_tulipOrange) flowerPalette.push_back(Tile::flower_tulipOrange);
	if (Tile::flower_tulipWhite) flowerPalette.push_back(Tile::flower_tulipWhite);
	if (Tile::flower_tulipPink) flowerPalette.push_back(Tile::flower_tulipPink);
	if (Tile::flower_oxeyeDaisy) flowerPalette.push_back(Tile::flower_oxeyeDaisy);
	if (Tile::flower_paeonia) flowerPalette.push_back(Tile::flower_paeonia);
	if (Tile::flower_roseBlue) flowerPalette.push_back(Tile::flower_roseBlue);
	if (Tile::flower_lilyOfTheValley) flowerPalette.push_back(Tile::flower_lilyOfTheValley);
	if (flowerPalette.empty()) {
		if (Tile::flower) flowerPalette.push_back(Tile::flower);
		if (Tile::rose) flowerPalette.push_back(Tile::rose);
	}

	for (int i = 0; i < flowerCount; ++i) {
		int fx = bx + rand.nextInt(16) + 8;
		int fz = bz + rand.nextInt(16) + 8;
		int fy = rand.nextInt(LEVEL_HEIGHT);
		Tile* picked = flowerPalette[rand.nextInt(flowerPalette.size())];
		FlowerFeature flower(picked ? picked->id : Tile::rose->id);
		flower.place(level, &rand, fx, fy, fz);
	}

	int grassCount = 1;
	if (biomeId == VanillaBiomeIds::PLAINS) grassCount = 10;
	else if (biomeId == VanillaBiomeIds::SAVANNA) grassCount = 12;
	for (int i = 0; i < grassCount; ++i) {
		int gx = bx + rand.nextInt(16) + 8;
		int gz = bz + rand.nextInt(16) + 8;
		int gy = rand.nextInt(LEVEL_HEIGHT);
		TallgrassFeature tallGrass(Tile::tallgrass->id, 1);
		tallGrass.place(level, &rand, gx, gy, gz);
	}

	// 5. Waterlilies in Swampland
	if (biomeId == VanillaBiomeIds::SWAMPLAND && Tile::waterlily) {
		for (int i = 0; i < 4; ++i) {
			int wx = bx + rand.nextInt(16) + 8;
			int wz = bz + rand.nextInt(16) + 8;
			int wy = rand.nextInt(LEVEL_HEIGHT);
			while (wy > 0 && level->getTile(wx, wy, wz) == 0) wy--;
			if (level->getTile(wx, wy, wz) == Tile::calmWater->id && level->getTile(wx, wy + 1, wz) == 0) {
				level->setTile(wx, wy + 1, wz, Tile::waterlily->id);
			}
		}
	}

	// 6. Cacti & Sugar cane in Desert / Beaches
	if (biomeId == VanillaBiomeIds::DESERT || biomeId == VanillaBiomeIds::DESERT_HILLS || biomeId == VanillaBiomeIds::MESA) {
		for (int i = 0; i < 5; ++i) {
			int cx = bx + rand.nextInt(16) + 8;
			int cz = bz + rand.nextInt(16) + 8;
			int cy = rand.nextInt(LEVEL_HEIGHT);
			CactusFeature cactus;
			cactus.place(level, &rand, cx, cy, cz);
		}
	}

	for (int i = 0; i < 5; ++i) {
		int rx = bx + rand.nextInt(16) + 8;
		int rz = bz + rand.nextInt(16) + 8;
		int ry = rand.nextInt(LEVEL_HEIGHT);
		ReedsFeature reeds;
		reeds.place(level, &rand, rx, ry, rz);
	}
}
