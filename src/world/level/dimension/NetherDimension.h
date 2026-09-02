#ifndef NET_MINECRAFT_WORLD_LEVEL_DIMENSION__NetherDimension_H__
#define NET_MINECRAFT_WORLD_LEVEL_DIMENSION__NetherDimension_H__

#include "Dimension.h"
#include "../Level.h"
#include "../levelgen/HellRandomLevelSource.h"
#include "../biome/FixedBiomeSource.h"

class NetherDimension : public Dimension {
public:
	NetherDimension() {
		id = -1;
		foggy = true;
		ultraWarm = true;
		hasCeiling = true;
	}

	void init() override {
		biomeSource = new FixedBiomeSource(Biome::hell, 1.0f, 0.0f);
	}

	bool isNaturalDimension() override {
		return false;
	}

	bool mayRespawn() override {
		return false;
	}

	float getTimeOfDay(long time, float a) override {
		return 0.5f;
	}

	Vec3 getFogColor(float td, float a) override {
		return Vec3(0.5f, 0.15f, 0.1f);
	}

	void updateLightRamp(float gamma) override {
		for (int i = 0; i <= 15; i++) {
			float f = (float)i / 15.0f;
			brightnessRamp[i] = f * 0.65f + 0.35f;
		}
	}

	ChunkSource* createRandomLevelSource() override {
		return new HellRandomLevelSource(level, level->getSeed());
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_DIMENSION__NetherDimension_H__*/
