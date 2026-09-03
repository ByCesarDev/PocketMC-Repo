#include "WeatherRenderer.h"
#include "../Minecraft.h"
#include "Textures.h"
#include "Tesselator.h"
#include "../../world/level/Level.h"
#include "../../world/level/biome/BiomeSource.h"
#include "../../world/level/biome/Biome.h"
#include "../../world/entity/Entity.h"
#include "../particle/ParticleEngine.h"
#include "../particle/WaterDropParticle.h"
#include "../particle/SmokeParticle.h"
#include "../../world/level/tile/Tile.h"
#include "../../world/level/material/Material.h"
#include "../../world/level/tile/LiquidTile.h"

WeatherRenderer::WeatherRenderer()
:	mc(NULL),
	random(1),
	_tick(0),
	_rainSoundTime(0)
{
}

WeatherRenderer::~WeatherRenderer()
{
}

void WeatherRenderer::tick()
{
	_tick++;

	if (mc == NULL || mc->level == NULL || mc->cameraTargetPlayer == NULL)
		return;

	float rainLevel = mc->level->getRainLevel(1.0f);
	if (!mc->options.getBooleanValue(OPTIONS_FANCY_GRAPHICS))
		rainLevel /= 2.0f;
	if (rainLevel == 0.0f)
		return;

	if (mc->level->dimension && mc->level->dimension->hasCeiling)
		return;

	random.setSeed(_tick * 312987231L);
	Entity* camera = mc->cameraTargetPlayer;
	Level* level = mc->level;
	int cx = Mth::floor(camera->x);
	int cy = Mth::floor(camera->y);
	int cz = Mth::floor(camera->z);
	const int var7 = 10;
	Vec3 rainPos;
	int var14 = 0;

	for (int var15 = 0; var15 < (int)(100.0f * rainLevel * rainLevel); ++var15) {
		int bx = cx + random.nextInt(var7) - random.nextInt(var7);
		int bz = cz + random.nextInt(var7) - random.nextInt(var7);
		int by = level->getTopSolidBlock(bx, bz);
		int tileBelow = level->getTile(bx, by - 1, bz);
		Biome* biome = level->getBiome(bx, bz);
		if (by <= cy + var7 && by >= cy - var7 && biome && biome->canOnlyRain()) {
			float r1 = random.nextFloat();
			float r2 = random.nextFloat();
			if (tileBelow > 0) {
				Tile* tile = Tile::tiles[tileBelow];
				if (tile == NULL)
					continue;
				float aabbMinY = 0.0f;
				AABB* aabb = tile->getAABB(level, bx, by - 1, bz);
				if (aabb != NULL)
					aabbMinY = aabb->y0;
				if (tile->material == Material::lava) {
					mc->particleEngine->add(new SmokeParticle(level, bx + r1, by + 0.1f - aabbMinY, bz + r2, 0, 0, 0));
				} else {
					++var14;
					if (random.nextInt(var14) == 0) {
						rainPos = Vec3(bx + r1, (by + 0.1f) - aabbMinY, bz + r2);
					}
					mc->particleEngine->add(new WaterDropParticle(level, bx + r1, by + 0.1f - aabbMinY, bz + r2));
				}
			}
		}
	}

	if (var14 > 0 && ++_rainSoundTime >= 10) {
		_rainSoundTime = 0;

		int cameraRoof = level->getTopSolidBlock(cx, cz);
		bool underCover = cameraRoof > cy + 1;

		if (underCover)
			level->playSound(
				camera->x,
				camera->y,
				camera->z,
				"ambient.weather.rain",
				0.6f * rainLevel,
				0.5f
			);
		else if (rainPos.y > camera->y + 1.0f && cameraRoof > 0)
			level->playSound(
				rainPos.x,
				rainPos.y,
				rainPos.z,
				"ambient.weather.rain",
				0.8f * rainLevel,
				0.5f
			);
		else
			level->playSound(
				camera->x,
				camera->y,
				camera->z,
				"ambient.weather.rain",
				1.4f * rainLevel,
				1.0f
			);
	}
}

