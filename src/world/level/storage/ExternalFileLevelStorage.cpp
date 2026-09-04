#if !defined(DEMO_MODE) && !defined(APPLE_DEMO_PROMOTION)

#include "LevelData.h"
#include "RegionFile.h"
#include "ExternalFileLevelStorage.h"
#include "FolderMethods.h"
#include "../chunk/LevelChunk.h"
#include "../Level.h"
#include "../LevelConstants.h"
#include "platform/log.h"
#include "../tile/TreeTile.h"
#include "../../entity/EntityFactory.h"
#include "../../../nbt/NbtIo.h"
#include "../../../util/RakDataIO.h"
#include "../../../raknet/GetTime.h"
#include "../tile/entity/TileEntity.h"

static const int ChunkVersion_Light = 1;
static const int ChunkVersion_Entity = 2;

const char* const fnLevelDatOld = "level.dat_old";
const char* const fnLevelDatNew = "level.dat_new";
const char* const fnLevelDat    = "level.dat";
const char* const fnPlayerDat   = "player.dat";

//
// Helpers for converting old levels to newer
//
class LevelConverters
{
public:
	// Replacing old Cloth (id based) with new Cloth (data based)
	static bool v1_ClothIdToClothData(LevelChunk* c) {
		bool changed = false;
		uint16_t* blocks = c->getBlockData();
		uint16_t newTile = Tile::cloth->id;

		for (int i = 0; i < 16*16*128; ++i) {
			uint16_t oldTile = blocks[i];
			//Tile::cloth_00 to Tile::cloth_61
			if (oldTile >= 101 && oldTile <= 115) {
				int color = 0xf - (oldTile - 101);
				blocks[i] = newTile;
				c->data.set(i, color);
				changed = true;
			}
		}
		return changed;
	}

	// Replace unavailable blocks with air so placeholder "Update!" blocks never persist.
	static bool ReplaceUnavailableBlocks(LevelChunk* c) {
		//int st = getTimeMs();
		
		bool changed = false;
		uint16_t* blocks = c->getBlockData();

		for (int i = 0; i < 16*16*128; ++i) {
			uint16_t oldTile = blocks[i];
			uint16_t newTile = (uint16_t)Tile::transformToValidBlockId(oldTile);
			if (oldTile != newTile) {
				blocks[i] = newTile;
				changed = true;
			}
		}

		//int et = getTimeMs();
		//LOGI("time: %d\n", et - st);
		return changed;
	}

	//static bool ConvertPlayerDatToLevelDat() {
	//	return false;
	//}
};


ExternalFileLevelStorage::ExternalFileLevelStorage(const std::string& levelId, const std::string& fullPath)
:	levelId(levelId),
	levelPath(fullPath),
	loadedLevelData(NULL),
	currentDimensionId(0),
	netherRegionFile(NULL),
	entitiesFile(NULL),
	tickCount(0),
	lastSavedEntitiesTick(-999999),
	level(NULL),
	loadedStorageVersion(SharedConstants::StorageVersion)
{
	createFolderIfNotExists(levelPath.c_str());

	std::string datFileName   = levelPath + "/" + fnLevelDat;
	std::string levelFileName = levelPath + "/" + fnPlayerDat;
	loadedLevelData = new LevelData();
	if (readLevelData(levelPath, *loadedLevelData))
	{
		loadedStorageVersion = loadedLevelData->getStorageVersion();
		readPlayerData(levelFileName, *loadedLevelData);
	} else {
		delete loadedLevelData;
		loadedLevelData = NULL;
	}
}

ExternalFileLevelStorage::~ExternalFileLevelStorage()
{
	for (auto& pair : regionFiles) {
		delete pair.second;
	}
	regionFiles.clear();
	delete netherRegionFile;
	delete loadedLevelData;
}

void ExternalFileLevelStorage::saveLevelData(LevelData& levelData, std::vector<Player*>* players) {
	ExternalFileLevelStorage::saveLevelData(levelPath, levelData, players);
}

