#include "NetherReactor.h"
#include "../../entity/player/Player.h"
#include "../Level.h"
#include "entity/NetherReactorTileEntity.h"
#include "NetherReactorPattern.h"
#include "../LevelConstants.h"
#include "../chunk/LevelChunk.h"

static bool existsReactorOrPortalElsewhere(Level* level, int x, int y, int z) {
	int centerCx = x >> 4;
	int centerCz = z >> 4;
	int r = 8;
	for (int cx = centerCx - r; cx <= centerCx + r; cx++) {
		for (int cz = centerCz - r; cz <= centerCz + r; cz++) {
			LevelChunk* chunk = level->getChunk(cx, cz);
			if (!chunk) continue;
			unsigned char* blocks = chunk->getBlockData();
			if (!blocks) continue;
			for (int lx = 0; lx < 16; lx++) {
				for (int lz = 0; lz < 16; lz++) {
					for (int ly = 10; ly < 120; ly++) {
						int absX = chunk->xt + lx;
						int absZ = chunk->zt + lz;
						if (absX == x && ly == y && absZ == z) continue;
						
						int tile = blocks[lx << 12 | lz << 8 | ly];
						if (tile == Tile::netherPortal->id || tile == Tile::netherReactor->id) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

NetherReactor::NetherReactor( int id, int tex, const Material* material ) : super(id, tex, material) { }

bool NetherReactor::use( Level* level, int x, int y, int z, Player* player ) {
	int currentPhase = level->getData(x, y, z);
	ItemInstance* carried = player->getCarriedItem();
	bool holdingGoldBlock = (carried && carried->id == Tile::goldBlock->id);

	if (currentPhase == 2) {
		if (holdingGoldBlock) {
			if (level->dimension && level->dimension->id == -1) {
				player->displayClientMessage("Cannot reactivate the reactor in the nether.");
				return false;
			}
			// Consume Gold Block
			carried->count--;
			if (carried->count <= 0) {
				player->inventory->clearSlot(player->inventory->selected);
			}
			player->displayClientMessage("The reactor has been reactivated!");
			NetherReactorTileEntity* reactor = static_cast<NetherReactorTileEntity*>(level->getTileEntity(x, y, z));
			if (reactor != NULL) {
				reactor->resetForReactivation();
				reactor->lightItUp(x, y, z);
			}
			return true;
		} else {
			player->displayClientMessage("Hold a gold block to reactivate the reactor.");
			return false;
		}
	}

	// Level, X, Z
	NetherReactorPattern pattern;
	for(int checkLevel = 0; checkLevel <= 2; ++checkLevel) {
		for(int checkX = -1; checkX <= 1; ++checkX) {
			for(int checkZ = -1; checkZ <= 1; ++checkZ) {
				if(level->getTile(x + checkX, y + checkLevel  - 1, z + checkZ) != pattern.getTileAt(checkLevel, checkX + 1, checkZ + 1)) {
					player->displayClientMessage("Not the correct pattern!");
					return false;
				}
			}
		}
	}
	if(!canSpawnStartNetherReactor(level, x, y, z, player)) return false;
	player->displayClientMessage("The reactor has been started, survive.");
	NetherReactorTileEntity* reactor = static_cast<NetherReactorTileEntity*>(level->getTileEntity(x, y, z));
	if (reactor != NULL) {
		reactor->lightItUp(x, y, z);
	}
	return true;
}

void NetherReactor::onPlace(Level* level, int x, int y, int z) {
	if (level->isClientSide) return;
	if (existsReactorOrPortalElsewhere(level, x, y, z)) {
		level->setTile(x, y, z, Tile::obsidian->id);
	}
}

TileEntity* NetherReactor::newTileEntity() {
	return TileEntityFactory::createTileEntity(TileEntityType::NetherReactor);
}

void NetherReactor::setPhase(Level* level, int x, int y, int z, int phase) {
	int curPhase = level->getData(x, y, z);
	if(curPhase != phase) {
		level->setData(x, y, z, phase);
	}
}

int NetherReactor::getTexture( int face, int data ) {
	switch(data) {
	case 1: return tex-1;
	case 2: return tex-2;
	default: return tex;
	}
}

bool NetherReactor::canSpawnStartNetherReactor( Level* level, int x, int y, int z, Player* player ) {
	if (level->dimension && level->dimension->id == -1) {
		player->displayClientMessage("The nether reactor cannot be activated in the nether.");
		return false;
	}
	if (existsReactorOrPortalElsewhere(level, x, y, z)) {
		player->displayClientMessage("Only one reactor can be activated in the overworld.");
		return false;
	}
	if(!allPlayersCloseToReactor(level, x, y, z)) {
		player->displayClientMessage("All players need to be close to the reactor.");
		return false;
	} else if(y > LEVEL_HEIGHT - 28) {
		player->displayClientMessage("The nether reactor needs to be built lower down.");
		return false;
	} else if(y < 2) {
		player->displayClientMessage("The nether reactor needs to be built higher up.");
		return false;
	}
	return true;
}

bool NetherReactor::allPlayersCloseToReactor( Level* level, int x, int y, int z ) {
	for(PlayerList::const_iterator i =  level->players.begin(); i != level->players.end(); ++i) {
		Player* currentPlayer = (*i);
		if(!(currentPlayer->x >= x - 5 && currentPlayer->x <= x + 5)) return false;
		if(!(currentPlayer->y - currentPlayer->heightOffset >= y -1 &&  currentPlayer->y - currentPlayer->heightOffset <= y + 1)) return false;
		if(!(currentPlayer->z >= z - 5 && currentPlayer->z <= z + 5)) return false;
	}
	return true;
}
