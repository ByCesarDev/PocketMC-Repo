#include "Dimension.h"
#include "NormalDayCycleDimension.h"
#include "NetherDimension.h"

//#include "../levelgen/SimpleLevelSource.h"
#include "../levelgen/RandomLevelSource.h"
#include "../levelgen/FlatLevelSource.h"
#include "../levelgen/vanilla/VanillaLevelSource.h"
#include "../storage/LevelData.h"
#include "../Level.h"
#include "../biome/BiomeSource.h"
#include "../chunk/ChunkSource.h"
#include "../tile/Tile.h"
#include "../../../util/Mth.h"


Dimension::Dimension()
:	foggy(false),
	ultraWarm(false),
	hasCeiling(false),
	biomeSource(NULL),
	id(0)
{
}

Dimension::~Dimension()
{
	delete biomeSource;
}

void Dimension::init( Level* level )
{
	this->level = level;
	init();
	updateLightRamp();
}

void Dimension::init()
{
	biomeSource = new BiomeSource(level);
}

/*virtual*/
bool Dimension::isValidSpawn(int x, int z) {
	int groundY = level->getTopTileY(x, z);
	if (groundY < 63 || groundY >= Level::DEPTH - 2) return false;

	int groundId = level->getTile(x, groundY, z);
	if (groundId <= 0 || groundId >= 256 || Tile::tiles[groundId] == NULL)
		return false;

	if (groundId == Tile::invisible_bedrock->id ||
		groundId == Tile::water->id || groundId == Tile::calmWater->id ||
		groundId == Tile::lava->id || groundId == Tile::calmLava->id)
		return false;

	Tile* ground = Tile::tiles[groundId];
	if (!ground->isSolidRender()) return false;

	if (!level->isEmptyTile(x, groundY + 1, z)) return false;
	if (!level->isEmptyTile(x, groundY + 2, z)) return false;

	return true;
}

float Dimension::getTimeOfDay(long time, float a) {
	return 1;
}

int Dimension::getMoonPhase(long time) {
	int phase = (int)(time / 24000L);
	phase = phase % 8;
	if (phase < 0) {
		phase = (phase - 1 | 0xfffffff8) + 1; // 0.7.6 original math or simpy phase += 8
	}
	return phase;
}

ChunkSource* Dimension::createRandomLevelSource() {
	if (level->getLevelData()) {
		if (level->getLevelData()->getWorldType() == WorldType::FLAT || level->getLevelData()->isFlat()) {
			return new FlatLevelSource(level, level->getSeed(), true);
		}
		if (level->getLevelData()->isExperimental() && level->getLevelData()->getWorldType() == WorldType::INFINITE_SIZE) {
			return new VanillaLevelSource(level, level->getSeed());
		}
	}
	return new RandomLevelSource(
		level,
		level->getSeed(),
		level->getLevelData()->getGeneratorVersion(),
		!level->isClientSide && level->getLevelData()->getSpawnMobs());
}


void Dimension::updateLightRamp()
{
	updateLightRamp(0.0f);
}

void Dimension::updateLightRamp(float gamma)
{
	for (int i = 0; i <= 15; i++) {
		float v = 1.0f - i * 0.0625f;
		float f = ((1.0f - v) / (v * 3.0f + 1.0f)) * 0.95f + 0.15f;
		brightnessRamp[i] = f * (1.0f - gamma) + (f * 3.0f + 1.0f) / 4.0f * gamma;
	}
}

float* Dimension::getSunriseColor( float td, float a )
{
	float span = 0.4f;
	float tt = Mth::cos(td * Mth::PI * 2) - 0.0f;
	float mid = -0.0f;
	if (tt >= mid - span && tt <= mid + span) {
		float aa = ((tt - mid) / span) * 0.5f + 0.5f;
		float mix = 1 - (((1 - Mth::sin(aa * Mth::PI))) * 0.99f);
		mix = mix * mix;
		sunriseCol[0] = (aa * 0.3f + 0.7f);
		sunriseCol[1] = (aa * aa * 0.7f + 0.2f);
		sunriseCol[2] = (aa * aa * 0.0f + 0.2f);
		sunriseCol[3] = mix;
		return sunriseCol;
	}
	return NULL;
}

Vec3 Dimension::getFogColor( float td, float a )
{
	float br = Mth::cos(td * Mth::PI * 2) * 2 + 0.5f;
	if (br < 0.0f) br = 0.0f;
	if (br > 1.0f) br = 1.0f;

	float r = ((fogColor >> 16) & 0xff) / 255.0f;
	float g = ((fogColor >> 8) & 0xff) / 255.0f;
	float b = ((fogColor) & 0xff) / 255.0f;
	r *= br * 0.94f + 0.06f;
	g *= br * 0.94f + 0.06f;
	b *= br * 0.91f + 0.09f;
	return Vec3(r, g, b);
	//return Vec3(0.752941f, 0.847059f, 1);
}

bool Dimension::mayRespawn()
{
	return true;
}

Dimension* Dimension::getNew( int id )
{
	if (id == NORMAL) return new Dimension();
	if (id == NORMAL_DAYCYCLE) return new NormalDayCycleDimension();
	if (id == -1) return new NetherDimension();
	return NULL;
}

//
// DimensionFactory
//
#include "../storage/LevelData.h"
Dimension* DimensionFactory::createDefaultDimension(LevelData* data )
{
	int dimensionId = data->getDimension();
	if (dimensionId == -1) {
		return Dimension::getNew(-1);
	}

	switch(data->getGameType()) {
	case GameType::Survival: dimensionId = Dimension::NORMAL_DAYCYCLE;
		break;
	case GameType::Creative:
		dimensionId = Dimension::NORMAL_DAYCYCLE;
		break;
	default:
		dimensionId = Dimension::NORMAL;
		break;
	}

	return Dimension::getNew(dimensionId);
}