void ExternalFileLevelStorage::saveLevelData( const std::string& levelPath, LevelData& levelData, std::vector<Player*>* players )
{
	std::string directory = levelPath + "/";
    std::string tmpFile = directory + fnLevelDatNew;
    std::string datFile = directory + fnLevelDat;
    std::string oldFile = directory + fnLevelDatOld;

	levelData.setStorageVersion(SharedConstants::StorageVersion);
	if (!writeLevelData(tmpFile, levelData, players))
        return;

    // Remove old backup
    remove(oldFile.c_str());

    // If it exists, move the previous save to backup (and possibly delete it)
    if (exists(datFile.c_str())) {
        if (rename(datFile.c_str(), oldFile.c_str())) {
            LOGE("Error@saveLevelData: Couldn't move savefile to level.dat_old\n");
            return;
        }
        remove(datFile.c_str());
    }
    // Move the new save to level.dat
    if (rename(tmpFile.c_str(), datFile.c_str())) {
        LOGE("Error@saveLevelData: Couldn't move new file to level.dat\n");
        return;
    }

    // Remove the temporary save, if the rename didn't do it
    remove(tmpFile.c_str());
}

LevelData* ExternalFileLevelStorage::prepareLevel(Level* _level)
{
	level = _level;
	return loadedLevelData;
}

ChunkStorage* ExternalFileLevelStorage::createChunkStorage(Dimension* dimension) {
	if (dimension) {
		currentDimensionId = dimension->id;
	}
	clearRegionCache();
	return this;
}

void ExternalFileLevelStorage::clearRegionCache() {
	for (auto& pair : regionFiles) {
		delete pair.second;
	}
	regionFiles.clear();
}

bool ExternalFileLevelStorage::readLevelData(const std::string& directory, LevelData& levelData)
{
    // Try to load level.dat
    std::string datFilename = directory + "/" + fnLevelDat;
	FILE* file = fopen(datFilename.c_str(), "rb");

    // If that fails, try to load level.dat_old
    if (!file) {
        datFilename = directory + "/" + fnLevelDatOld;
        file = fopen(datFilename.c_str(), "rb");
    }

    if (!file)
        return false;

	int version = 0;
	int size = 0;
	unsigned char* data = NULL;

	do {
		if (fread(&version, sizeof(version), 1, file) != 1)
		{
			break;
		}
		if (fread(&size, sizeof(size), 1, file) != 1)
		{
			break;
		}

		int left = getRemainingFileSize(file);
		if (size > left || size <= 0)
			break;

		data = new unsigned char[size];
		if (fread(data, 1, size, file) != size)
		{
			break;
		}

		if (version == 1) {
			RakNet::BitStream bitStream(data, size, false);
			levelData.v1_read(bitStream, version);
		} else if (version >= 2) {
			//LOGI("---> Trying to load level with version %d\n", version);
			RakNet::BitStream bitStream(data, size, false);
			RakDataInput stream(bitStream);
			//LOGI("dat: %s\n", datFileName.c_str());
			CompoundTag* tag = NbtIo::read(&stream);
			if (tag) {
				levelData.getTagData(tag);
				tag->deleteChildren();
				delete tag;
			}
			//LOGI("<--- Finished reading level tag: %p\n", tag);
		}
	} while (false);

	fclose(file);
	delete [] data;

    return true;
}

