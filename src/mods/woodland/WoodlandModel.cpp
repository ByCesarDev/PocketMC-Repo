#ifndef STANDALONE_SERVER
#include "WoodlandModel.h"

#include <cmath>
#include <cstring>

// Bedrock -> ModelPart sign conventions. If limbs swing the wrong way, flip these.
static const float ROT_SIGN_X = -1.0f, ROT_SIGN_Y = -1.0f, ROT_SIGN_Z = 1.0f;
static const float POS_SIGN_X = 1.0f,  POS_SIGN_Y = -1.0f, POS_SIGN_Z = 1.0f;
static const float DEG2RAD = 3.14159265f / 180.0f;

WoodlandModel::WoodlandModel(const WlMobDef* def)
    : idleAnim(0), walkAnim(0), hasModel(false)
{
    for (int i = 0; i < WL_GEO_COUNT; ++i) {
        if (def->geo[0] && strcmp(WL_GEOS[i].id, def->geo) == 0) {
            build(&WL_GEOS[i]);
            hasModel = true;
            break;
        }
    }
    if (def->animPrefix[0]) {
        idleAnim = findAnim(def->animPrefix, "idle", "stand");
        walkAnim = findAnim(def->animPrefix, "walk", "run");
    }
    apply(0, 0);   // rest pose
}

WoodlandModel::~WoodlandModel()
{
    for (size_t i = 0; i < parts.size(); ++i) delete parts[i].part;
}

void WoodlandModel::build(const WlGeo* geo)
{
    // ENGINE: ModelPart(int u, int v), setTexSize(w, h), texOffs(u, v), addBox(x, y, z, w, h, d, inflate),
    // setPos(x, y, z), addChild(part), render(scale) and public xRot/yRot/zRot/x/y/z/mirror.
    // Edit only this function if your ModelPart differs.
    for (int i = 0; i < geo->boneCount; ++i) {
        const WlBone* b = &WL_BONES[geo->boneStart + i];
        Part p;
        p.bone = b;
        p.part = new ModelPart(0, 0);
        p.part->setTexSize(geo->texW, geo->texH);
        p.parent = -1;
        for (int k = 0; k < b->cubeCount; ++k) {
            const WlCube* c = &WL_CUBES[b->cubeStart + k];
            p.part->mirror = c->mirror != 0;
            p.part->texOffs((int)c->u, (int)c->v);
            // Bedrock (y up, absolute pivots) -> ModelPart (y down, pivot-relative).
            p.part->addBox(c->ox - b->px, b->py - c->oy - c->sy, c->oz - b->pz,
                           (int)(c->sx + 0.5f) < 1 ? 1 : (int)(c->sx + 0.5f),
                           (int)(c->sy + 0.5f) < 1 ? 1 : (int)(c->sy + 0.5f),
                           (int)(c->sz + 0.5f) < 1 ? 1 : (int)(c->sz + 0.5f), c->inflate);
        }
        parts.push_back(p);
    }
    // Link parents (a bone may be listed before its parent).
    for (size_t i = 0; i < parts.size(); ++i) {
        const WlBone* b = parts[i].bone;
        for (size_t j = 0; b->parent[0] && j < parts.size(); ++j)
            if (strcmp(parts[j].bone->name, b->parent) == 0) { parts[i].parent = (int)j; break; }
    }
    for (size_t i = 0; i < parts.size(); ++i) {
        Part& p = parts[i];
        const WlBone* b = p.bone;
        if (p.parent < 0) { p.restX = b->px; p.restY = 24.0f - b->py; p.restZ = b->pz; }
        else {
            const WlBone* pb = parts[p.parent].bone;
            p.restX = b->px - pb->px; p.restY = pb->py - b->py; p.restZ = b->pz - pb->pz;
            parts[p.parent].part->addChild(p.part);
        }
        p.restXRot = ROT_SIGN_X * b->rx * DEG2RAD;
        p.restYRot = ROT_SIGN_Y * b->ry * DEG2RAD;
        p.restZRot = ROT_SIGN_Z * b->rz * DEG2RAD;
    }
}

const WlAnim* WoodlandModel::findAnim(const char* prefix, const char* a, const char* b) const
{
    size_t n = strlen(prefix);
    const WlAnim* fallback = 0;
    for (int i = 0; i < WL_ANIM_COUNT; ++i) {
        const WlAnim* an = &WL_ANIMS[i];
        if (strncmp(an->name, prefix, n) != 0) continue;
        if (strstr(an->name + n, a)) return an;
        if (strstr(an->name + n, b) && !fallback) fallback = an;
    }
    return fallback;
}

static void sampleChannel(const WlChannel* ch, float t, float out[3])
{
    const WlKey* k = &WL_KEYS[ch->keyStart];
    int n = ch->keyCount;
    if (n == 1 || t <= k[0].t) { out[0] = k[0].x; out[1] = k[0].y; out[2] = k[0].z; return; }
    if (t >= k[n - 1].t) { out[0] = k[n - 1].x; out[1] = k[n - 1].y; out[2] = k[n - 1].z; return; }
    for (int i = 0; i + 1 < n; ++i) {
        if (t >= k[i].t && t <= k[i + 1].t) {
            float span = k[i + 1].t - k[i].t;
            float f = span > 0 ? (t - k[i].t) / span : 0;
            out[0] = k[i].x + (k[i + 1].x - k[i].x) * f;
            out[1] = k[i].y + (k[i + 1].y - k[i].y) * f;
            out[2] = k[i].z + (k[i + 1].z - k[i].z) * f;
            return;
        }
    }
}

void WoodlandModel::apply(const WlAnim* anim, float seconds)
{
    for (size_t i = 0; i < parts.size(); ++i) {
        Part& p = parts[i];
        p.part->x = p.restX; p.part->y = p.restY; p.part->z = p.restZ;
        p.part->xRot = p.restXRot; p.part->yRot = p.restYRot; p.part->zRot = p.restZRot;
    }
    if (!anim) return;

    float t = seconds;
    if (anim->length > 0) {
        if (anim->loop == 1) t = fmodf(seconds, anim->length);
        else if (t > anim->length) t = anim->length;
    }
    for (int c = 0; c < anim->chanCount; ++c) {
        const WlChannel* ch = &WL_CHANNELS[anim->chanStart + c];
        for (size_t i = 0; i < parts.size(); ++i) {
            if (strcmp(parts[i].bone->name, ch->bone) != 0) continue;
            float v[3];
            sampleChannel(ch, t, v);
            Part& p = parts[i];
            if (ch->type == 0) {
                p.part->xRot += ROT_SIGN_X * v[0] * DEG2RAD;
                p.part->yRot += ROT_SIGN_Y * v[1] * DEG2RAD;
                p.part->zRot += ROT_SIGN_Z * v[2] * DEG2RAD;
            } else if (ch->type == 1) {
                p.part->x += POS_SIGN_X * v[0];
                p.part->y += POS_SIGN_Y * v[1];
                p.part->z += POS_SIGN_Z * v[2];
            }
            // type 2 (scale) is ignored: ModelPart has no scale.
            break;
        }
    }
}

void WoodlandModel::render(Mob* mob, float time, float r, float bob, float yRot, float xRot, float scale)
{
    if (!hasModel) return;
    // bob = mob age in ticks (+ partial tick); r = walk speed.
    const WlAnim* anim = (r > 0.02f && walkAnim) ? walkAnim : idleAnim;
    apply(anim, bob / 20.0f);
    for (size_t i = 0; i < parts.size(); ++i)
        if (parts[i].parent < 0) parts[i].part->render(scale);
}
#endif