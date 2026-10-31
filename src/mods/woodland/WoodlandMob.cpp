#include "WoodlandMob.h"
#include "WoodlandMod.h"

#include "../../world/entity/player/Player.h"
#include "../../world/level/Level.h"
#include "../../world/level/storage/LevelData.h"

#include <algorithm>
#include <cmath>
#include <cstring>

WoodlandMob::WoodlandMob(Level* level, int mobIndex)
    : PathfinderMob(level), def(&WL_MOBS[mobIndex]), index(mobIndex)
{
    // ENGINE: texture path member, size and speed field names.
    textureName = def->tex;          // e.g. "mods/woodland/textures/entity/wooden_one.png"
    setSize(def->w, def->h);
    health = getMaxHealth();
    
    // 0.6.1 'speed' is private. PathfinderMob uses 'runSpeed' for its movement logic.
    runSpeed = def->speed;
    
    // Direct match with the ID registered in EntityRenderDispatcher
    this->entityRendererId = (EntityRendererId)(WoodlandMod::EntityTypeBase + index);
}

int WoodlandMob::getMaxHealth() { return (int)def->health; }

int WoodlandMob::getEntityTypeId() const { return WoodlandMod::EntityTypeBase + index; }

Entity* WoodlandMob::findAttackTarget()
{
    if (!(def->flags & (WL_MOB_AGGRESSIVE | WL_MOB_FOLLOWS))) return 0;
    if (def->speed <= 0.0f) return 0;
    
    float range = std::min(def->follow, 64.0f);   // pathfinding that far is too slow on a phone
    Player* p = level->getNearestPlayer(this, range);
    
    // 0.6.1 uses LevelData to check if the world is Creative (1 = Creative, 0 = Survival)
    bool isCreative = (level->getLevelData() && level->getLevelData()->getGameType() == 1);
    
    return (p && !isCreative && canSee(p)) ? p : 0;
}

void WoodlandMob::updateAi()
{
    if (def->speed <= 0.0f) {
        // Standers (oak_stander, distant_mimic, wooddie): never walk, just stare.
        Player* p = level->getNearestPlayer(this, 24.0f);
        if (p) lookAt(p, 30.0f, 30.0f);
        return;
    }
    PathfinderMob::updateAi();   // pathing toward findAttackTarget() / random wandering
}

void WoodlandMob::checkHurtTarget(Entity* target, float d)
{
    if (!(def->flags & WL_MOB_AGGRESSIVE) || def->damage <= 0.0f) return;
    if (d < 2.5f && attackTime <= 0 && target->bb.y1 > bb.y0 && target->bb.y0 < bb.y1) {
        attackTime = 20;
        int dmg = (int)(def->damage + 0.5f);
        target->hurt(this, dmg < 1 ? 1 : dmg);
    }
}

void WoodlandMob::die(Entity* cause)
{
    // Original WoodenEntity: on death the surrounding area turns into planks.
    // 0.6.1 uses `isClientSide` instead of `isOnline`
    if (!level->isClientSide && strcmp(def->name, "wooden") == 0) woodify(12);
    PathfinderMob::die(cause);
}

void WoodlandMob::woodify(int r)
{
    int cx = (int)floorf(x), cy = (int)floorf(y), cz = (int)floorf(z);
    for (int dx = -r; dx <= r; ++dx)
        for (int dz = -r; dz <= r; ++dz)
            for (int dy = -r / 2; dy <= r / 2; ++dy) {
                int t = level->getTile(cx + dx, cy + dy, cz + dz);
                if (t == 0 || t == 7 /*bedrock*/ || t == 8 || t == 9 || t == 10 || t == 11 || t == 54 /*chest*/) continue;
                level->setTile(cx + dx, cy + dy, cz + dz, 5 /*planks*/);
            }
}