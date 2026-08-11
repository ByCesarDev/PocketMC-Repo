#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Inventory.h"
#include "../../level/material/Material.h"
#include "../../level/tile/QuartzBlockTile.h"
#include "../../level/tile/TreeTile.h"
#include "../../level/tile/StoneSlabTile.h"
#include "../../item/DyePowderItem.h"
#include "../../item/crafting/Recipe.h"
#include "../../item/CoalItem.h"
#include "../../level/tile/SandStoneTile.h"

Inventory::Inventory( Player* player, bool creativeMode )
:   super(	36 + Inventory::MAX_SELECTION_SIZE,
			MAX_SELECTION_SIZE,
			ContainerType::INVENTORY,
			creativeMode),
	BaseContainerMenu(ContainerType::INVENTORY),
	player(player),
	selected(0)
{
	setupDefault();
	compressLinkedSlotList(0);
}

Inventory::~Inventory() {
}

ItemInstance* Inventory::getSelected() {
	return getLinked(selected);
}

void Inventory::selectSlot( int slot ) {
	if (slot < MAX_SELECTION_SIZE && slot >= 0)
		selected = slot;
}

bool Inventory::moveToSelectedSlot( int inventorySlot, bool propagate ) {
	return linkSlot(selected, inventorySlot, propagate);
}

int Inventory::getSelectionSize() {
	return MAX_SELECTION_SIZE;
}

