#ifndef POCKETMC_MODS_WOODLAND_MODEL_H__
#define POCKETMC_MODS_WOODLAND_MODEL_H__

#include "../../client/model/Model.h"
#include "../../client/model/geom/ModelPart.h"
#include "WoodlandData.h"

#include <vector>

// Builds a Model from the Bedrock .geo.json data (WL_GEOS) and plays the converted
// GeckoLib/Bedrock animations (WL_ANIMS): "idle" while standing, "walk" while moving.
class WoodlandModel : public Model {
public:
    WoodlandModel(const WlMobDef* def);
    virtual ~WoodlandModel();

    // ENGINE: check Model::render's signature in your Model.h.
    virtual void render(Mob* mob, float time, float r, float bob, float yRot, float xRot, float scale);

private:
    struct Part {
        ModelPart* part;
        const WlBone* bone;
        int parent;                 // index into parts, -1 = root
        float restX, restY, restZ, restXRot, restYRot, restZRot;
    };

    void build(const WlGeo* geo);
    const WlAnim* findAnim(const char* prefix, const char* a, const char* b) const;
    void apply(const WlAnim* anim, float seconds);

    std::vector<Part> parts;
    const WlAnim* idleAnim;
    const WlAnim* walkAnim;
    bool hasModel;
};

#endif
