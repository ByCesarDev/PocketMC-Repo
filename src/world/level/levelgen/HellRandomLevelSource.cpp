#include "HellRandomLevelSource.h"
#include "../Level.h"
#include "../../../util/Mth.h"
#include "../ChunkPos.h"
#include "../biome/BiomeSource.h"
#include "../biome/Biome.h"
#include "../chunk/LevelChunk.h"
#include "../tile/Tile.h"
#include "../tile/HeavyTile.h"
#include "../../../util/Random.h"
#include "../material/Material.h"
#include "../../entity/MobCategory.h"
#include "../LevelConstants.h"

#include "feature/FeatureInclude.h"
#include "feature/HellFireFeature.h"
#include "feature/HellSpringFeature.h"
#include "feature/LightGemFeature.h"
#include "feature/OreFeature.h"
#include "feature/FlowerFeature.h"

const int HellRandomLevelSource::MAX_BUFFER_SIZE = 1024;

HellRandomLevelSource::HellRandomLevelSource(Level* level, int64_t seed)
    : random(seed),
      lperlinNoise1(&random, 16),
      lperlinNoise2(&random, 16),
      perlinNoise1(&random, 8),
      perlinNoise2(&random, 4),
      perlinNoise3(&random, 4),
      scaleNoise(&random, 10),
      depthNoise(&random, 16),
      level(level),
      caveFeature(true),
      buffer(nullptr),
      pnr(nullptr), ar(nullptr), br(nullptr), sr(nullptr), dr(nullptr)
{
    buffer = new float[MAX_BUFFER_SIZE];
}

HellRandomLevelSource::~HellRandomLevelSource()
{
    delete[] buffer;
    delete[] pnr;
    delete[] ar;
    delete[] br;
    delete[] sr;
    delete[] dr;
}

bool HellRandomLevelSource::hasChunk(int x, int y)
{
    return true;
}

LevelChunk* HellRandomLevelSource::create(int x, int z)
{
    return getChunk(x, z);
}

LevelChunk* HellRandomLevelSource::getChunk(int xOffs, int zOffs)
{
    int hashedPos = ChunkPos::hashCode(xOffs, zOffs);
    auto it = chunkMap.find(hashedPos);
    if (it != chunkMap.end())
        return it->second;

    random.setSeed(341873128712L * xOffs + 132897987541L * zOffs);

    uint16_t* blocks = new uint16_t[CHUNK_BLOCK_COUNT];
    memset(blocks, 0, CHUNK_BLOCK_COUNT * sizeof(uint16_t));

    prepareHeights(xOffs, zOffs, blocks);
    buildSurfaces(xOffs, zOffs, blocks);

    caveFeature.apply(this, level, xOffs, zOffs, blocks, CHUNK_BLOCK_COUNT);

    LevelChunk* levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
    chunkMap.insert(std::make_pair(hashedPos, levelChunk));
    levelChunk->recalcHeightmap();

    return levelChunk;
}

LevelChunk* HellRandomLevelSource::getChunkDontCreate(int x, int z)
{
    int hashedPos = ChunkPos::hashCode(x, z);
    auto it = chunkMap.find(hashedPos);
    if (it != chunkMap.end())
        return it->second;

    static uint16_t* emptyBlocks = nullptr;
    static LevelChunk* dummyChunk = nullptr;
    if (!dummyChunk) {
        emptyBlocks = new uint16_t[CHUNK_BLOCK_COUNT];
        memset(emptyBlocks, 0, CHUNK_BLOCK_COUNT * sizeof(uint16_t));
        dummyChunk = new LevelChunk(level, emptyBlocks, 0, 0);
    }
    return dummyChunk;
}

