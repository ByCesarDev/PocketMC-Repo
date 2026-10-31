#ifndef POCKETMC_MODS_WOODLAND_MOB_H__
#define POCKETMC_MODS_WOODLAND_MOB_H__

#include "../../world/entity/PathfinderMob.h"
#include "WoodlandData.h"

// One class for all 33 woodland mobs. Stats and behaviour flags come from WL_MOBS
// (generated from the Java entities' attributes and goals).
class WoodlandMob : public PathfinderMob {
public:
    WoodlandMob(Level* level, int mobIndex);

    virtual int  getMaxHealth();
    virtual int  getEntityTypeId() const;
    virtual void updateAi();
    virtual Entity* findAttackTarget();
    virtual void checkHurtTarget(Entity* target, float distance);
    virtual void die(Entity* cause);

    const WlMobDef* def;
    int index;

private:
    void woodify(int radius);
};

#endif