bool ExternalFileLevelStorage::writeLevelData(const std::string& datFileName, LevelData& levelData, const std::vector<Player*>* players)
{
	// Write level info
	FILE* file = fopen(datFileName.c_str(), "wb");
	if (!file)
        return false;

    //if (levelData.getStorageVersion() == 1) {
	RakNet::BitStream data;
	if (levelData.getStorageVersion() == 1)
		levelData.v1_write(data);
	else {
		RakDataOutput buf(data);
		//LOGI("---> Trying to write level with version %d\n", version);
		CompoundTag* tag = NULL;
		if (players && !players->empty())
			tag = levelData.createTag(*players);
		else
			tag = levelData.createTag();

		NbtIo::write(tag, &buf);
		tag->deleteChildren();
		delete tag;
		//LOGI("<--- Finished writing level data. Size: %d\n", fdout.bytesWritten);
	}

	int version = levelData.getStorageVersion(); // 1
	fwrite(&version, sizeof(version), 1, file);
	int size = data.GetNumberOfBytesUsed();
	fwrite(&size, sizeof(size), 1, file);

	fwrite(data.GetData(), 1, size, file);
	fclose(file);

	return true;
}

bool ExternalFileLevelStorage::readPlayerData(const std::string& filename, LevelData& dest)
{
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp)
		return false;

	do {
		int version;
		if (fread(&version, 4, 1, fp) != 1)
			break;

		int size;
		if (fread(&size, 4, 1, fp) != 1)
			break;

		if (version == 1) {
			if (fread(&dest.playerData, 1, sizeof(dest.playerData), fp) != size)
				break;

			// Fix coordinates to new world limits
			Vec3& pos = dest.playerData.pos;
			if (pos.x < (WORLD_MIN_X + 0.5f)) pos.x = WORLD_MIN_X + 0.5f;
			if (pos.z < (WORLD_MIN_Z + 0.5f)) pos.z = WORLD_MIN_Z + 0.5f;
			if (pos.x > (WORLD_MAX_X + 0.5f)) pos.x = WORLD_MAX_X + 0.5f;
			if (pos.z > (WORLD_MAX_Z + 0.5f)) pos.z = WORLD_MAX_Z + 0.5f;
			if (pos.y < 0) pos.y = 64;

			dest.playerDataVersion = version;
		}
	} while (false);

	fclose(fp);
	return true;
}

void ExternalFileLevelStorage::tick()
{
	tickCount++;
	if ((tickCount % 1000) == 0 && level) {
		// Note: With infinite world, we can't iterate over all chunks
		// Instead, chunks are saved when they're unloaded or periodically
		// This is a simplified version - full implementation would track dirty chunks
		savePendingUnsavedChunks(2);
	}
	if (tickCount - lastSavedEntitiesTick > (60 * SharedConstants::TicksPerSecond)) {
		saveEntities(level, NULL);
	}
}

RegionFile* ExternalFileLevelStorage::getRegionFile(int chunkX, int chunkZ)
{
	// Calculate region coordinates
	int regionX = chunkX >> 5; // equivalent to floor(chunkX / 32)
	int regionZ = chunkZ >> 5;
	
	RegionKey key = {regionX, regionZ};
	auto it = regionFiles.find(key);
	
	if (it != regionFiles.end()) {
		return it->second;
	}
	
	// Create new region file with dimension-aware path
	std::string path;
	if (currentDimensionId == -1) {
		path = levelPath + "/DIM-1/region";
	} else {
		path = levelPath + "/region";
	}
	createFolderIfNotExists(path.c_str());
	
	RegionFile* newRegion = new RegionFile(path, regionX, regionZ);
	if (!newRegion->open()) {
		delete newRegion;
		return NULL;
	}
	
	regionFiles[key] = newRegion;
	return newRegion;
}