void Inventory::setupDefault() {
	clearInventory();
	int Sel[MAX_SELECTION_SIZE] = {0};

#ifdef DEMO_MODE
	if (_isCreative) {
		Sel[0] = addItem(new ItemInstance(Item::shovel_stone));
		addItem(new ItemInstance(Item::pickAxe_stone));
		addItem(new ItemInstance(Item::hatchet_stone));
		addItem(new ItemInstance((Item*)Item::shears));
		addItem(new ItemInstance(Tile::ladder));
		Sel[3] = addItem(new ItemInstance(Tile::torch));
		addItem(new ItemInstance(Item::door_wood));

		Sel[4] = addItem(new ItemInstance(Tile::stoneBrick));
		Sel[5] = addItem(new ItemInstance(Tile::wood));
		Sel[2] = addItem(new ItemInstance(Tile::redBrick));
		Sel[1] = addItem(new ItemInstance(Tile::dirt));
		addItem(new ItemInstance(Tile::sandStone));
		addItem(new ItemInstance(Tile::gravel));
		addItem(new ItemInstance(Tile::rock));
		addItem(new ItemInstance(Tile::sand));
		//addItem(new ItemInstance(Tile::clay));

		addItem(new ItemInstance(Tile::cloth, 1, 15));
		addItem(new ItemInstance(Tile::cloth, 1, 14));
		addItem(new ItemInstance(Tile::cloth, 1, 13));
		Sel[7] = addItem(new ItemInstance(Tile::cloth, 1, 12));
		addItem(new ItemInstance(Tile::cloth, 1, 11));
		addItem(new ItemInstance(Tile::cloth, 1, 10));
		Sel[8] = addItem(new ItemInstance(Tile::cloth, 1, 9));
		addItem(new ItemInstance(Tile::cloth, 1, 8));
		Sel[6] = addItem(new ItemInstance(Tile::glass));
		addItem(new ItemInstance(Tile::thinGlass));
		addItem(new ItemInstance(Tile::stairs_stone));
        addItem(new ItemInstance(Tile::bookshelf));
        addItem(new ItemInstance(Tile::workBench));
		addItem(new ItemInstance(Tile::chest));
		addItem(new ItemInstance(Tile::furnace));

		addItem(new ItemInstance(((Tile*)Tile::flower)));
		addItem(new ItemInstance(Tile::cactus));

		//
		// Those below are inactive due to demo
		//
		addItem(new ItemInstance(Item::sword_stone));
		addItem(new ItemInstance(Tile::treeTrunk, 1, 0));
		addItem(new ItemInstance(Tile::treeTrunk, 1, 1));
		addItem(new ItemInstance(Tile::treeTrunk, 1, 2));
		addItem(new ItemInstance(Tile::fence));
		addItem(new ItemInstance(Tile::fenceGate));
		addItem(new ItemInstance(Item::reeds));
		addItem(new ItemInstance(((Tile*)Tile::rose)));
		addItem(new ItemInstance(((Tile*)Tile::mushroom2)));
		addItem(new ItemInstance(((Tile*)Tile::mushroom1)));
		addItem(new ItemInstance(Tile::cloth, 1, 7));
		addItem(new ItemInstance(Tile::cloth, 1, 6));
		addItem(new ItemInstance(Tile::cloth, 1, 5));
		addItem(new ItemInstance(Tile::cloth, 1, 4));
		addItem(new ItemInstance(Tile::cloth, 1, 3));
		addItem(new ItemInstance(Tile::stairs_wood));
		addItem(new ItemInstance(Tile::goldBlock));
		addItem(new ItemInstance(Tile::ironBlock));
		addItem(new ItemInstance(Tile::emeraldBlock));
		addItem(new ItemInstance(Tile::lapisBlock));
		addItem(new ItemInstance(Tile::obsidian));
		addItem(new ItemInstance((Tile*)Tile::leaves, 1, 0));
		addItem(new ItemInstance((Tile*)Tile::leaves, 1, 1));
		addItem(new ItemInstance((Tile*)Tile::leaves, 1, 2));
		addItem(new ItemInstance(Tile::stoneSlabHalf));
	} else {
#if defined(WIN32)
		// Survival
		addItem(new ItemInstance((Item*)Item::shears));
		addItem(new ItemInstance(Tile::redBrick));
		addItem(new ItemInstance(Tile::glass));
#endif
	}
#else
	if (_isCreative) {
		// Populate all Tiles and variations
		for (int i = 1; i < Tile::NUM_BLOCK_TYPES; ++i) {
			Tile* t = Tile::tiles[i];
			if (t) {
				if (t == Tile::cloth) {
					for (int m = 0; m < 16; ++m) addItem(new ItemInstance(t, 1, m));
				} else if (t == Tile::treeTrunk || t == Tile::sapling || t == Tile::leaves || t == Tile::sandStone || t == Tile::quartzBlock) {
					for (int m = 0; m < 3; ++m) addItem(new ItemInstance(t, 1, m));
				} else if (t == Tile::stoneSlabHalf) {
					for (int m = 0; m < 6; ++m) addItem(new ItemInstance(t, 1, m));
				} else if (t == Tile::stoneBrickSmooth) {
					for (int m = 0; m < 3; ++m) addItem(new ItemInstance(t, 1, m));
				} else {
					addItem(new ItemInstance(t));
				}
			}
		}

		// Populate all Items and variations
		for (int i = 256; i < Item::MAX_ITEMS; ++i) {
			Item* it = Item::items[i];
			if (it) {
				if (it == Item::dye_powder) {
					for (int m = 0; m < 16; ++m) addItem(new ItemInstance(it, 1, m));
				} else {
					addItem(new ItemInstance(it));
				}
			}
		}

		// Setup default hotbar selections
		Sel[0] = 0;
		Sel[1] = 1;
		Sel[2] = 2;
		Sel[3] = 3;
		Sel[4] = 4;
		Sel[5] = 5;
		Sel[6] = 6;
		Sel[7] = 7;
		Sel[8] = 8;
	} else {
#if defined(WIN32)
		// Survival
		// addItem(new ItemInstance(Item::ironIngot, 64));
		// addItem(new ItemInstance(Item::ironIngot, 34));
		// addItem(new ItemInstance(Tile::stonecutterBench));
		// addItem(new ItemInstance(Tile::workBench));
		// addItem(new ItemInstance(Tile::furnace));
		// addItem(new ItemInstance(Tile::wood, 54));
		// addItem(new ItemInstance(Item::stick, 14));
		// addItem(new ItemInstance(Item::coal, 31));
		// addItem(new ItemInstance(Tile::sand, 6));
		// addItem(new ItemInstance(Item::dye_powder, 23, DyePowderItem::PURPLE));
#endif
	}
#endif

	for (unsigned int i = 0; i < items.size(); ++i) {
		ItemInstance* item = items[i];

		if (i < MAX_SELECTION_SIZE) {
			if (item)
				LOGE("Error: Should not have items on slot %i\n", i);

			items[i] = NULL;
			continue;
		}

		if (item && _isCreative)
			item->count = 5;
	}

	for (int i = 0; i < MAX_SELECTION_SIZE; ++i) {
		linkedSlots[i] = LinkedSlot(Sel[i]);
	}

	//LOGI("Inventory has %d items\n", (int)items.size());
}

