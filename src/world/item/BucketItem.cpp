#include "BucketItem.h"
#include "ItemInstance.h"
#include "Item.h"
#include "../level/tile/Tile.h"
#include "../level/Level.h"
#include "../entity/player/Player.h"
#include "../Facing.h"
#include "../phys/HitResult.h"
#include "../phys/AABB.h"
#include "../level/dimension/Dimension.h"
#include "../../util/Mth.h"
#include "../entity/EntityTypes.h"

bool BucketItem::useOn(ItemInstance* instance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
    if (contains == 0) {
        int tx = x;
        int ty = y;
        int tz = z;
        int tileId = level->getTile(tx, ty, tz);
        int data = level->getData(tx, ty, tz);

        if ((tileId == Tile::water->id || tileId == Tile::calmWater->id) && data == 0) {
            const Item* filled = Item::bucket_water;
            if (player != NULL && player->abilities.instabuild) {
                if (!level->isClientSide) level->setTile(tx, ty, tz, 0);
                if (!player->inventory->add(new ItemInstance(filled))) {
                    player->drop(new ItemInstance(filled), true);
                }
                return true;
            }
            if (!level->isClientSide) level->setTile(tx, ty, tz, 0);
            if (instance->count == 1) {
                instance->id = filled->id;
            } else {
                instance->count--;
                if (!player->inventory->add(new ItemInstance(filled))) {
                    player->drop(new ItemInstance(filled), true);
                }
            }
            return true;
        }

        if ((tileId == Tile::lava->id || tileId == Tile::calmLava->id) && data == 0) {
            const Item* filled = Item::bucket_lava;
            if (player != NULL && player->abilities.instabuild) {
                if (!level->isClientSide) level->setTile(tx, ty, tz, 0);
                if (!player->inventory->add(new ItemInstance(filled))) {
                    player->drop(new ItemInstance(filled), true);
                }
                return true;
            }
            if (!level->isClientSide) level->setTile(tx, ty, tz, 0);
            if (instance->count == 1) {
                instance->id = filled->id;
            } else {
                instance->count--;
                if (!player->inventory->add(new ItemInstance(filled))) {
                    player->drop(new ItemInstance(filled), true);
                }
            }
            return true;
        }

        switch (face) {
            case Facing::DOWN : y--; break;
            case Facing::UP   : y++; break;
            case Facing::NORTH: z--; break;
            case Facing::SOUTH: z++; break;
            case Facing::WEST : x--; break;
            case Facing::EAST : x++; break;
        }

        tileId = level->getTile(x, y, z);
        data = level->getData(x, y, z);

        if ((tileId == Tile::water->id || tileId == Tile::calmWater->id) && data == 0) {
            const Item* filled = Item::bucket_water;
            if (player != NULL && player->abilities.instabuild) {
                if (!level->isClientSide) level->setTile(x, y, z, 0);
                if (!player->inventory->add(new ItemInstance(filled))) {
                    player->drop(new ItemInstance(filled), true);
                }
                return true;
            }
            if (!level->isClientSide) level->setTile(x, y, z, 0);
            if (instance->count == 1) {
                instance->id = filled->id;
            } else {
                instance->count--;
                if (!player->inventory->add(new ItemInstance(filled))) {
                    player->drop(new ItemInstance(filled), true);
                }
            }
            return true;
        }

        if ((tileId == Tile::lava->id || tileId == Tile::calmLava->id) && data == 0) {
            const Item* filled = Item::bucket_lava;
            if (player != NULL && player->abilities.instabuild) {
                if (!level->isClientSide) level->setTile(x, y, z, 0);
                if (!player->inventory->add(new ItemInstance(filled))) {
                    player->drop(new ItemInstance(filled), true);
                }
                return true;
            }
            if (!level->isClientSide) level->setTile(x, y, z, 0);
            if (instance->count == 1) {
                instance->id = filled->id;
            } else {
                instance->count--;
                if (!player->inventory->add(new ItemInstance(filled))) {
                    player->drop(new ItemInstance(filled), true);
                }
            }
            return true;
        }

        return false;
    }

    if (contains > 0) {
        if (level->getTile(x, y, z) == Tile::topSnow->id) {
            face = 0;
        } else {
            switch (face) {
                case Facing::DOWN : y--; break;
                case Facing::UP   : y++; break;
                case Facing::NORTH: z--; break;
                case Facing::SOUTH: z++; break;
                case Facing::WEST : x--; break;
                case Facing::EAST : x++; break;
            }
        }

        if (instance->count == 0) return false;

        if (level->dimension && level->dimension->ultraWarm && (contains == Tile::water->id || contains == Tile::calmWater->id)) {
            level->playSound((float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f, "random.fizz", 0.5f, 2.6f + (level->random.nextFloat() - level->random.nextFloat()) * 0.8f);
            for (int i = 0; i < 8; i++) {
                level->addParticle(PARTICLETYPE(largesmoke), (float)x + Mth::random(), (float)y + Mth::random(), (float)z + Mth::random(), 0, 0, 0);
            }

            if (player != NULL && player->abilities.instabuild) {
                // creative
            } else {
                if (instance->count == 1) {
                    instance->id = Item::bucket_empty->id;
                } else {
                    instance->count--;
                    if (!player->inventory->add(new ItemInstance(Item::bucket_empty))) {
                        player->drop(new ItemInstance(Item::bucket_empty), true);
                    }
                }
            }
            return true;
        }

        if (level->mayPlace(contains, x, y, z, false, (unsigned char)face)) {
            if (!level->isClientSide) level->setTileAndData(x, y, z, contains, 0);

            if (player != NULL && player->abilities.instabuild) {
                // do nothing cuz player is in creative
            } else {
                if (instance->count == 1) {
                    instance->id = Item::bucket_empty->id;
                } else {
                    instance->count--;
                    if (!player->inventory->add(new ItemInstance(Item::bucket_empty))) {
                        player->drop(new ItemInstance(Item::bucket_empty), true);
                    }
                }
            }
            return true;
        }
        return false;
    }

    return false;
}

ItemInstance* BucketItem::use(ItemInstance* instance, Level* level, Player* player) {
    if (contains != 0 || player == NULL || instance == NULL) return instance;

    Vec3 from = player->getPos(1).add(0, player->getHeadHeight(), 0);
    Vec3 look = player->getViewVector(1);
    Vec3 to = from + look * 5.0f;

    Vec3 pv = look * 5.0f;
    AABB aabb = player->bb.expand(pv.x, pv.y, pv.z).grow(1.0f, 1.0f, 1.0f);
    EntityList& objects = level->getEntities(player, aabb);
    Entity* hovered = NULL;
    float nearest = 0;
    for (unsigned int i = 0; i < objects.size(); i++) {
        Entity* e = objects[i];
        if (!e->isPickable()) continue;

        float rr = e->getPickRadius();
        AABB eb = e->bb.grow(rr, rr, rr);
        HitResult p = eb.clip(from, to);

        if (eb.contains(from)) {
            if (nearest >= 0) {
                hovered = e;
                nearest = 0;
            }
        } else if (p.isHit()) {
            float dd = from.distanceTo(p.pos);
            if (dd < nearest || nearest == 0) {
                hovered = e;
                nearest = dd;
            }
        }
    }

    if (hovered != NULL) {
        float distToBlock = from.distanceTo(to);
        if (nearest < distToBlock) {
            if (contains == 0 && hovered->isEntityType(MobTypes::Cow)) {
                const Item* milkItem = Item::milk;
                if (player->abilities.instabuild) {
                    if (!player->inventory->add(new ItemInstance(milkItem))) player->drop(new ItemInstance(milkItem), true);
                    return instance;
                }
                if (instance->count == 1) {
                    instance->id = milkItem->id;
                } else {
                    instance->count--;
                    if (!player->inventory->add(new ItemInstance(milkItem))) player->drop(new ItemInstance(milkItem), true);
                }
                return instance;
            }
        }
    }

    return instance;
}