void HellRandomLevelSource::postProcess(ChunkSource* parent, int xt, int zt)
{
    level->isGeneratingTerrain = true;
    HeavyTile::instaFall = true;

    int xo = xt * 16;
    int zo = zt * 16;

    random.setSeed(level->getSeed());
    int xScale = random.nextInt() / 2 * 2 + 1;
    int zScale = random.nextInt() / 2 * 2 + 1;
    random.setSeed(((xt * xScale) + (zt * zScale)) ^ level->getSeed());

    // Lava springs
    if (Tile::lava) {
        for (int i = 0; i < 8; i++) {
            int x = xo + random.nextInt(16) + 8;
            int y = random.nextInt(120) + 4;
            int z = zo + random.nextInt(16) + 8;
            HellSpringFeature(Tile::lava->id).place(level, &random, x, y, z);
        }
    }

    // Fire patches
    if (Tile::fire) {
        int fireCount = random.nextInt(random.nextInt(10) + 1) + 1;
        for (int i = 0; i < fireCount; i++) {
            int x = xo + random.nextInt(16) + 8;
            int y = random.nextInt(120) + 4;
            int z = zo + random.nextInt(16) + 8;
            HellFireFeature().place(level, &random, x, y, z);
        }
    }

    // Glowstone clusters
    if (Tile::lightGem) {
        int glowCount = random.nextInt(random.nextInt(10) + 1);
        for (int i = 0; i < glowCount; i++) {
            int x = xo + random.nextInt(16) + 8;
            int y = random.nextInt(120) + 4;
            int z = zo + random.nextInt(16) + 8;
            LightGemFeature().place(level, &random, x, y, z);
        }
    }

    // Brown mushrooms
    if (Tile::mushroom1 && random.nextInt(1) == 0) {
        int x = xo + random.nextInt(16) + 8;
        int y = random.nextInt(128);
        int z = zo + random.nextInt(16) + 8;
        FlowerFeature(Tile::mushroom1->id).place(level, &random, x, y, z);
    }

    // Red mushrooms
    if (Tile::mushroom2 && random.nextInt(1) == 0) {
        int x = xo + random.nextInt(16) + 8;
        int y = random.nextInt(128);
        int z = zo + random.nextInt(16) + 8;
        FlowerFeature(Tile::mushroom2->id).place(level, &random, x, y, z);
    }

    // Nether Quartz Ore (PocketMC feature)
    if (Tile::netherQuartzOre) {
        for (int i = 0; i < 16; ++i) {
            int x = xo + random.nextInt(16);
            int y = random.nextInt(128);
            int z = zo + random.nextInt(16);
            OreFeature(Tile::netherQuartzOre->id, 14).place(level, &random, x, y, z);
        }
    }

    HeavyTile::instaFall = false;
    level->isGeneratingTerrain = false;
}

bool HellRandomLevelSource::tick()
{
    return false;
}

bool HellRandomLevelSource::shouldSave()
{
    return true;
}

std::string HellRandomLevelSource::gatherStats()
{
    return "HellRandomLevelSource";
}

Biome::MobList HellRandomLevelSource::getMobsAt(const MobCategory& mobCategory, int x, int y, int z)
{
    BiomeSource* biomeSource = level->getBiomeSource();
    if (biomeSource == NULL) {
        return Biome::MobList();
    }
    Biome* biome = biomeSource->getBiome(x, z);
    if (biome == NULL) {
        return Biome::MobList();
    }
    return biome->getMobs(mobCategory);
}

