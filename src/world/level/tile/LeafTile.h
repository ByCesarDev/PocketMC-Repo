#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__LeafTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__LeafTile_H__

//package net.minecraft.world.level.tile;

#include "TransparentTile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../item/Item.h"
#include "../../item/ItemInstance.h"
#include "../FoliageColor.h"
#include "../biome/BiomeSource.h"

class Entity;

class LeafTile: public TransparentTile
{
	typedef TransparentTile super;

public:
	static const int LEAF_TYPE_MASK = 7;
	static const int REQUIRED_WOOD_RANGE = 4;

	//@attn @note:	PERSISTENT_LEAF_BIT and UPDATE_LEAF_BIT are reversed
	//				here, compared to desktop version
	static const int PERSISTENT_LEAF_BIT = 8; // player-placed
    static const int UPDATE_LEAF_BIT = 4;
    static const int NORMAL_LEAF = 0;
    static const int EVERGREEN_LEAF = 1;
    static const int BIRCH_LEAF = 2;
    static const int JUNGLE_LEAF = 3;
    static const int ACACIA_LEAF = 4;
    static const int DARK_OAK_LEAF = 5;

    int leafType;

	LeafTile(int id, int tex, int leafType = NORMAL_LEAF)
	:	super(id, tex, Material::leaves, false),
		leafType(leafType),
		oTex(tex),
		checkBuffer(NULL)
	{
        setTicking(true);
    }

	~LeafTile() {
		if (checkBuffer != NULL)
			delete[] checkBuffer;
	}

	int getRenderLayer() {
        return isSolidRender()? Tile::RENDERLAYER_OPAQUE : Tile::RENDERLAYER_ALPHATEST;
    }

	int getLeafType(int data = 0) const {
		if (this == (Tile*)Tile::spruceLeaves)  return EVERGREEN_LEAF;
		if (this == (Tile*)Tile::birchLeaves)   return BIRCH_LEAF;
		if (this == (Tile*)Tile::jungleLeaves)  return JUNGLE_LEAF;
		if (this == (Tile*)Tile::acaciaLeaves)  return ACACIA_LEAF;
		if (this == (Tile*)Tile::darkOakLeaves) return DARK_OAK_LEAF;
		if (leafType != NORMAL_LEAF)            return leafType;
		return (data & LEAF_TYPE_MASK);
	}

    int getColor(LevelSource* level, int x, int y, int z) {
        int data = getLeafType(level ? level->getData(x, y, z) : 0);
        if (data == EVERGREEN_LEAF) {
            return FoliageColor::getEvergreenColor();
        }
        if (data == BIRCH_LEAF) {
            return FoliageColor::getBirchColor();
        }
        if (data == ACACIA_LEAF) {
            return 0xaea42a;
        }
        if (data == DARK_OAK_LEAF) {
            return 0x3b5919;
        }
		if (!FoliageColor::useTint) {
			return FoliageColor::getDefaultColor();
		}
		if (!level || !level->getBiomeSource()) {
			return FoliageColor::getDefaultColor();
		}

        level->getBiomeSource()->getBiomeBlock(x, z, 1, 1);
        float temperature = level->getBiomeSource()->temperatures[0];
        float rainfall = level->getBiomeSource()->downfalls[0];

        return FoliageColor::get(temperature, rainfall);
    }

    void onRemove(Level* level, int x, int y, int z) {
        int r = 1;
        int r2 = r + 1;

        if (level->hasChunksAt(x - r2, y - r2, z - r2, x + r2, y + r2, z + r2)) {
            for (int xo = -r; xo <= r; xo++)
            for (int yo = -r; yo <= r; yo++)
            for (int zo = -r; zo <= r; zo++) {
                int t = level->getTile(x + xo, y + yo, z + zo);
                if (t == Tile::leaves->id) {
                    int currentData = level->getData(x + xo, y + yo, z + zo);
                    level->setDataNoUpdate(x + xo, y + yo, z + zo, currentData | UPDATE_LEAF_BIT);
                }
            }
        }
    }

    int* checkBuffer; //@todo Rewrite this?

