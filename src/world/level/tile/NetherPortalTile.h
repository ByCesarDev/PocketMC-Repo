#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__NetherPortalTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__NetherPortalTile_H__

#include "Tile.h"
#include "../Level.h"
#include "../../entity/player/Player.h"
#include "../material/Material.h"
#include "../dimension/Dimension.h"
#include "../LevelConstants.h"
#include "../chunk/LevelChunk.h"
#include "entity/NetherReactorTileEntity.h"
#include <chrono>
#include <map>


class NetherPortalTile : public Tile {
public:
    NetherPortalTile(int id, int tex)
    : Tile(id, tex, Material::glass) {
        // Material::glass allows transparency and doesn't block light
    }

    bool isSolidRender() override { return false; }

    bool isCubeShaped() override { return true; }

    bool mayPick() override { return true; }

    AABB* getAABB(Level* level, int x, int y, int z) override {
        return Tile::getAABB(level, x, y, z);
    }

    void onPlace(Level* level, int x, int y, int z) override {
        // Enforce 1 portal block max in the Nether
        if (level->dimension && level->dimension->id == -1) {
            int existingX = 0, existingY = 0, existingZ = 0;
            if (findExistingPortal(level, x, z, existingX, existingY, existingZ)) {
                if (existingX != x || existingY != y || existingZ != z) {
                    level->setTile(x, y, z, Tile::obsidian->id);
                }
            }
        }
    }

    bool findExistingPortal(Level* level, int startX, int startZ, int& outX, int& outY, int& outZ) {
        int centerCx = startX >> 4;
        int centerCz = startZ >> 4;
        int r = 8;
        for (int cx = centerCx - r; cx <= centerCx + r; cx++) {
            for (int cz = centerCz - r; cz <= centerCz + r; cz++) {
                LevelChunk* chunk = level->getChunk(cx, cz);
                if (!chunk) continue;
                for (int x = 0; x < 16; ++x) {
                    for (int z = 0; z < 16; ++z) {
                        for (int y = 0; y < 128; ++y) {
                            if (chunk->getTile(x, y, z) == id) {
                                outX = chunk->xt + x;
                                outY = y;
                                outZ = chunk->zt + z;
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    void teleport(Level* level, Player* player) {
        if (level->isClientSide) return;

        bool inNether = (level->dimension && level->dimension->id == -1);
        int targetDimId = inNether ? Dimension::NORMAL_DAYCYCLE : -1;

        // Switch the dimension (saves current world, loads new one, manages ProgressScreen)
        level->changeDimension(targetDimId);

        // Find if there is already a portal in the destination world
        int destX = 0, destY = 0, destZ = 0;
        if (findExistingPortal(level, (int)player->x, (int)player->z, destX, destY, destZ)) {
            // Teleport player on top of the existing portal block (2 blocks up)
            player->setPos((float)destX + 0.5f, (float)destY + 2.0f, (float)destZ + 0.5f);
        } else {
            // If no portal exists, generate one at corresponding/safe coordinates
            float oldX = player->x;
            float oldZ = player->z;
            int landingX = (int)oldX;
            int landingZ = (int)oldZ;
            int safeY = 80;

            for (int checkY = 120; checkY > 10; checkY--) {
                int groundId = level->getTile(landingX, checkY - 1, landingZ);
                int air1     = level->getTile(landingX, checkY,     landingZ);
                int air2     = level->getTile(landingX, checkY + 1, landingZ);

                bool groundSolid = (groundId != 0 && groundId != Tile::lava->id
                                                  && groundId != Tile::calmLava->id);
                if (groundSolid && air1 == 0 && air2 == 0) {
                    safeY = checkY;
                    break;
                }
            }

            // Build a minimal safe platform if nothing found
            int platformBlock = (targetDimId == -1) ? Tile::netherrack->id : Tile::stoneBrick->id;
            if (level->getTile(landingX, safeY - 1, landingZ) == 0 ||
                level->getTile(landingX, safeY - 1, landingZ) == Tile::lava->id)
            {
                level->setTile(landingX, safeY - 1, landingZ, platformBlock);
            }
            // Clear headroom
            level->setTile(landingX, safeY,     landingZ, 0);
            level->setTile(landingX, safeY + 1, landingZ, 0);

            // Place exactly one portal block
            level->setTile(landingX, safeY, landingZ, Tile::netherPortal->id);

            // Teleport player on top of the portal block (2 blocks up)
            player->setPos((float)landingX + 0.5f, (float)safeY + 2.0f, (float)landingZ + 0.5f);
        }
    }

    bool use(Level* level, int x, int y, int z, Player* player) override {
        if (level->isClientSide) return true;

        // Check Gold Block reactivation
        ItemInstance* carried = player->getCarriedItem();
        if (carried && carried->id == Tile::goldBlock->id) {
            if (level->dimension && level->dimension->id == -1) {
                player->displayClientMessage("Cannot reactivate the reactor in the nether.");
                return true;
            }
            // Consume Gold Block
            carried->count--;
            if (carried->count <= 0) {
                player->inventory->clearSlot(player->inventory->selected);
            }
            level->setTile(x, y, z, Tile::netherReactor->id);
            player->displayClientMessage("The reactor has been reactivated!");
            NetherReactorTileEntity* reactor = static_cast<NetherReactorTileEntity*>(level->getTileEntity(x, y, z));
            if (reactor != NULL) {
                reactor->resetForReactivation();
                reactor->lightItUp(x, y, z);
            }
            return true;
        }

        static std::map<Player*, std::chrono::steady_clock::time_point> cooldowns;
        auto now = std::chrono::steady_clock::now();
        if (cooldowns.find(player) != cooldowns.end()) {
            if (std::chrono::duration_cast<std::chrono::seconds>(now - cooldowns[player]).count() < 4) {
                return true;
            }
        }
        cooldowns[player] = now;

        teleport(level, player);
        return true;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__NetherPortalTile_H__*/