void ExternalFileLevelStorage::save(Level* level, LevelChunk* levelChunk)
{
	RegionFile* rFile = getRegionFile(levelChunk->x, levelChunk->z);
	if (!rFile) {
		return;
	}

	// Write chunk (16-bit block buffer: CHUNK_BLOCK_COUNT * sizeof(uint16_t))
	RakNet::BitStream chunkData;
	chunkData.Write((const char*)levelChunk->getBlockData(), CHUNK_BLOCK_COUNT * sizeof(uint16_t));
	chunkData.Write((const char*)levelChunk->data.data, CHUNK_BLOCK_COUNT / 2);

	chunkData.Write((const char*)levelChunk->skyLight.data, CHUNK_BLOCK_COUNT / 2);
	chunkData.Write((const char*)levelChunk->blockLight.data, CHUNK_BLOCK_COUNT / 2);

	chunkData.Write((const char*)levelChunk->updateMap, sizeof(levelChunk->updateMap));

	rFile->writeChunk(levelChunk->x, levelChunk->z, chunkData);

	// Write entities

	//LOGI("Saved chunk (%d, %d)\n", levelChunk->x, levelChunk->z);

}

LevelChunk* ExternalFileLevelStorage::load(Level* level, int x, int z)
{
	RegionFile* rFile = getRegionFile(x, z);
	if (!rFile) {
		return NULL;
	}

	RakNet::BitStream* chunkData = NULL;
	if (!rFile->readChunk(x, z, &chunkData))
	{
		//LOGI("Failed to read data for %d, %d\n", x, z);
		return NULL;
	}

	chunkData->ResetReadPointer();

	uint16_t* blockIds = new uint16_t[CHUNK_BLOCK_COUNT];
	int totalBytes = chunkData->GetNumberOfBytesUsed();
	bool sanitizedBlockIdsOnRead = false;

	// If total chunk size >= 200KB, it's modern 16-bit block format; otherwise legacy 8-bit format
	if (totalBytes >= (int)(CHUNK_BLOCK_COUNT * sizeof(uint16_t))) {
		chunkData->Read((char*)blockIds, CHUNK_BLOCK_COUNT * sizeof(uint16_t));
		for (int i = 0; i < CHUNK_BLOCK_COUNT; ++i) {
			uint16_t oldTile = blockIds[i];
			uint16_t newTile = (uint16_t)Tile::transformToValidBlockId(oldTile);
			if (oldTile != newTile) {
				blockIds[i] = newTile;
				sanitizedBlockIdsOnRead = true;
			}
		}
	} else {
		unsigned char* blockIds8 = new unsigned char[CHUNK_BLOCK_COUNT];
		chunkData->Read((char*)blockIds8, CHUNK_BLOCK_COUNT);
		for (int i = 0; i < CHUNK_BLOCK_COUNT; ++i) {
			uint16_t oldTile = blockIds8[i];
			uint16_t newTile = (uint16_t)Tile::transformToValidBlockId(oldTile);
			blockIds[i] = newTile;
			if (oldTile != newTile) {
				sanitizedBlockIdsOnRead = true;
			}
		}
		delete [] blockIds8;
	}

	LevelChunk* levelChunk = new LevelChunk(level, blockIds, x, z);
	chunkData->Read((char*)levelChunk->data.data, CHUNK_BLOCK_COUNT / 2);
	if (loadedStorageVersion >= ChunkVersion_Light) {
		chunkData->Read((char*)levelChunk->skyLight.data, CHUNK_BLOCK_COUNT / 2);
		chunkData->Read((char*)levelChunk->blockLight.data, CHUNK_BLOCK_COUNT / 2);
	}
	chunkData->Read((char*)levelChunk->updateMap, sizeof(levelChunk->updateMap));
	// This will be difficult to maintain.. Storage version could be per chunk
	// too (but probably better to just read all -> write all, so that all
	// chunks got same version anyway)
	//if (loadedStorageVersion >= ChunkVersion_Entity) {
	//	int dictSize;
	//	chunkData->Read(dictSize);

	//	RakDataInput dis(*chunkData);
	//	Tag* tmp = Tag::readNamedTag(&dis);
	//	if (tmp && tmp->getId() == Tag::TAG_Compound) {
	//		CompoundTag* tag = (CompoundTag*) tmp;

	//		delete tmp;
	//	}
	//}

	delete [] chunkData->GetData();
	delete chunkData;

	//bool dbg = (x == 7 && z == 9);

	//int t = 0;
	//for (int i = 0; i < CHUNK_COLUMNS; ++i) {
	//	char bits = levelChunk->updateMap[i];
	//	t += (bits != 0);
	//	int xx = x * 16 + i%16;
	//	int zz = z * 16 + i/16;
	//	if (dbg && xx == 125 && zz == 152) {
	//		LOGI("xz: %d, %d: %d\n", xx, zz, bits);
	//		for (int j = 0; j < 8; ++j) {
	//			if (bits & (1 << j)) {
	//				LOGI("%d - %d\n", j << 4, ((j+1) << 4) - 1);
	//			}
	//		}
	//	}
	//}

	//
	// Convert LevelChunks here if necessary
	//
	//LOGI("level version: %d: upd: %d - (%d, %d)\n", loadedStorageVersion, t, x, z);
	bool changed = sanitizedBlockIdsOnRead;

	// Loaded level has old Cloth types (one Tile* per color)
	if (loadedStorageVersion == 1)
		changed |= LevelConverters::v1_ClothIdToClothData(levelChunk);

	// Loaded level is newer than our level - replace all unavailable block types
	//if (loadedStorageVersion > SharedConstants::StorageVersion)
		changed |= LevelConverters::ReplaceUnavailableBlocks(levelChunk);

	levelChunk->recalcHeightmap();
	levelChunk->unsaved = changed;
	levelChunk->terrainPopulated = true;
	levelChunk->createdFromSave = true;

	return levelChunk;
}

