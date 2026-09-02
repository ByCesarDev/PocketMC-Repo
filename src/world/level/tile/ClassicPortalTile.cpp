#include "ClassicPortalTile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "FireTile.h"
#include "../../entity/Entity.h"
#include "../../entity/player/Player.h"
#include "../../../util/Mth.h"
#include "../../Facing.h"
#include "../../phys/AABB.h"

ClassicPortalTile::ClassicPortalTile(int id, int tex, const Material* material)
    : super(id, tex, material)
{
}

AABB* ClassicPortalTile::getAABB(Level* level, int x, int y, int z)
{
    return nullptr;
}

bool ClassicPortalTile::isCubeShaped()
{
    return false;
}

void ClassicPortalTile::updateShape(LevelSource* level, int x, int y, int z)
{
    float var5, var6;
    if (level->getTile(x - 1, y, z) != id && level->getTile(x + 1, y, z) != id) {
        var5 = 0.125f;
        var6 = 0.5f;
    } else {
        var5 = 0.5f;
        var6 = 0.125f;
    }
    setShape(0.5f - var5, 0.0f, 0.5f - var6, 0.5f + var5, 1.0f, 0.5f + var6);
}

int ClassicPortalTile::getRenderLayer()
{
    return RENDERLAYER_BLEND;
}

int ClassicPortalTile::getResourceCount(Random* random)
{
    return 0;
}

void ClassicPortalTile::neighborChanged(Level* level, int x, int y, int z, int type)
{
    int xDiff = 0, zDiff = 1;
    if (level->getTile(x - 1, y, z) == id || level->getTile(x + 1, y, z) == id) {
        xDiff = 1;
        zDiff = 0;
    }

    int tpX = x, tpY = y, tpZ = z;
    while (level->getTile(tpX, tpY - 1, tpZ) == id) {
        tpY--;
    }

    if (level->getTile(tpX, tpY - 1, tpZ) != Tile::obsidian->id) {
        level->setTile(x, y, z, 0);
        return;
    }

    int var9;
    for (var9 = 1; var9 < 4 && level->getTile(tpX, tpY + var9, tpZ) == id; var9++) {
    }

    if (var9 == 3 && level->getTile(tpX, tpY + var9, tpZ) == Tile::obsidian->id) {
        bool var10 = level->getTile(x - 1, y, z) == id || level->getTile(x + 1, y, z) == id;
        bool var11 = level->getTile(x, y, z - 1) == id || level->getTile(x, y, z + 1) == id;
        if (var10 && var11) {
            level->setTile(x, y, z, 0);
        } else if ((level->getTile(x + xDiff, y, z + zDiff) != Tile::obsidian->id || level->getTile(x - xDiff, y, z - zDiff) != id)
            && (level->getTile(x - xDiff, y, z - zDiff) != Tile::obsidian->id || level->getTile(x + xDiff, y, z + zDiff) != id)) {
            level->setTile(x, y, z, 0);
        }
    } else {
        level->setTile(x, y, z, 0);
    }
}

void ClassicPortalTile::entityInside(Level* level, int x, int y, int z, Entity* entity)
{
    if (!entity->isPlayer())
        return;

    float var5, var6;
    if (level->getTile(x - 1, y, z) != id && level->getTile(x + 1, y, z) != id) {
        var5 = 0.125f;
        var6 = 0.5f;
    } else {
        var5 = 0.5f;
        var6 = 0.125f;
    }

    AABB portalShape(
        x + 0.5f - var5, y + 0.0f, z + 0.5f - var6,
        x + 0.5f + var5, y + 1.0f, z + 0.5f + var6
    );

    if (entity->bb.intersects(portalShape))
        ((Player*)entity)->handleInsideClassicPortal();
}

