#include "VanillaTrees.h"
#include "../../../../tile/Tile.h"
#include "../../../../tile/LeafTile.h"
#include <cmath>

BigOakTreeFeature::BigOakTreeFeature(bool doUpdate) : Feature(doUpdate) {}

bool BigOakTreeFeature::place(Level* level, Random* random, int x, int y, int z)
{
	int height = random->nextInt(6) + 8;
	if (y < 1 || y + height + 1 > 128) return false;

	int soil = level->getTile(x, y - 1, z);
	if (soil != Tile::grass->id && soil != Tile::dirt->id) return false;

	// Trunk
	for (int dy = 0; dy < height; ++dy) {
		level->setTileAndData(x, y + dy, z, Tile::treeTrunk->id, 0);
	}

	// Branches and leaves clusters
	for (int ly = y + height - 3; ly <= y + height + 1; ++ly) {
		int radius = (ly >= y + height) ? 1 : 2;
		for (int lx = x - radius; lx <= x + radius; ++lx) {
			for (int lz = z - radius; lz <= z + radius; ++lz) {
				if (std::abs(lx - x) == radius && std::abs(lz - z) == radius) {
					if (random->nextInt(2) == 0) continue;
				}
				if (level->getTile(lx, ly, lz) == 0) {
					level->setTileAndData(lx, ly, lz, Tile::leaves->id, 0);
				}
			}
		}
	}

	// 2 diagonal branches
	for (int b = 0; b < 2; ++b) {
		int bx = x + (random->nextInt(3) - 1);
		int bz = z + (random->nextInt(3) - 1);
		int by = y + height - random->nextInt(3) - 2;
		level->setTileAndData(bx, by, bz, Tile::treeTrunk->id, 0);
		for (int lx = bx - 1; lx <= bx + 1; ++lx) {
			for (int lz = bz - 1; lz <= bz + 1; ++lz) {
				if (level->getTile(lx, by + 1, lz) == 0) {
					level->setTileAndData(lx, by + 1, lz, Tile::leaves->id, 0);
				}
			}
		}
	}

	return true;
}

AcaciaTreeFeature::AcaciaTreeFeature(bool doUpdate) : Feature(doUpdate) {}

bool AcaciaTreeFeature::place(Level* level, Random* random, int x, int y, int z)
{
	int height = random->nextInt(3) + random->nextInt(3) + 5;
	if (y < 1 || y + height + 1 > 128) return false;

	int soil = level->getTile(x, y - 1, z);
	if (soil != Tile::grass->id && soil != Tile::dirt->id) return false;

	int dx = (random->nextInt(2) == 0) ? 1 : -1;
	int dz = (random->nextInt(2) == 0) ? 1 : -1;
	if (random->nextInt(2) == 0) dx = 0; else dz = 0;

	int twistHeight = height - 1 - random->nextInt(3);
	int cx = x, cz = z;
	int topY = y;

	for (int dy = 0; dy < height; ++dy) {
		if (dy >= twistHeight) {
			cx += dx;
			cz += dz;
		}
		topY = y + dy;
		level->setTileAndData(cx, topY, cz, Tile::treeTrunk->id, 0);
	}

	// Flat acacia canopy
	for (int lx = -3; lx <= 3; ++lx) {
		for (int lz = -3; lz <= 3; ++lz) {
			if (std::abs(lx) < 3 || std::abs(lz) < 3) {
				if (level->getTile(cx + lx, topY, cz + lz) == 0) {
					level->setTileAndData(cx + lx, topY, cz + lz, Tile::leaves->id, 0);
				}
			}
			if (std::abs(lx) <= 1 && std::abs(lz) <= 1) {
				if (level->getTile(cx + lx, topY + 1, cz + lz) == 0) {
					level->setTileAndData(cx + lx, topY + 1, cz + lz, Tile::leaves->id, 0);
				}
			}
		}
	}

	return true;
}

DarkOakTreeFeature::DarkOakTreeFeature(bool doUpdate) : Feature(doUpdate) {}

bool DarkOakTreeFeature::place(Level* level, Random* random, int x, int y, int z)
{
	int height = random->nextInt(3) + 6;
	if (y < 1 || y + height + 2 > 128) return false;

	for (int dx = 0; dx <= 1; ++dx) {
		for (int dz = 0; dz <= 1; ++dz) {
			int soil = level->getTile(x + dx, y - 1, z + dz);
			if (soil != Tile::grass->id && soil != Tile::dirt->id) return false;
		}
	}

	// 2x2 Trunk
	for (int dy = 0; dy < height; ++dy) {
		for (int dx = 0; dx <= 1; ++dx) {
			for (int dz = 0; dz <= 1; ++dz) {
				level->setTileAndData(x + dx, y + dy, z + dz, Tile::treeTrunk->id, 1);
			}
		}
	}

	// Dense canopy
	for (int dy = -2; dy <= 1; ++dy) {
		int radius = (dy >= 0) ? 2 : 3;
		for (int lx = -radius; lx <= radius + 1; ++lx) {
			for (int lz = -radius; lz <= radius + 1; ++lz) {
				if (level->getTile(x + lx, y + height + dy, z + lz) == 0) {
					level->setTileAndData(x + lx, y + height + dy, z + lz, Tile::leaves->id, 1);
				}
			}
		}
	}

	return true;
}

