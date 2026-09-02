#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__FireTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__FireTile_H__

//package net.minecraft.world.level.tile;

#include "../../../util/Random.h"
#include "../material/Material.h"
#include "../Level.h"

#include "Tile.h"
#include "ClassicPortalTile.h"

class FireTile: public Tile
{
    int flameOdds[256];
    int burnOdds[256];
public:
    static const int FLAME_INSTANT = 60;
    static const int FLAME_EASY = 30;
    static const int FLAME_MEDIUM = 15;
    static const int FLAME_HARD = 5;

    static const int BURN_INSTANT = 100;
    static const int BURN_EASY = 60;
    static const int BURN_MEDIUM = 20;
    static const int BURN_HARD = 5;
    static const int BURN_NEVER = 0;

	FireTile(int id, int tex)
	:	Tile(id, tex, Material::fire)
	{
		for (int i = 0; i < 256; ++i)
			flameOdds[i] = burnOdds[i] = 0;

        setFlammable(Tile::wood->id, FLAME_HARD, BURN_MEDIUM);
        setFlammable(Tile::treeTrunk->id, FLAME_HARD, BURN_HARD);
        setFlammable(((Tile*)Tile::leaves)->id, FLAME_EASY, BURN_EASY);
        setFlammable(Tile::bookshelf->id, FLAME_EASY, BURN_MEDIUM);
        setFlammable(Tile::tnt->id, FLAME_MEDIUM, BURN_INSTANT);
        setFlammable(Tile::cloth->id, FLAME_EASY, BURN_EASY);

        if (Tile::birchTrunk) setFlammable(Tile::birchTrunk->id, FLAME_HARD, BURN_HARD);
        if (Tile::spruceTrunk) setFlammable(Tile::spruceTrunk->id, FLAME_HARD, BURN_HARD);
        if (Tile::birchPlanks) setFlammable(Tile::birchPlanks->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::sprucePlanks) setFlammable(Tile::sprucePlanks->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::spruceSlab) setFlammable(Tile::spruceSlab->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::spruceSlabHalf) setFlammable(Tile::spruceSlabHalf->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::birchSlab) setFlammable(Tile::birchSlab->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::birchSlabHalf) setFlammable(Tile::birchSlabHalf->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::stairs_spruce) setFlammable(Tile::stairs_spruce->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::stairs_birch) setFlammable(Tile::stairs_birch->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::fenceSpruce) setFlammable(Tile::fenceSpruce->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::fenceBirch) setFlammable(Tile::fenceBirch->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::fenceGateSpruce) setFlammable(Tile::fenceGateSpruce->id, FLAME_HARD, BURN_MEDIUM);
        if (Tile::fenceGateBirch) setFlammable(Tile::fenceGateBirch->id, FLAME_HARD, BURN_MEDIUM);

        setTicking(true); //@fire
    }

    AABB* getAABB(Level* level, int x, int y, int z) {
        return NULL;
    }

    bool blocksLight() {
        return false;
    }

    bool isSolidRender() {
        return false;
    }

    bool isCubeShaped() {
        return false;
    }

    int getRenderShape() {
        return Tile::SHAPE_FIRE;
    }

    int getResourceCount(Random* random) {
        return 0;
    }

    int getTickDelay() {
        return 10;
    }