void ExternalFileLevelStorage::saveEntities( Level* level, LevelChunk* levelChunk )
{
	lastSavedEntitiesTick = tickCount;
	int count = 0;
	float st = getTimeS();

	// Version 1: Save ALL Entities for all chunks in one structure
	EntityList& entities = level->entities;

	ListTag* entityTags = new ListTag();
	for (unsigned int i = 0; i < entities.size(); ++i) {
		Entity* e = entities[i];

		CompoundTag* tag = new CompoundTag();
		if (e->save(tag)) {
			count++;
			entityTags->add(tag);
		} else
			delete tag;
	}

	// Version 1: Save ALL TileEntities for all chunks in one structure
	TileEntityList& tileEntities = level->tileEntities;
	//TileEntityList keep, dontKeep;
	//partitionTileEntities

	ListTag* tileEntityTags = new ListTag();
	for (unsigned int i = 0; i < tileEntities.size(); ++i) {
		TileEntity* e = tileEntities[i];
		if (!e->shouldSave()) continue;

		CompoundTag* tag = new CompoundTag();
		if (e->save(tag)) {
			count++;
			tileEntityTags->add(tag);
		} else
			delete tag;
	}

	CompoundTag base;
	base.put("Entities", entityTags);
	base.put("TileEntities", tileEntityTags);

	RakNet::BitStream stream;
	RakDataOutput dos(stream);
	NbtIo::write(&base, &dos);
	int numBytes = stream.GetNumberOfBytesUsed();

	bool isNether = (level && level->dimension && level->dimension->id == -1);
	std::string entitiesPath = levelPath;
	if (isNether) {
		entitiesPath += "/DIM-1";
		createFolderIfNotExists(entitiesPath.c_str());
	}
	FILE* fp = fopen((entitiesPath + "/entities.dat").c_str(), "wb");
	if (fp) {
		int version = 1;
		fwrite("ENT\0", 1, 4, fp);
		fwrite(&version, sizeof(int), 1, fp);
		fwrite(&numBytes, sizeof(int), 1, fp);
		fwrite(stream.GetData(), 1, numBytes, fp);
		fclose(fp);
	}

	base.deleteChildren();
}