void Inventory::clearInventoryWithDefault()
{
	clearInventory();
	setupDefault();
}

int Inventory::getAttackDamage( Entity* entity )
{
	ItemInstance* item = getSelected();
	if (item != NULL) return item->getAttackDamage(entity);
	return 1;
}

bool Inventory::canDestroy( Tile* tile )
{
	if (tile->material->isAlwaysDestroyable()) return true;

	ItemInstance* item = getSelected();
	if (item != NULL) return item->canDestroySpecial(tile);
	return false;
}

float Inventory::getDestroySpeed( Tile* tile )
{
	ItemInstance* item = getSelected();
	if (item && item->id >= 256) {
		return Item::items[item->id]->getDestroySpeed(NULL, tile);
	}
	return 1.0f;
}

bool Inventory::moveToSelectionSlot( int selectionSlot, int inventorySlot, bool propagate ) {
	return linkSlot(selectionSlot, inventorySlot, propagate);
}

bool Inventory::moveToEmptySelectionSlot( int inventorySlot ) {
	return linkEmptySlot(inventorySlot);
}

void Inventory::doDrop( ItemInstance* item, bool randomly )
{
	player->drop(item, randomly);
}

bool Inventory::stillValid(Player* player) {
	if (this->player->removed) return false;
	if (player->distanceToSqr(this->player) > 8 * 8) return false;
	return true;
}

bool Inventory::add( ItemInstance* item ){
	if (_isCreative || player->hasFakeInventory)
		return true;

	return super::add(item);
}

bool Inventory::removeItem( const ItemInstance* samePtr ) {
	for (int i = MAX_SELECTION_SIZE; i < (int)items.size(); ++i) {
		if (items[i] == samePtr) {
			clearSlot(i);
			return true;
		}
	}
	return false;
}

int Inventory::removeResource(ItemInstance& item, bool isAnyAuxValue) {
    int removedCount = 0;
    int toRemove = item.count;

    if (isAnyAuxValue) {
        for (int i = 0; i < getContainerSize() && toRemove > 0; ++i) {
            ItemInstance* current = getItem(i);
            if (current && current->id == item.id) {
                int canRemove = std::min(toRemove, current->count);
                current->count -= canRemove;
                toRemove -= canRemove;
                removedCount += canRemove;
                if (current->count == 0) {
                    setItem(i, nullptr);
                }
            }
        }
    } else {
        for (int i = 0; i < getContainerSize() && toRemove > 0; ++i) {
            ItemInstance* current = getItem(i);
            if (current && current->id == item.id && current->getAuxValue() == item.getAuxValue()) {
                int canRemove = std::min(toRemove, current->count);
                current->count -= canRemove;
                toRemove -= canRemove;
                removedCount += canRemove;
                if (current->count == 0) {
                    setItem(i, nullptr);
                }
            }
        }
        if (toRemove > 0) {
            for (int i = 0; i < getContainerSize() && toRemove > 0; ++i) {
                ItemInstance* current = getItem(i);
                if (current && current->id == item.id && current->getAuxValue() == Recipe::ANY_AUX_VALUE) {
                    int canRemove = std::min(toRemove, current->count);
                    current->count -= canRemove;
                    toRemove -= canRemove;
                    removedCount += canRemove;
                    if (current->count == 0) {
                        setItem(i, nullptr);
                    }
                }
            }
        }
    }
    
    return removedCount;
}

BaseContainerMenu::ItemList Inventory::getItems() {
    BaseContainerMenu::ItemList result;
    result.reserve(getContainerSize());
    for (int i = 0; i < getContainerSize(); ++i) {
        ItemInstance* item = getItem(i);
        if (item) {
            result.push_back(*item);
        } else {
            result.push_back(ItemInstance());
        }
    }
    return result;
}

void Inventory::setSlot(int slot, ItemInstance* item) {
    setItem(slot, item);
}

bool Inventory::tileEntityDestroyedIsInvalid(int) {
    return false;
}