float* HellRandomLevelSource::getHeights(float* buffer, int x, int y, int z, int xSize, int ySize, int zSize)
{
    const int size = xSize * ySize * zSize;
    if (size > MAX_BUFFER_SIZE) {
        return buffer;
    }

    constexpr float C_MAGIC_1 = 684.412f;
    constexpr float C_MAGIC_2 = 2053.236f;

    sr = scaleNoise.getRegion(sr, x, z, xSize, zSize, 1.0f, 0.0f, 1.0f);
    dr = depthNoise.getRegion(dr, x, z, xSize, zSize, 100.0f, 0.0f, 100.0f);

    pnr = perlinNoise1.getRegion(pnr, (float)x, (float)y, (float)z, xSize, ySize, zSize,
        C_MAGIC_1 / 80.0f, C_MAGIC_2 / 60.0f, C_MAGIC_1 / 80.0f);
    ar = lperlinNoise1.getRegion(ar, (float)x, (float)y, (float)z, xSize, ySize, zSize,
        C_MAGIC_1, C_MAGIC_2, C_MAGIC_1);
    br = lperlinNoise2.getRegion(br, (float)x, (float)y, (float)z, xSize, ySize, zSize,
        C_MAGIC_1, C_MAGIC_2, C_MAGIC_1);

    float* yOffsFactors = new float[ySize];
    for (int i = 0; i < ySize; i++) {
        yOffsFactors[i] = Mth::cos(i * Mth::PI * 6.0f / ySize) * 2.0f;
        float fi = (float)i;
        if (i > ySize / 2) {
            fi = (float)(ySize - 1 - i);
        }
        if (fi < 4.0f) {
            fi = 4.0f - fi;
            yOffsFactors[i] -= fi * fi * fi * 10.0f;
        }
    }

    int p = 0;
    int pp = 0;

    for (int xx = 0; xx < xSize; xx++) {
        for (int zz = 0; zz < zSize; zz++) {
            float scale = (sr[pp] + 256.0f) / 512.0f;
            if (scale > 1.0f) scale = 1.0f;
            if (scale < 0.0f) scale = 0.0f;
            scale += 0.5f;

            float depth = dr[pp] / 8000.0f;
            if (depth < 0.0f) {
                depth = -depth * 0.3f;
            }
            depth = depth * 3.0f - 2.0f;
            if (depth < 0.0f) {
                depth /= 2.0f;
                if (depth < -1.0f) depth = -1.0f;
                depth /= 1.4f;
                depth /= 2.0f;
                scale = 0.0f;
            } else {
                if (depth > 1.0f) depth = 1.0f;
                depth /= 8.0f;
            }
            if (scale < 0.0f) scale = 0.0f;

            depth = depth * (float)ySize / 16.0f;

            pp++;

            for (int yy = 0; yy < ySize; yy++) {
                float val = 0.0f;
                float yOffs = yOffsFactors[yy];

                float bb = ar[p] / 512.0f;
                float cc = br[p] / 512.0f;
                float v = (pnr[p] / 10.0f + 1.0f) / 2.0f;

                if (v < 0.0f) val = bb;
                else if (v > 1.0f) val = cc;
                else val = bb + (cc - bb) * v;

                val -= yOffs;

                if (yy > ySize - 4) {
                    float slide = (float)(yy - (ySize - 4)) / 3.0f;
                    val = val * (1.0f - slide) + -10.0f * slide;
                }

                buffer[p] = val;
                p++;
            }
        }
    }

    delete[] yOffsFactors;
    return buffer;
}

void HellRandomLevelSource::prepareHeights(int xOffs, int zOffs, uint16_t* blocks)
{
    int xChunks = 16 / 4;
    int xSize = xChunks + 1;
    int ySize = 17;
    int zSize = xChunks + 1;

    buffer = getHeights(buffer, xOffs * xChunks, 0, zOffs * xChunks, xSize, ySize, zSize);

    for (int xc = 0; xc < xChunks; xc++) {
        for (int zc = 0; zc < xChunks; zc++) {
            for (int yc = 0; yc < 16; yc++) {
                float yStep = 1.0f / 8.0f;

                float s0 = buffer[((xc + 0) * zSize + (zc + 0)) * ySize + (yc + 0)];
                float s1 = buffer[((xc + 0) * zSize + (zc + 1)) * ySize + (yc + 0)];
                float s2 = buffer[((xc + 1) * zSize + (zc + 0)) * ySize + (yc + 0)];
                float s3 = buffer[((xc + 1) * zSize + (zc + 1)) * ySize + (yc + 0)];
                float s0a = (buffer[((xc + 0) * zSize + (zc + 0)) * ySize + (yc + 1)] - s0) * yStep;
                float s1a = (buffer[((xc + 0) * zSize + (zc + 1)) * ySize + (yc + 1)] - s1) * yStep;
                float s2a = (buffer[((xc + 1) * zSize + (zc + 0)) * ySize + (yc + 1)] - s2) * yStep;
                float s3a = (buffer[((xc + 1) * zSize + (zc + 1)) * ySize + (yc + 1)] - s3) * yStep;

                for (int y = 0; y < 8; y++) {
                    float xStep = 1.0f / 4.0f;
                    float _s0 = s0;
                    float _s1 = s1;
                    float _s0a = (s2 - s0) * xStep;
                    float _s1a = (s3 - s1) * xStep;

                    for (int x = 0; x < 4; x++) {
                        int offs = (x + xc * 4) << 12 | (0 + zc * 4) << 8 | (yc * 8 + y);
                        int step = 1 << 8;
                        float zStep = 1.0f / 4.0f;
                        float val = _s0;
                        float vala = (_s1 - _s0) * zStep;

                        for (int z = 0; z < 4; z++) {
                            int globalY = yc * 8 + y;
                            uint16_t tileId = 0;

                            if (globalY < 32 && Tile::calmLava) {
                                tileId = (uint16_t)Tile::calmLava->id;
                            }

                            if (val > 0.0f && Tile::netherrack) {
                                tileId = (uint16_t)Tile::netherrack->id;
                            }

                            blocks[offs] = tileId;
                            offs += step;
                            val += vala;
                        }
                        _s0 += _s0a;
                        _s1 += _s1a;
                    }
                    s0 += s0a;
                    s1 += s1a;
                    s2 += s2a;
                    s3 += s3a;
                }
            }
        }
    }
}

