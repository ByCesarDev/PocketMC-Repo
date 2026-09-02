#ifndef NET_MINECRAFT_WORLD_LEVEL_BIOME__FixedBiomeSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_BIOME__FixedBiomeSource_H__

#include "BiomeSource.h"
#include "Biome.h"

class FixedBiomeSource : public BiomeSource
{
public:
    FixedBiomeSource(float temperature, float downfall)
        : BiomeSource()
    {
        m_temperature = temperature;
        m_downfall = downfall;
        m_biome = Biome::hell;
        m_biomes = nullptr;
        m_biomesLen = 0;
    }

    FixedBiomeSource(Biome* biome, float temperature, float downfall)
        : BiomeSource()
    {
        m_temperature = temperature;
        m_downfall = downfall;
        m_biome = biome;
        m_biomes = nullptr;
        m_biomesLen = 0;
    }

    ~FixedBiomeSource() override
    {
        delete[] m_biomes;
    }

    Biome* getBiome(int x, int z) override
    {
        return m_biome;
    }

    Biome* getBiome(const ChunkPos& chunk) override
    {
        return m_biome;
    }

    float getTemperature(int x, int z)
    {
        return m_temperature;
    }

    float* getTemperatureBlock(int x, int z, int w, int h) override
    {
        if (lenTemperatures < w * h) {
            delete[] temperatures;
            temperatures = new float[w * h];
            lenTemperatures = w * h;
        }
        for (int i = 0; i < w * h; i++)
            temperatures[i] = m_temperature;
        return temperatures;
    }

    Biome** getBiomeBlock(int x, int z, int w, int h) override
    {
        if (m_biomesLen < w * h) {
            delete[] m_biomes;
            m_biomes = new Biome*[w * h];
            m_biomesLen = w * h;
        }
        for (int i = 0; i < w * h; i++)
            m_biomes[i] = m_biome;

        return m_biomes;
    }

private:
    float m_temperature;
    float m_downfall;
    Biome* m_biome;
    Biome** m_biomes;
    int m_biomesLen;
};

#endif