    void tick(Level* level, int x, int y, int z, Random* random) {
        if (level->isClientSide) return;

        int currentData = level->getData(x, y, z);
        if ((currentData & UPDATE_LEAF_BIT) != 0 && (currentData & PERSISTENT_LEAF_BIT) == 0) {
            const int r = LeafTile::REQUIRED_WOOD_RANGE;
            int r2 = r + 1;

            const int W = 32;
            const int WW = W * W;
            const int WO = W / 2;
            if (!checkBuffer) {
                checkBuffer = new int[W * W * W];
            }

            if (level->hasChunksAt(x - r2, y - r2, z - r2, x + r2, y + r2, z + r2)) {
                for (int xo = -r; xo <= r; xo++)
                for (int yo = -r; yo <= r; yo++)
                for (int zo = -r; zo <= r; zo++) {
                    int t = level->getTile(x + xo, y + yo, z + zo);
                    if (t == Tile::treeTrunk->id || t == Tile::birchTrunk->id || t == Tile::spruceTrunk->id) {
                        checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO)] = 0;
                    } else if (t == Tile::leaves->id) {
                        checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO)] = -2;
                    } else {
                        checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO)] = -1;
                    }
                }
                for (int i = 1; i <= LeafTile::REQUIRED_WOOD_RANGE; i++) {
                    for (int xo = -r; xo <= r; xo++)
                    for (int yo = -r; yo <= r; yo++)
                    for (int zo = -r; zo <= r; zo++) {
                        if (checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO)] == i - 1) {
                            if (checkBuffer[(xo + WO - 1) * WW + (yo + WO) * W + (zo + WO)] == -2) {
                                checkBuffer[(xo + WO - 1) * WW + (yo + WO) * W + (zo + WO)] = i;
                            }
                            if (checkBuffer[(xo + WO + 1) * WW + (yo + WO) * W + (zo + WO)] == -2) {
                                checkBuffer[(xo + WO + 1) * WW + (yo + WO) * W + (zo + WO)] = i;
                            }
                            if (checkBuffer[(xo + WO) * WW + (yo + WO - 1) * W + (zo + WO)] == -2) {
                                checkBuffer[(xo + WO) * WW + (yo + WO - 1) * W + (zo + WO)] = i;
                            }
                            if (checkBuffer[(xo + WO) * WW + (yo + WO + 1) * W + (zo + WO)] == -2) {
                                checkBuffer[(xo + WO) * WW + (yo + WO + 1) * W + (zo + WO)] = i;
                            }
                            if (checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO - 1)] == -2) {
                                checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO - 1)] = i;
                            }
                            if (checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO + 1)] == -2) {
                                checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO + 1)] = i;
                            }
                        }
                    }
                }
            }

            int mid = checkBuffer[(WO) * WW + (WO) * W + (WO)];
            if (mid >= 0) {
                level->setDataNoUpdate(x, y, z, currentData & ~UPDATE_LEAF_BIT);
            } else {
                die(level, x, y, z);
            }
        }
    }

	void playerDestroy(Level* level, Player* player, int x, int y, int z, int data) {
		if (!level->isClientSide) {
			ItemInstance* item = player->inventory->getSelected();
			if (item && item->id == ((Item*)Item::shears)->id) {
				// drop this specific leaf block
				popResource(level, x, y, z, ItemInstance(this->id, 1, 0));
				return;
			}
		}
		super::playerDestroy(level, player, x, y, z, data);
	}

    int getResourceCount(Random* random) {
        return random->nextInt(20) == 0 ? 1 : 0;
    }

    int getResource(int data, Random* random) {
        int type = getLeafType(data);
        if (type == EVERGREEN_LEAF && Tile::spruceSapling) return Tile::spruceSapling->id;
        if (type == BIRCH_LEAF && Tile::birchSapling)     return Tile::birchSapling->id;
        if (type == JUNGLE_LEAF && Tile::jungleSapling)   return Tile::jungleSapling->id;
        if (type == ACACIA_LEAF && Tile::acaciaSapling)   return Tile::acaciaSapling->id;
        if (type == DARK_OAK_LEAF && Tile::darkOakSapling) return Tile::darkOakSapling->id;
        return Tile::sapling ? Tile::sapling->id : 6;
    }

	void spawnResources(Level* level, int x, int y, int z, int data, float odds) {
		if (!level->isClientSide) {
			int chance = 20;
			if (level->random.nextInt(chance) == 0) {
				int type = getResource(data, &level->random);
				popResource(level, x, y, z, ItemInstance(type, 1, 0));
			}

			if (getLeafType(data) == NORMAL_LEAF && level->random.nextInt(200) == 0) {
				popResource(level, x, y, z, ItemInstance(Item::apple, 1, 0));
			}
		}
	}

    bool isSolidRender() {
        return !allowSame;
    }

    int getTexture(int face, int data) {
		int type = getLeafType(data);
		if (this == Tile::leaves_carried) {
			if (type == EVERGREEN_LEAF) return 49 | Tile::TEXTURE_ALT_FLAG;
			if (type == BIRCH_LEAF)     return 50 | Tile::TEXTURE_ALT_FLAG;
			if (type == JUNGLE_LEAF)    return 51 | Tile::TEXTURE_ALT_FLAG;
			if (type == ACACIA_LEAF)    return 52 | Tile::TEXTURE_ALT_FLAG;
			if (type == DARK_OAK_LEAF)  return 53 | Tile::TEXTURE_ALT_FLAG;
			return 235; // Oak carried
		}
		bool opaque = isSolidRender();
		if (type == EVERGREEN_LEAF) return opaque ? 133 : 132;
		if (type == BIRCH_LEAF)     return (opaque ? 42 : 41) | Tile::TEXTURE_ALT_FLAG;
		if (type == JUNGLE_LEAF)    return (opaque ? 44 : 43) | Tile::TEXTURE_ALT_FLAG;
		if (type == ACACIA_LEAF)    return (opaque ? 46 : 45) | Tile::TEXTURE_ALT_FLAG;
		if (type == DARK_OAK_LEAF)  return (opaque ? 48 : 47) | Tile::TEXTURE_ALT_FLAG;
		return opaque ? 53 : 52; // Oak
    }

    void setFancy(bool fancyGraphics) {
        allowSame = fancyGraphics;
        tex = oTex + (fancyGraphics ? 0 : 1);
    }
protected:
	int getSpawnResourcesAuxValue(int data) {
		return data & LEAF_TYPE_MASK;
	}
private:
    void die(Level* level, int x, int y, int z) {
        spawnResources(level, x, y, z, level->getData(x, y, z) & LEAF_TYPE_MASK, 0);
        level->setTile(x, y, z, 0);
    }

	int oTex;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__LeafTile_H__*/
