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
	for (int i = 0; i < MAX_SELECTION_SIZE; ++i) {
		linkedSlots[i] = LinkedSlot(i);
	}
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