void ExternalFileLevelStorage::loadEntities(Level* level, LevelChunk* chunk) {
	lastSavedEntitiesTick = tickCount;
	bool isNether = (level && level->dimension && level->dimension->id == -1);
	std::string entitiesPath = levelPath;
	if (isNether) {
		entitiesPath += "/DIM-1";
	}
	FILE* fp = fopen((entitiesPath + "/entities.dat").c_str(), "rb");
	if (fp) {
		char header[5];
		int version, numBytes;
		fread(header, 1, 4, fp);
		fread(&version, sizeof(int), 1, fp);
		fread(&numBytes, sizeof(int), 1, fp);

		int left = getRemainingFileSize(fp);
		if (numBytes <= left && numBytes > 0) {
			unsigned char* buf = new unsigned char[numBytes];

			fread(buf, 1, numBytes, fp);

			RakNet::BitStream stream(buf, numBytes, false);
			RakDataInput dis(stream);

			CompoundTag* tag = NbtIo::read(&dis);
			//
			// Read Entity:es
			//
			if (tag->contains("Entities", Tag::TAG_List)) {
				ListTag* entityTags = tag->getList("Entities");
				for (int i = 0; i < entityTags->size(); ++i) {
					Tag* _et = entityTags->get(i);
					if (!_et || _et->getId() != Tag::TAG_Compound) {
						LOGE("Entity tag is either NULL or not a compoundTag: %p : %d!\n", _et, _et?_et->getId() : -1);
						continue;
					}
					CompoundTag* et = (CompoundTag*)_et;
					if (Entity* e = EntityFactory::loadEntity(et, level)) {
						level->addEntity(e);
					}
				}
			}
			//
			// Read TileEntity:s
			//
			if (tag->contains("TileEntities", Tag::TAG_List)) {
				ListTag* tileEntityTags = tag->getList("TileEntities");
				for (int i = 0; i < tileEntityTags->size(); ++i) {
					Tag* _et = tileEntityTags->get(i);
					if (!_et || _et->getId() != Tag::TAG_Compound) {
						LOGE("TileEntity tag is either NULL or not a compoundTag: %p : %d!\n", _et, _et?_et->getId() : -1);
						continue;
					}
					CompoundTag* et = (CompoundTag*)_et;
					if (TileEntity* e = TileEntity::loadStatic(et)) {
						
						LevelChunk* chunk = level->getChunkAt(e->x, e->z);
						if (chunk && !chunk->hasTileEntityAt(e)) {
							chunk->addTileEntity(e);
						} else {
							if (!chunk)
								LOGE("Couldn't find chunk at %d, %d to add %d\n", e->x, e->z, e->type);
							else
								LOGE("Already have TileEntity at %d, %d to add %d\n", e->x, e->z, e->type);
							delete e;
						}
					}
				}
			}

			tag->deleteChildren();
			delete tag;

			delete[] buf;
		}

		//fread(stream.GetData(), 1, numBytes, fp);
		fclose(fp);
	}
}

void ExternalFileLevelStorage::saveGame(Level* level) {
	saveEntities(level, NULL);
}

int ExternalFileLevelStorage::savePendingUnsavedChunks( int maxCount ) {
    if (maxCount < 0)
        maxCount = unsavedChunkList.size();

    int count = 0;
    while (++count <= maxCount && !unsavedChunkList.empty()) {

        UnsavedChunkList::iterator it = unsavedChunkList.begin();
        UnsavedChunkList::iterator remove = unsavedChunkList.begin();
        UnsavedLevelChunk* oldest = &(*it);

        for ( ; it != unsavedChunkList.end(); ++it) {
            if ((*it).addedToList < oldest->addedToList) {
                oldest = &(*it);
                remove = it;
            }
        }
        LevelChunk* chunk = oldest->chunk;
        unsavedChunkList.erase(remove);

        save(level, chunk);
    }
    return count;
}

void ExternalFileLevelStorage::saveAll( Level* level, std::vector<LevelChunk*>& levelChunks ) {
    ChunkStorage::saveAll(level, levelChunks);
	savePendingUnsavedChunks(-1);
}

#endif /*DEMO_MODE*/