void WeatherRenderer::render(float a)
{
	if (mc == NULL || mc->level == NULL || mc->cameraTargetPlayer == NULL)
		return;

	Level* level = mc->level;
	float rainLevel = level->getRainLevel(a);
	if (rainLevel <= 0.0f)
		return;
	if (level->dimension && level->dimension->hasCeiling)
		return;

	Entity* player = mc->cameraTargetPlayer;
	int bPosX = Mth::floor(player->x);
	int bPosY = Mth::floor(player->y);
	int bPosZ = Mth::floor(player->z);

	Vec3 pos(
		player->xOld + (player->x - player->xOld) * a,
		player->yOld + (player->y - player->yOld) * a,
		player->zOld + (player->z - player->zOld) * a);
	pos.y -= player->heightOffset;
	Tesselator& t = Tesselator::instance;

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int range = mc->options.getBooleanValue(OPTIONS_FANCY_GRAPHICS) ? 10 : 5;

	Biome** biomes = level->getBiomeSource()->getBiomeBlock(bPosX - range, bPosZ - range, range * 2 + 1, range * 2 + 1);
	int i = 0;

	for (int bx = bPosX - range; bx <= bPosX + range; bx++) {
		for (int bz = bPosZ - range; bz <= bPosZ + range; bz++) {
			Biome* biome = biomes[i++];
			if (biome == NULL)
				continue;
			if (!biome->m_bHasSnow && !biome->m_bHasRain)
				continue;

			int tsb = level->getTopSolidBlock(bx, bz);
			if (tsb < 0)
				tsb = 0;

			int minY = bPosY - range;
			int maxY = bPosY + range;

			if (minY < tsb)
				minY = tsb;
			if (maxY < tsb)
				maxY = tsb;

			float offs = 1.0f;
			if (minY == maxY)
				continue;

			random.setSeed((int64_t)(bx * bx * 3121 + bx * 45238971 + bz * bz * 418711 + bz * 13761));

			float f1 = (float)(bx + 0.5f) - player->x;
			float f2 = (float)(bz + 0.5f) - player->z;
			float f3 = Mth::sqrt(f1 * f1 + f2 * f2) / (float)range;
			float f4 = level->getBrightness(bx, minY, bz);

			if (biome->m_bHasSnow) {
				float x1 = (float)_tick + a;
				float x2 = ((float)(_tick & 511) + a) / 512.0f;
				float x3 = random.nextFloat() + x1 * 0.01f * random.nextGaussian();
				float x4 = random.nextFloat() + x1 * 0.001f * random.nextGaussian();
				mc->textures->loadAndBindTexture("environment/snow.png");
				t.begin();
				t.color(f4, f4, f4, ((1.0f - f3 * f3) * 0.3f + 0.5f) * rainLevel);
				t.offset(-pos.x, -pos.y, -pos.z);
				t.vertexUV((float)(bx + 0), (float)minY, (float)(bz + 0.5f), 0.0f * offs + x3, (float)minY * offs / 4.0f + x2 * offs + x4);
				t.vertexUV((float)(bx + 1), (float)minY, (float)(bz + 0.5f), 1.0f * offs + x3, (float)minY * offs / 4.0f + x2 * offs + x4);
				t.vertexUV((float)(bx + 1), (float)maxY, (float)(bz + 0.5f), 1.0f * offs + x3, (float)maxY * offs / 4.0f + x2 * offs + x4);
				t.vertexUV((float)(bx + 0), (float)maxY, (float)(bz + 0.5f), 0.0f * offs + x3, (float)maxY * offs / 4.0f + x2 * offs + x4);
				t.vertexUV((float)(bx + 0.5f), (float)minY, (float)(bz + 0), 0.0f * offs + x3, (float)minY * offs / 4.0f + x2 * offs + x4);
				t.vertexUV((float)(bx + 0.5f), (float)minY, (float)(bz + 1), 1.0f * offs + x3, (float)minY * offs / 4.0f + x2 * offs + x4);
				t.vertexUV((float)(bx + 0.5f), (float)maxY, (float)(bz + 1), 1.0f * offs + x3, (float)maxY * offs / 4.0f + x2 * offs + x4);
				t.vertexUV((float)(bx + 0.5f), (float)maxY, (float)(bz + 0), 0.0f * offs + x3, (float)maxY * offs / 4.0f + x2 * offs + x4);
				t.offset(0.0f, 0.0f, 0.0f);
				t.draw();
			}
			if (biome->canOnlyRain()) {
				float x2 = ((float)(_tick + bx * bx * 3121 + bx * 45238971 + bz * bz * 418711 + bz * 13761 & 31) + a) / 32.0f * (3.0f + random.nextFloat());
				mc->textures->loadAndBindTexture("environment/rain.png");
				t.begin();
				f4 = f4 * 0.85f + 0.15f;
				t.color(f4, f4, f4, ((1.0f - f3 * f3) * 0.5f + 0.5f) * rainLevel);
				t.offset(-pos.x, -pos.y, -pos.z);
				t.vertexUV((float)(bx + 0), (float)minY, (float)(bz + 0.5f), 0.0f * offs, (float)minY * offs / 4.0f + x2 * offs);
				t.vertexUV((float)(bx + 1), (float)minY, (float)(bz + 0.5f), 1.0f * offs, (float)minY * offs / 4.0f + x2 * offs);
				t.vertexUV((float)(bx + 1), (float)maxY, (float)(bz + 0.5f), 1.0f * offs, (float)maxY * offs / 4.0f + x2 * offs);
				t.vertexUV((float)(bx + 0), (float)maxY, (float)(bz + 0.5f), 0.0f * offs, (float)maxY * offs / 4.0f + x2 * offs);
				t.vertexUV((float)(bx + 0.5f), (float)minY, (float)(bz + 0), 0.0f * offs, (float)minY * offs / 4.0f + x2 * offs);
				t.vertexUV((float)(bx + 0.5f), (float)minY, (float)(bz + 1), 1.0f * offs, (float)minY * offs / 4.0f + x2 * offs);
				t.vertexUV((float)(bx + 0.5f), (float)maxY, (float)(bz + 1), 1.0f * offs, (float)maxY * offs / 4.0f + x2 * offs);
				t.vertexUV((float)(bx + 0.5f), (float)maxY, (float)(bz + 0), 0.0f * offs, (float)maxY * offs / 4.0f + x2 * offs);
				t.offset(0.0f, 0.0f, 0.0f);
				t.draw();
			}
		}
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}