void HellRandomLevelSource::buildSurfaces(int xOffs, int zOffs, uint16_t* blocks)
{
    float s = 1.0f / 32.0f;
    perlinNoise2.getRegion(sandBuffer, (float)(xOffs * 16), (float)(zOffs * 16), 0, 16, 16, 1, s, s, 1);
    perlinNoise2.getRegion(gravelBuffer, (float)(xOffs * 16), 109.0134f, (float)(zOffs * 16), 16, 1, 16, s, 1, s);
    perlinNoise3.getRegion(depthBuffer, (float)(xOffs * 16), (float)(zOffs * 16), 0, 16, 16, 1, 1.0f / 16.0f, 1.0f / 16.0f, 1.0f / 16.0f);

    constexpr int byte0 = 64;
    int netherrackId = Tile::netherrack ? Tile::netherrack->id : 87;
    int soulSandId = Tile::soulSand ? Tile::soulSand->id : netherrackId;
    int gravelId = Tile::gravel ? Tile::gravel->id : 13;
    int bedrockId = Tile::unbreakable ? Tile::unbreakable->id : 7;
    int lavaId = Tile::calmLava ? Tile::calmLava->id : 11;

    for (int k = 0; k < 16; k++) {
        for (int l = 0; l < 16; l++) {
            bool hasSurfaceNoise = sandBuffer[k + l * 16] + random.nextDouble() * 0.2 > 0.0;
            bool hasGravelNoise = gravelBuffer[k + l * 16] + random.nextDouble() * 0.2 > 3.0;
            int i1 = (int)(depthBuffer[k + l * 16] / 3.0 + 3.0 + random.nextDouble() * 0.25);
            int j1 = -1;

            uint16_t topBlock = (uint16_t)netherrackId;
            uint16_t fillerBlock = (uint16_t)netherrackId;

            for (int k1 = 127; k1 >= 0; k1--) {
                int l1 = (k * 16 + l) * 128 + k1;

                if (k1 >= 127 - random.nextInt(5) || k1 <= random.nextInt(5)) {
                    blocks[l1] = (uint16_t)bedrockId;
                    continue;
                }

                uint16_t byte3 = blocks[l1];
                if (byte3 == 0) {
                    j1 = -1;
                    continue;
                }

                if (byte3 != (uint16_t)netherrackId)
                    continue;

                if (j1 == -1) {
                    if (i1 <= 0) {
                        topBlock = 0;
                        fillerBlock = (uint16_t)netherrackId;
                    }
                    else if (k1 >= byte0 - 4 && k1 <= byte0 + 1) {
                        topBlock = (uint16_t)netherrackId;
                        fillerBlock = (uint16_t)netherrackId;
                        if (hasGravelNoise) {
                            topBlock = (uint16_t)gravelId;
                            fillerBlock = (uint16_t)netherrackId;
                        }
                        if (hasSurfaceNoise) {
                            topBlock = (uint16_t)soulSandId;
                            fillerBlock = (uint16_t)soulSandId;
                        }
                    }

                    if (k1 < byte0 && topBlock == 0)
                        topBlock = (uint16_t)lavaId;

                    j1 = i1;

                    if (k1 >= byte0 - 1)
                        blocks[l1] = topBlock;
                    else
                        blocks[l1] = fillerBlock;

                    continue;
                }

                if (j1 <= 0)
                    continue;

                j1--;
                blocks[l1] = fillerBlock;
            }
        }
    }
}
