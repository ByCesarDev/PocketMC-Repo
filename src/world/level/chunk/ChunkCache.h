#ifndef NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__
#define NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__

//package net.minecraft.world.level.chunk;

#include <unordered_map>
#include <memory>
#include "ChunkSource.h"
#include "storage/ChunkStorage.h"
#include "EmptyLevelChunk.h"
#include "../Level.h"
#include "../LevelConstants.h"

class ChunkCache: public ChunkSource {
    static const int MAX_SAVES = 2;
    static const int MAX_LOADED_CHUNKS = 1024; // Maximum chunks to keep in memory
    
    // Hash function for chunk coordinates
    static uint64_t chunkHash(int x, int z) {
        return ((uint64_t)(uint32_t)x << 32) | (uint32_t)z;
    }
    
public:
    ChunkCache(Level* level_, ChunkStorage* storage_, ChunkSource* source_)
	:	xLast(-999999999),
		zLast(-999999999),
		last(NULL),
		level(level_),
		storage(storage_),
		source(source_)
	{
		isChunkCache = true;
		emptyChunk = new EmptyLevelChunk(level_, 0, 0);
    }

	~ChunkCache() {
		delete source;
		delete emptyChunk;

		for (auto& pair : chunkMap) {
			if (pair.second && pair.second.get() != emptyChunk) {
				pair.second->deleteBlockData();
			}
		}
		chunkMap.clear();
	}

    bool hasChunk(int x, int z) {
        if (x == xLast && z == zLast && last != NULL) {
            return true;
        }
        uint64_t hash = chunkHash(x, z);
        auto it = chunkMap.find(hash);
        return it != chunkMap.end() && (it->second.get() == emptyChunk || it->second->isAt(x, z));
    }

    LevelChunk* create(int x, int z) {
        return getChunk(x, z);
    }

    LevelChunk* getChunk(int x, int z) {
		if (x == xLast && z == zLast && last != NULL) {
            return last;
        }
        
        uint64_t hash = chunkHash(x, z);
        
        if (!hasChunk(x, z)) {
            LevelChunk* newChunk = load(x, z);
			bool updateLights = false;
            if (newChunk == NULL) {
                if (source == NULL) {
                    newChunk = emptyChunk;
                } else {
                    newChunk = source->getChunk(x, z);
                }
            } else {
				updateLights = true;
            }
            
            chunkMap[hash] = std::shared_ptr<LevelChunk>(newChunk);
            newChunk->lightLava();

			if (updateLights)
			{
				for (int cx = 0; cx < 16; cx++)
				{
					for (int cz = 0; cz < 16; cz++)
					{
						int height = level->getHeightmap(cx + x * 16, cz + z * 16);
						for (int cy = height; cy >= 0; cy--)
						{
							level->updateLight(LightLayer::Sky, cx + x * 16, cy, cz + z * 16, cx + x * 16, cy, cz + z * 16);
							level->updateLight(LightLayer::Block, cx + x * 16 - 1, cy, cz + z * 16 - 1, cx + x * 16 + 1, cy, cz + z * 16 + 1);
						}
					}
				}
			}

            if (newChunk != NULL) {
                newChunk->load();
            }

            if (!newChunk->terrainPopulated && hasChunk(x + 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x + 1, z)) postProcess(this, x, z);
            if (hasChunk(x - 1, z) && !getChunk(x - 1, z)->terrainPopulated && hasChunk(x - 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x - 1, z)) postProcess(this, x - 1, z);
            if (hasChunk(x, z - 1) && !getChunk(x, z - 1)->terrainPopulated && hasChunk(x + 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x + 1, z)) postProcess(this, x, z - 1);
            if (hasChunk(x - 1, z - 1) && !getChunk(x - 1, z - 1)->terrainPopulated && hasChunk(x - 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x - 1, z)) postProcess(this, x - 1, z - 1);
            
            // Unload distant chunks if cache is too large
            if (chunkMap.size() > MAX_LOADED_CHUNKS) {
                unloadDistantChunks();
            }
        }
        
        xLast = x;
        zLast = z;
        last = chunkMap[hash].get();

        return last;
    }

	Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z) {
		return source->getMobsAt(mobCategory, x, y, z);
	}

    void postProcess(ChunkSource* parent, int x, int z) {
        LevelChunk* chunk = getChunk(x, z);
        if (!chunk->terrainPopulated) {
            chunk->terrainPopulated = true;
            if (source != NULL) {
                source->postProcess(parent, x, z);
				chunk->clearUpdateMap();
            }
        }
    }

    bool tick() {
        if (storage != NULL) storage->tick();
        return source->tick();
    }

    bool shouldSave() {
        return true;
    }

    std::string gatherStats() {
        char buf[64];
        sprintf(buf, "ChunkCache: %zu loaded", chunkMap.size());
        return std::string(buf);
    }

	bool findSpawnPosition(int& spawnX, int& spawnZ) override {
		if (source != NULL) {
			return source->findSpawnPosition(spawnX, spawnZ);
		}
		return false;
	}
	
	void saveAll(bool onlyUnsaved) {
		if (storage != NULL) {
			std::vector<LevelChunk*> chunks;
			for (auto& pair : chunkMap) {
				LevelChunk* chunk = pair.second.get();
				if (chunk && chunk != emptyChunk && (!onlyUnsaved || chunk->shouldSave(false))) {
					chunks.push_back(chunk);
				}
			}
			storage->saveAll(level, chunks);
		}
	}
	
	void unloadDistantChunks() {
		// Get player positions (simplified - in real implementation, get all player positions)
		// For now, just unload oldest chunks if cache is full
		// This is a simple LRU-like eviction
		// TODO: Implement proper distance-based unloading based on player positions
	}
private:
    LevelChunk* load(int x, int z) {
        if (storage == NULL) return emptyChunk;
        LevelChunk* levelChunk = storage->load(level, x, z);
        if (levelChunk != NULL) {
            levelChunk->lastSaveTime = level->getTime();
        }
        return levelChunk;
    }

    void saveEntities(LevelChunk* levelChunk) {
        if (storage == NULL) return;
        storage->saveEntities(level, levelChunk);
    }

    void save(LevelChunk* levelChunk) {
        if (storage == NULL) return;
        levelChunk->lastSaveTime = level->getTime();
        storage->save(level, levelChunk);
    }

public:
	int xLast;
    int zLast;
private:
    LevelChunk* emptyChunk;
    ChunkSource* source;
    ChunkStorage* storage;
    std::unordered_map<uint64_t, std::shared_ptr<LevelChunk>> chunkMap;
    Level* level;

    LevelChunk* last;

};

#endif /*NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__*/
