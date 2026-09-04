#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__HellRandomLevelSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__HellRandomLevelSource_H__

#include <unordered_map>
#include "../chunk/ChunkSource.h"
#include "synth/PerlinNoise.h"
#include "LargeCaveFeature.h"
#include "../../../util/Random.h"

class BiomeSource;
class Level;

class HellRandomLevelSource : public ChunkSource
{
public:
    HellRandomLevelSource(Level* level, int64_t seed);
    virtual ~HellRandomLevelSource();

    bool hasChunk(int x, int y) override;
    LevelChunk* create(int x, int z) override;
    LevelChunk* getChunk(int xOffs, int zOffs) override;
    LevelChunk* getChunkDontCreate(int x, int z);
    void postProcess(ChunkSource* parent, int xt, int zt) override;
    bool tick() override;
    bool shouldSave() override;
    std::string gatherStats() override;
    Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z) override;

    void prepareHeights(int xOffs, int zOffs, uint16_t* blocks);
    void buildSurfaces(int xOffs, int zOffs, uint16_t* blocks);

private:
    float* getHeights(float* buffer, int x, int y, int z, int xSize, int ySize, int zSize);

    static const int MAX_BUFFER_SIZE;

    std::unordered_map<int, LevelChunk*> chunkMap;
    Random random;
    PerlinNoise lperlinNoise1;
    PerlinNoise lperlinNoise2;
    PerlinNoise perlinNoise1;
    PerlinNoise perlinNoise2;
    PerlinNoise perlinNoise3;
    PerlinNoise scaleNoise;
    PerlinNoise depthNoise;
    Level* level;
    LargeCaveFeature caveFeature;

    float* buffer;
    float sandBuffer[16 * 16];
    float gravelBuffer[16 * 16];
    float depthBuffer[16 * 16];

    float* pnr;
    float* ar;
    float* br;
    float* sr;
    float* dr;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__HellRandomLevelSource_H__*/
