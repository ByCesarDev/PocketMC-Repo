#include "MesaGroundGenerator.h"
#include "../../../LevelConstants.h"
#include "../../../tile/Tile.h"
#include <cmath>

static const int CLAY_ORANGE = 1;
static const int CLAY_WHITE = 0;
static const int CLAY_YELLOW = 4;
static const int CLAY_BROWN = 12;
static const int CLAY_RED = 14;
static const int CLAY_LIGHT_GRAY = 8;

MesaGroundGenerator::MesaGroundGenerator(Type type)
:	GroundGenerator(Tile::sand->id, 1, Tile::sandStone->id, 0), type(type), seed(0), initialized(false)
{
	for (int i = 0; i < 64; ++i) colorLayer[i] = -1;
}

void MesaGroundGenerator::initialize(int64_t worldSeed)
{
	if (initialized && seed == worldSeed) return;
	seed = worldSeed;
	initialized = true;

	Random rand(worldSeed);
	colorNoise = VanillaGenerator::SimplexOctaveGenerator(rand, 1, 0, 0, 0);
	colorNoise.setScale(1.0 / 512.0);
	initializeColorLayers(rand);
}

void MesaGroundGenerator::initializeColorLayers(Random& rand)
{
	for (int i = 0; i < 64; ++i) colorLayer[i] = -1;

	int i = 0;
	while (i < 64) {
		i += rand.nextInt(5) + 1;
		if (i < 64) colorLayer[i++] = CLAY_ORANGE;
	}

	auto setBands = [&](int count, int color, int minThick, int maxThick) {
		for (int c = 0; c < count; ++c) {
			int thick = rand.nextInt(maxThick - minThick + 1) + minThick;
			int start = rand.nextInt(64 - thick);
			for (int j = 0; j < thick && start + j < 64; ++j) {
				colorLayer[start + j] = color;
			}
		}
	};

	setBands(rand.nextInt(3) + 3, CLAY_YELLOW, 1, 2);
	setBands(rand.nextInt(2) + 2, CLAY_BROWN, 1, 1);
	setBands(rand.nextInt(2) + 1, CLAY_RED, 1, 4);

	int whiteBands = rand.nextInt(3) + 3;
	int cur = 0;
	for (int w = 0; w < whiteBands; ++w) {
		cur += rand.nextInt(16) + 4;
		if (cur >= 64) break;
		if (cur - 1 < 64) colorLayer[cur - 1] = CLAY_WHITE;
		if (cur < 64) colorLayer[cur] = CLAY_LIGHT_GRAY;
		if (cur + 1 < 64) colorLayer[cur + 1] = CLAY_WHITE;
		cur += 2;
	}
}

static Tile* getStainedClayTile(int color) {
	switch (color) {
		case 0: return Tile::stainedClay_white ? Tile::stainedClay_white : Tile::hardenedClay;
		case 1: return Tile::stainedClay_orange ? Tile::stainedClay_orange : Tile::hardenedClay;
		case 2: return Tile::stainedClay_magenta ? Tile::stainedClay_magenta : Tile::hardenedClay;
		case 3: return Tile::stainedClay_lightBlue ? Tile::stainedClay_lightBlue : Tile::hardenedClay;
		case 4: return Tile::stainedClay_yellow ? Tile::stainedClay_yellow : Tile::hardenedClay;
		case 5: return Tile::stainedClay_lime ? Tile::stainedClay_lime : Tile::hardenedClay;
		case 6: return Tile::stainedClay_pink ? Tile::stainedClay_pink : Tile::hardenedClay;
		case 7: return Tile::stainedClay_gray ? Tile::stainedClay_gray : Tile::hardenedClay;
		case 8: return Tile::stainedClay_silver ? Tile::stainedClay_silver : Tile::hardenedClay;
		case 9: return Tile::stainedClay_cyan ? Tile::stainedClay_cyan : Tile::hardenedClay;
		case 10: return Tile::stainedClay_purple ? Tile::stainedClay_purple : Tile::hardenedClay;
		case 11: return Tile::stainedClay_blue ? Tile::stainedClay_blue : Tile::hardenedClay;
		case 12: return Tile::stainedClay_brown ? Tile::stainedClay_brown : Tile::hardenedClay;
		case 13: return Tile::stainedClay_green ? Tile::stainedClay_green : Tile::hardenedClay;
		case 14: return Tile::stainedClay_red ? Tile::stainedClay_red : Tile::hardenedClay;
		case 15: return Tile::stainedClay_black ? Tile::stainedClay_black : Tile::hardenedClay;
		default: return Tile::hardenedClay;
	}
}

void MesaGroundGenerator::generateTerrainColumn(LevelChunk* chunk, Random& rand, int x, int z, int biomeId, double surfaceNoise)
{
	initialize(rand.getSeed());
	int seaLevel = 64;

	int surfaceHeight = (int)(surfaceNoise / 3.0 + 3.0 + rand.nextFloat() * 0.25);
	if (surfaceHeight < 1) surfaceHeight = 1;

	bool colored = std::cos(surfaceNoise / 3.0 * 3.14159265358979323846) <= 0.0;
	int deep = -1;

	for (int y = LEVEL_HEIGHT - 1; y >= 0; --y) {
		if (y <= rand.nextInt(5)) {
			chunk->setTile(x, y, z, Tile::unbreakable->id);
		} else {
			int matId = chunk->getTile(x, y, z);
			if (matId == 0) {
				deep = -1;
			} else if (matId == Tile::rock->id) {
				if (deep == -1) {
					deep = surfaceHeight;
					if (y >= seaLevel - 1) {
						if (type == FOREST && y > 86) {
							chunk->setTile(x, y, z, Tile::grass->id);
						} else {
							chunk->setTileAndData(x, y, z, Tile::sand->id, 1); // Red sand
						}
					} else {
						chunk->setTile(x, y, z, Tile::hardenedClay ? Tile::hardenedClay->id : Tile::sandStone->id);
					}
				} else if (deep > 0) {
					--deep;
					if (colored) {
						int clayMeta = colorLayer[(y + (int)(surfaceNoise * 2.0)) & 63];
						if (clayMeta >= 0) {
							Tile* clayTile = getStainedClayTile(clayMeta);
							chunk->setTile(x, y, z, clayTile ? clayTile->id : Tile::rock->id);
						} else {
							chunk->setTile(x, y, z, Tile::hardenedClay ? Tile::hardenedClay->id : Tile::sandStone->id);
						}
					} else {
						chunk->setTile(x, y, z, Tile::hardenedClay ? Tile::hardenedClay->id : Tile::sandStone->id);
					}
				}
			}
		}
	}
}