    void tick(Level* level, int x, int y, int z, Random* random) {
		

        bool infiniBurn = level->getTile(x, y - 1, z) == Tile::netherrack->id;

        int age = level->getData(x, y, z);
        if (age < 15) {
            level->setData(x, y, z, age + 1);
            level->addToTickNextTick(x, y, z, id, getTickDelay());
        }
        if (!infiniBurn && !isValidFireLocation(level, x, y, z)) {
            if (!level->isSolidBlockingTile(x, y - 1, z) || age > 3) level->setTile(x, y, z, 0);
            return;
        }

        if (!infiniBurn && !canBurn(level, x, y - 1, z)) {
            if (age == 15 && random->nextInt(4) == 0) {
                level->setTile(x, y, z, 0);
                return;
            }
        }

        if (age % 2 == 0 && age > 2) {
            checkBurn(level, x + 1, y, z, 300, random);
            checkBurn(level, x - 1, y, z, 300, random);
            checkBurn(level, x, y - 1, z, 250, random);
            checkBurn(level, x, y + 1, z, 250, random);
            checkBurn(level, x, y, z - 1, 300, random);
            checkBurn(level, x, y, z + 1, 300, random);

            for (int xx = x - 1; xx <= x + 1; xx++) {
                for (int zz = z - 1; zz <= z + 1; zz++) {
                    for (int yy = y - 1; yy <= y + 4; yy++) {
                        if (xx == x && yy == y && zz == z) continue;

                        int rate = 100;
                        if (yy > y + 1) {
                            rate += ((yy - (y + 1)) * 100);
                        }

                        int odds = getFireOdds(level, xx, yy, zz);
                        if (odds > 0 && random->nextInt(rate) <= odds) {
                            level->setTile(xx, yy, zz, this->id);
                        }
                    }
                }
            }
        }
        if (age == 15) {
            checkBurn(level, x + 1, y, z, 1, random);
            checkBurn(level, x - 1, y, z, 1, random);
            checkBurn(level, x, y - 1, z, 1, random);
            checkBurn(level, x, y + 1, z, 1, random);
            checkBurn(level, x, y, z - 1, 1, random);
            checkBurn(level, x, y, z + 1, 1, random);
        }
    }

    bool mayPick() {
        return false;
    }

		    int getRenderLayer() {
        return Tile::RENDERLAYER_ALPHATEST;
    }

    bool canBurn(LevelSource* level, int x, int y, int z) {
        return flameOdds[level->getTile(x, y, z)] > 0;
    }

    int getFlammability(Level* level, int x, int y, int z, int odds) {

        int f = flameOdds[level->getTile(x, y, z)];
        if (f > odds) return f;
        return odds;
    }


    bool mayPlace(Level* level, int x, int y, int z, unsigned char face) {
        return level->isSolidBlockingTile(x, y - 1, z) || isValidFireLocation(level, x, y, z);
    }

    void neighborChanged(Level* level, int x, int y, int z, int type) {
        if (!level->isSolidBlockingTile(x, y - 1, z) && !isValidFireLocation(level, x, y, z)) {
            level->setTile(x, y, z, 0);
            return;
        }
    }

    void onPlace(Level* level, int x, int y, int z) {
        if (level->getTile(x, y - 1, z) == Tile::obsidian->id && Tile::classicPortal != NULL) {
            if (((ClassicPortalTile*)Tile::classicPortal)->trySpawnPortal(level, x, y, z))
                return;
        }

        if (!level->isSolidBlockingTile(x, y - 1, z) && !isValidFireLocation(level, x, y, z)) {
            level->setTile(x, y, z, 0);
            return;
        }
        level->addToTickNextTick(x, y, z, id, getTickDelay());
    }

    bool isFlammable(int tile) {
        return flameOdds[tile] > 0;
    }

    void ignite(Level* level, int x, int y, int z) {

        bool lit = false;
        if (!lit) lit = tryIgnite(level, x, y + 1, z);
        if (!lit) lit = tryIgnite(level, x - 1, y, z);
        if (!lit) lit = tryIgnite(level, x + 1, y, z);
        if (!lit) lit = tryIgnite(level, x, y, z - 1);
        if (!lit) lit = tryIgnite(level, x, y, z + 1);
        if (!lit) lit = tryIgnite(level, x, y - 1, z);
        if (!lit) {
            level->setTile(x, y, z, Tile::fire->id);
        }
    }

    void animateTick(Level* level, int x, int y, int z, Random* random) {

        if (random->nextInt(24) == 0) {
            level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, "fire.fire", 1 + random->nextFloat(), random->nextFloat() * 0.7f + 0.3f);
        }

