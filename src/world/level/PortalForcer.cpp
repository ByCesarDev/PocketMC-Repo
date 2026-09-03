#include "PortalForcer.h"
#include "tile/ClassicPortalTile.h"
#include "tile/Tile.h"
#include "material/Material.h"
#include "LightLayer.h"
#include "../entity/Entity.h"
#include "../../util/Mth.h"

#ifndef STANDALONE_SERVER
#include "../../platform/log.h"
#else
#define LOGI(...)
#endif

void PortalForcer::force(Level* level, Entity* entity)
{
    LOGI("[PortalForcer::force] START: entity=%p, dim=%d, entityPos=(%.2f, %.2f, %.2f), heightOffset=%.2f\n",
         entity, level ? (level->dimension ? level->dimension->id : 0) : 0, entity->x, entity->y, entity->z, entity->heightOffset);
    if (!findPortal(level, entity)) {
        LOGI("[PortalForcer::force] No portal found. Calling createPortal...\n");
        createPortal(level, entity);
        findPortal(level, entity);
    }
    LOGI("[PortalForcer::force] END: entityPos=(%.2f, %.2f, %.2f)\n", entity->x, entity->y, entity->z);
}

bool PortalForcer::findPortal(Level* level, Entity* entity)
{
    int searchRadius = (level && level->dimension && level->dimension->id == -1) ? 16 : 128;
    double bestDist = -1.0;
    int bestX = 0, bestY = 0, bestZ = 0;

    int px = Mth::floor(entity->x);
    int pz = Mth::floor(entity->z);

    LOGI("[PortalForcer::findPortal] Searching for portal near (%d, %d), radius=%d in dim=%d\n",
         px, pz, searchRadius, level ? (level->dimension ? level->dimension->id : 0) : 0);

    int pcx = px >> 4;
    int pcz = pz >> 4;
    int cRadius = (searchRadius >> 4) + 1;
    for (int cx = pcx - cRadius; cx <= pcx + cRadius; cx++) {
        for (int cz = pcz - cRadius; cz <= pcz + cRadius; cz++) {
            level->getChunk(cx, cz);
        }
    }

    for (int x = px - searchRadius; x <= px + searchRadius; x++) {
        double dx = x + 0.5 - entity->x;
        for (int z = pz - searchRadius; z <= pz + searchRadius; z++) {
            if (!level->hasChunk(x >> 4, z >> 4)) continue;
            double dz = z + 0.5 - entity->z;
            for (int y = Level::DEPTH - 1; y >= 0; y--) {
                if (level->getTile(x, y, z) == Tile::classicPortal->id) {
                    while (level->getTile(x, y - 1, z) == Tile::classicPortal->id)
                        y--;

                    double dy = y + 0.5 - entity->y;
                    double dist = dx * dx + dy * dy + dz * dz;
                    if (bestDist < 0.0 || dist < bestDist) {
                        bestDist = dist;
                        bestX = x;
                        bestY = y;
                        bestZ = z;
                    }
                }
            }
        }
    }

    if (bestDist >= 0.0) {
        float moveX = (float)bestX + 0.5f;
        float moveY = (float)bestY;
        float moveZ = (float)bestZ + 0.5f;

        bool isXAxis = (level->getTile(bestX - 1, bestY, bestZ) == Tile::classicPortal->id ||
                        level->getTile(bestX + 1, bestY, bestZ) == Tile::classicPortal->id);

        LOGI("[PortalForcer::findPortal] FOUND portal at (%d, %d, %d) dist=%.2f, isXAxis=%d\n",
             bestX, bestY, bestZ, bestDist, isXAxis);
        LOGI("[PortalForcer::findPortal] Moving entity to feet=(%.2f, %.2f, %.2f)\n", moveX, moveY, moveZ);

        entity->moveTo(moveX, moveY, moveZ, entity->yRot, entity->xRot);
        entity->xd = entity->yd = entity->zd = 0;
        return true;
    }

    LOGI("[PortalForcer::findPortal] NO portal found within search radius.\n");
    return false;
}