bool ClassicPortalTile::shouldRenderFace(LevelSource* level, int x, int y, int z, int face)
{
    if (level->getTile(x, y, z) == id) {
        return false;
    }
    bool w = level->getTile(x - 1, y, z) == id && level->getTile(x - 2, y, z) != id;
    bool e = level->getTile(x + 1, y, z) == id && level->getTile(x + 2, y, z) != id;
    bool n = level->getTile(x, y, z - 1) == id && level->getTile(x, y, z - 2) != id;
    bool s = level->getTile(x, y, z + 1) == id && level->getTile(x, y, z + 2) != id;
    bool we = w || e;
    bool ns = n || s;
    return (we && (face == Facing::WEST || face == Facing::EAST)) || (ns && (face == Facing::NORTH || face == Facing::SOUTH));
}

void ClassicPortalTile::animateTick(Level* level, int x, int y, int z, Random* random)
{
    if (random->nextInt(100) == 0) {
        level->playSound((float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f, "portal.portal", 1.0f, random->nextFloat() * 0.4f + 0.8f);
    }

    for (int i = 0; i < 4; i++) {
        float var7 = (float)x + random->nextFloat();
        float var9 = (float)y + random->nextFloat();
        float var11 = (float)z + random->nextFloat();
        float velX, velY, velZ;
        int var19 = random->nextInt(2) * 2 - 1;
        velX = (random->nextFloat() - 0.5f) * 0.5f;
        velY = (random->nextFloat() - 0.5f) * 0.5f;
        velZ = (random->nextFloat() - 0.5f) * 0.5f;

        if (level->getTile(x - 1, y, z) != id && level->getTile(x + 1, y, z) != id) {
            var7 = (float)x + 0.5f + 0.25f * var19;
            velX = random->nextFloat() * 2.0f * var19;
        } else {
            var11 = (float)z + 0.5f + 0.25f * var19;
            velZ = random->nextFloat() * 2.0f * var19;
        }

        level->addParticle("portal", var7, var9, var11, velX, velY, velZ);
    }
}

bool ClassicPortalTile::trySpawnPortal(Level* level, int x, int y, int z)
{
    int xDiff = 0, zDiff = 0;
    if (level->getTile(x - 1, y, z) == Tile::obsidian->id || level->getTile(x + 1, y, z) == Tile::obsidian->id)
        xDiff = 1;
    if (level->getTile(x, y, z - 1) == Tile::obsidian->id || level->getTile(x, y, z + 1) == Tile::obsidian->id)
        zDiff = 1;

    if (xDiff == zDiff)
        return false;

    int tpX = x, tpY = y, tpZ = z;
    if (level->isEmptyTile(tpX - xDiff, y, tpZ - zDiff)) {
        tpX -= xDiff;
        tpZ -= zDiff;
    }

    for (int var7 = -1; var7 <= 2; var7++) {
        for (int var8 = -1; var8 <= 3; var8++) {
            bool isFrame = (var7 == -1 || var7 == 2 || var8 == -1 || var8 == 3);
            if (var7 != -1 && var7 != 2 || var8 != -1 && var8 != 3) {
                int nx = tpX + xDiff * var7;
                int ny = y + var8;
                int nz = tpZ + zDiff * var7;
                int existing = level->getTile(nx, ny, nz);
                if (isFrame) {
                    if (existing != Tile::obsidian->id)
                        return false;
                } else {
                    if (existing != 0 && existing != Tile::fire->id)
                        return false;
                }
            }
        }
    }

    level->noNeighborUpdate = true;

    for (int var7 = 0; var7 < 2; var7++) {
        for (int var8 = 0; var8 < 3; var8++) {
            int nx = tpX + xDiff * var7;
            int ny = y + var8;
            int nz = tpZ + zDiff * var7;
            level->setTile(nx, ny, nz, Tile::classicPortal->id);
        }
    }

    level->noNeighborUpdate = false;

    for (int var7 = 0; var7 < 2; var7++) {
        for (int var8 = 0; var8 < 3; var8++) {
            int nx = tpX + xDiff * var7;
            int ny = y + var8;
            int nz = tpZ + zDiff * var7;
            level->updateNeighborsAt(nx, ny, nz, Tile::classicPortal->id);
        }
    }

    return true;
}