        if (level->isSolidBlockingTile(x, y - 1, z) || Tile::fire->canBurn(level, x, y - 1, z)) {
            for (int i = 0; i < 3; i++) {
                float xx = x + random->nextFloat();
                float yy = y + random->nextFloat() * 0.5f + 0.5f;
                float zz = z + random->nextFloat();
                level->addParticle(PARTICLETYPE(largesmoke), xx, yy, zz, 0, 0, 0);
            }
        } else {
            if (Tile::fire->canBurn(level, x - 1, y, z)) {
                for (int i = 0; i < 2; i++) {
                    float xx = x + random->nextFloat() * 0.1f;
                    float yy = y + random->nextFloat();
                    float zz = z + random->nextFloat();
                    level->addParticle(PARTICLETYPE(largesmoke), xx, yy, zz, 0, 0, 0);
                }
            }
            if (Tile::fire->canBurn(level, x + 1, y, z)) {
                for (int i = 0; i < 2; i++) {
                    float xx = x + 1 - random->nextFloat() * 0.1f;
                    float yy = y + random->nextFloat();
                    float zz = z + random->nextFloat();
                    level->addParticle(PARTICLETYPE(largesmoke), xx, yy, zz, 0, 0, 0);
                }
            }
            if (Tile::fire->canBurn(level, x, y, z - 1)) {
                for (int i = 0; i < 2; i++) {
                    float xx = x + random->nextFloat();
                    float yy = y + random->nextFloat();
                    float zz = z + random->nextFloat() * 0.1f;
                    level->addParticle(PARTICLETYPE(largesmoke), xx, yy, zz, 0, 0, 0);
                }
            }
            if (Tile::fire->canBurn(level, x, y, z + 1)) {
                for (int i = 0; i < 2; i++) {
                    float xx = x + random->nextFloat();
                    float yy = y + random->nextFloat();
                    float zz = z + 1 - random->nextFloat() * 0.1f;
                    level->addParticle(PARTICLETYPE(largesmoke), xx, yy, zz, 0, 0, 0);
                }
            }
            if (Tile::fire->canBurn(level, x, y + 1, z)) {
                for (int i = 0; i < 2; i++) {
                    float xx = x + random->nextFloat();
                    float yy = y + 1 - random->nextFloat() * 0.1f;
                    float zz = z + random->nextFloat();
                    level->addParticle(PARTICLETYPE(largesmoke), xx, yy, zz, 0, 0, 0);
                }
            }
        }
    }

private:
    void setFlammable(int id, int flame, int burn) {
        flameOdds[id] = flame;
        burnOdds[id] = burn;
    }

    void checkBurn(Level* level, int x, int y, int z, int chance, Random* random) {

		int odds = burnOdds[level->getTile(x, y, z)];
        if (random->nextInt(chance) < odds) {
            bool wasTnt = level->getTile(x, y, z) == Tile::tnt->id;
            if (random->nextInt(2) == 0) {
                level->setTile(x, y, z, this->id);
            } else {
                level->setTile(x, y, z, 0);
            }
            if (wasTnt) {
                Tile::tnt->destroy(level, x, y, z, 0);
            }
        }
    }

    bool isValidFireLocation(Level* level, int x, int y, int z) {

        if (canBurn(level, x + 1, y, z)) return true;
        if (canBurn(level, x - 1, y, z)) return true;
        if (canBurn(level, x, y - 1, z)) return true;
        if (canBurn(level, x, y + 1, z)) return true;
        if (canBurn(level, x, y, z - 1)) return true;
        if (canBurn(level, x, y, z + 1)) return true;

        return false;
    }

    int getFireOdds(Level* level, int x, int y, int z) {

		int odds = 0;
        if (!level->isEmptyTile(x, y, z)) return 0;

        odds = getFlammability(level, x + 1, y, z, odds);
        odds = getFlammability(level, x - 1, y, z, odds);
        odds = getFlammability(level, x, y - 1, z, odds);
        odds = getFlammability(level, x, y + 1, z, odds);
        odds = getFlammability(level, x, y, z - 1, odds);
        odds = getFlammability(level, x, y, z + 1, odds);

        return odds;
    }

	bool tryIgnite(Level* level, int x, int y, int z) {

		int t = level->getTile(x, y, z);
        if (t == Tile::fire->id) return true;
        if (t == 0) {
            level->setTile(x, y, z, Tile::fire->id);
            return true;
        }
        return false;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__FireTile_H__*/