bool PortalForcer::createPortal(Level* level, Entity* entity)
{
    const int searchRadius = 16;
    double bestDistance = -1.0;
    int px = Mth::floor(entity->x);
    int py = Mth::floor(entity->y);
    int pz = Mth::floor(entity->z);

    // Preload chunks around player in destination dimension so terrain can be searched
    int pcx = px >> 4;
    int pcz = pz >> 4;
    for (int cx = pcx - 2; cx <= pcx + 2; cx++) {
        for (int cz = pcz - 2; cz <= pcz + 2; cz++) {
            level->getChunk(cx, cz);
        }
    }

    int bestX = px, bestY = py, bestZ = pz;
    int bestOrientation = 0;

    int baseOrientation = m_random.nextInt(4);

    for (int x = px - searchRadius; x <= px + searchRadius; x++) {
        double dx = x + 0.5 - entity->x;
        for (int z = pz - searchRadius; z <= pz + searchRadius; z++) {
            if (!level->hasChunk(x >> 4, z >> 4)) continue;
            double dz = z + 0.5 - entity->z;
            for (int y = Level::DEPTH - 1; y >= 0; y--) {
                if (!level->isEmptyTile(x, y, z)) continue;

                int ny = y;
                while (ny > 0 && level->isEmptyTile(x, ny - 1, z)) ny--;

                for (int o = baseOrientation; o < baseOrientation + 4; o++) {
                    int dxo = o % 2;
                    int dzo = 1 - dxo;
                    if (o % 4 >= 2) {
                        dxo = -dxo;
                        dzo = -dzo;
                    }

                    bool valid = true;
                    for (int w = 0; w < 3 && valid; w++) {
                        for (int h = 0; h < 4 && valid; h++) {
                            for (int d = -1; d < 4 && valid; d++) {
                                int tx = x + (h - 1) * dxo + w * dzo;
                                int ty = ny + d;
                                int tz = z + (h - 1) * dzo - w * dxo;

                                if ((d < 0 && !level->getMaterial(tx, ty, tz)->isSolid()) ||
                                    (d >= 0 && !level->isEmptyTile(tx, ty, tz))) {
                                    valid = false;
                                }
                            }
                        }
                    }

                    if (valid) {
                        double dy = ny + 0.5 - entity->y;
                        double dist = dx * dx + dy * dy + dz * dz;
                        if (bestDistance < 0.0 || dist < bestDistance) {
                            bestDistance = dist;
                            bestX = x;
                            bestY = ny;
                            bestZ = z;
                            bestOrientation = o % 4;
                        }
                    }
                }
            }
        }
    }

    int dxo = bestOrientation % 2;
    int dzo = 1 - dxo;
    if (bestOrientation >= 2) {
        dxo = -dxo;
        dzo = -dzo;
    }

    LOGI("[PortalForcer::createPortal] START: entity at (%d, %d, %d) in dim=%d\n",
         px, py, pz, level ? (level->dimension ? level->dimension->id : 0) : 0);

    if (bestDistance < 0.0) {
        bestY = Mth::clamp(py, 32, 96);
        LOGI("[PortalForcer::createPortal] NO valid natural ground found! Fallback bestY=%d (py=%d), chamber at (%d, %d, %d)\n",
             bestY, py, bestX, bestY, bestZ);
    } else {
        LOGI("[PortalForcer::createPortal] FOUND valid ground at (%d, %d, %d), orientation=%d, dist=%.2f\n",
             bestX, bestY, bestZ, bestOrientation, bestDistance);
    }

    // Clear minimal air space inside and directly around the portal frame
    for (int w = -1; w <= 1; w++) {
        for (int h = 0; h < 4; h++) {
            for (int d = 0; d < 3; d++) {
                int tx = bestX + (h - 1) * dxo + w * dzo;
                int ty = bestY + d;
                int tz = bestZ + (h - 1) * dzo - w * dxo;
                level->setTile(tx, ty, tz, 0);
            }
        }
    }

    level->noNeighborUpdate = true;
    for (int h = 0; h < 4; h++) {
        for (int d = -1; d < 4; d++) {
            int tx = bestX + (h - 1) * dxo;
            int ty = bestY + d;
            int tz = bestZ + (h - 1) * dzo;
            bool isFrame = h == 0 || h == 3 || d == -1 || d == 3;
            level->setTile(tx, ty, tz, isFrame ? Tile::obsidian->id : Tile::classicPortal->id);
        }
    }
    level->noNeighborUpdate = false;

    // Force block lighting recalculation in the newly carved chamber
    int minX = bestX - 4, maxX = bestX + 4;
    int minY = bestY - 1, maxY = bestY + 5;
    int minZ = bestZ - 4, maxZ = bestZ + 4;
    level->updateLight(LightLayer::Block, minX, minY, minZ, maxX, maxY, maxZ);

    return true;
}