MegaPineTreeFeature::MegaPineTreeFeature(bool usePodzol, bool doUpdate)
:	Feature(doUpdate), usePodzol(usePodzol) {}

bool MegaPineTreeFeature::place(Level* level, Random* random, int x, int y, int z)
{
	int height = random->nextInt(13) + 18;
	if (y < 1 || y + height + 2 > 128) return false;

	// 2x2 Trunk
	for (int dy = 0; dy < height; ++dy) {
		for (int dx = 0; dx <= 1; ++dx) {
			for (int dz = 0; dz <= 1; ++dz) {
				level->setTileAndData(x + dx, y + dy, z + dz, Tile::treeTrunk->id, 1);
			}
		}
	}

	// Leaves cone
	int radius = 0;
	for (int dy = y + height; dy >= y + 6; --dy) {
		if (dy > y + height - 3) radius = 1;
		else if (radius < 4 && dy % 2 == 0) ++radius;

		for (int lx = -radius; lx <= radius + 1; ++lx) {
			for (int lz = -radius; lz <= radius + 1; ++lz) {
				if (std::abs(lx) == radius && std::abs(lz) == radius && radius > 1) continue;
				if (level->getTile(x + lx, dy, z + lz) == 0) {
					level->setTileAndData(x + lx, dy, z + lz, Tile::leaves->id, 1);
				}
			}
		}
	}

	return true;
}

MegaJungleTreeFeature::MegaJungleTreeFeature(bool doUpdate) : Feature(doUpdate) {}

bool MegaJungleTreeFeature::place(Level* level, Random* random, int x, int y, int z)
{
	int height = random->nextInt(15) + 15;
	if (y < 1 || y + height + 2 > 128) return false;

	// 2x2 Trunk
	for (int dy = 0; dy < height; ++dy) {
		for (int dx = 0; dx <= 1; ++dx) {
			for (int dz = 0; dz <= 1; ++dz) {
				level->setTileAndData(x + dx, y + dy, z + dz, Tile::treeTrunk->id, 3);
			}
		}
	}

	// Jungle canopy
	for (int dy = -2; dy <= 2; ++dy) {
		int radius = 3 - std::abs(dy);
		for (int lx = -radius; lx <= radius + 1; ++lx) {
			for (int lz = -radius; lz <= radius + 1; ++lz) {
				if (level->getTile(x + lx, y + height + dy, z + lz) == 0) {
					level->setTileAndData(x + lx, y + height + dy, z + lz, Tile::leaves->id, 3);
				}
			}
		}
	}

	return true;
}

SwampTreeFeature::SwampTreeFeature(bool doUpdate) : Feature(doUpdate) {}

bool SwampTreeFeature::place(Level* level, Random* random, int x, int y, int z)
{
	int height = random->nextInt(4) + 5;
	if (y < 1 || y + height + 1 > 128) return false;

	for (int dy = 0; dy < height; ++dy) {
		level->setTileAndData(x, y + dy, z, Tile::treeTrunk->id, 0);
	}

	for (int dy = y + height - 3; dy <= y + height; ++dy) {
		int radius = (dy == y + height) ? 1 : 3;
		for (int lx = x - radius; lx <= x + radius; ++lx) {
			for (int lz = z - radius; lz <= z + radius; ++lz) {
				if (level->getTile(lx, dy, lz) == 0) {
					level->setTileAndData(lx, dy, lz, Tile::leaves->id, 0);
				}
			}
		}
	}

	return true;
}

HugeMushroomFeature::HugeMushroomFeature(int type, bool doUpdate)
:	Feature(doUpdate), mushroomType(type) {}

bool HugeMushroomFeature::place(Level* level, Random* random, int x, int y, int z)
{
	int height = random->nextInt(3) + 4;
	if (y < 1 || y + height + 1 > 128) return false;

	int blockId = (mushroomType == 0) ? (Tile::mushroom1 ? Tile::mushroom1->id : Tile::wood->id)
	                                  : (Tile::mushroom2 ? Tile::mushroom2->id : Tile::wood->id);

	// Stem
	for (int dy = 0; dy < height; ++dy) {
		level->setTileAndData(x, y + dy, z, blockId, 10);
	}

	// Cap
	for (int lx = -2; lx <= 2; ++lx) {
		for (int lz = -2; lz <= 2; ++lz) {
			if (mushroomType == 0) { // Flat Brown Cap
				level->setTileAndData(x + lx, y + height, z + lz, blockId, 14);
			} else { // Rounded Red Cap
				if (std::abs(lx) == 2 && std::abs(lz) == 2) continue;
				level->setTileAndData(x + lx, y + height, z + lz, blockId, 14);
			}
		}
	}

	return true;
}
