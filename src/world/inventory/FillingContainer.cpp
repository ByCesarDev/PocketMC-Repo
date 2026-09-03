#include "FillingContainer.h"
#include <sstream>
#include <algorithm>
#include "../item/ItemInstance.h"
#include "../../nbt/CompoundTag.h"
#include "../../nbt/ListTag.h"
#include "../item/crafting/Recipe.h"
#include "../../util/Mth.h"
#include "../level/tile/Tile.h"
#include "../level/tile/StoneSlabTile.h"
#include "../entity/player/Inventory.h"
#include "../../platform/log.h"

static const int MAGIX_VAL = 255;

FillingContainer::FillingContainer( int totalSlots, int numLinkedSlots, int popTimeDuration, bool isCreative )
:	Container(ContainerType::INVENTORY),
	numTotalSlots(totalSlots),
	numLinkedSlots(numLinkedSlots),
	items(totalSlots, (ItemInstance*)NULL),
	_isCreative(isCreative)
{
	linkedSlots = new LinkedSlot[numLinkedSlots];
	for (int i = 0; i < numLinkedSlots; ++i) {
		linkedSlots[i].inventorySlot = i;
	}
}

FillingContainer::~FillingContainer()
{
	clearInventory();
	delete[] linkedSlots;
}

void FillingContainer::clearInventory()
{
	for (int i = 0; i < numLinkedSlots; ++i) {
		linkedSlots[i].inventorySlot = i;
	}

	for (unsigned int i = 0; i < items.size(); i++) {
		release(i);
	}

	items.resize(numTotalSlots);
}

bool FillingContainer::removeResource( int type )
{
	if (_isCreative) return true;

	int slot = getSlot(type);
	if (slot < 0) return false;
	if (--items[slot]->count <= 0) release(slot);

	return true;
}

bool FillingContainer::removeResource( const ItemInstance& item ) {
	return removeResource(item, false) == 0;
}

int FillingContainer::removeResource( const ItemInstance& item, bool requireExactAux )
{
	if (_isCreative) return 0;

	int count = item.count;
	while (count > 0) {
		// If any AUX value, remove any with that id
		int slot = -1;
		if (!requireExactAux && (Recipe::isAnyAuxValue(&item) || item.getAuxValue() == Recipe::ANY_AUX_VALUE))
			slot = getNonEmptySlot(item.id);
		else
			slot = getNonEmptySlot(item.id, item.getAuxValue());

		if (slot < 0)
            return count;

		ItemInstance* slotItem = items[slot];
		int toRemove = Mth::Min(count, slotItem->count);
		slotItem->count -= toRemove;
		count -= toRemove;

		if (slotItem->count <= 0)
			clearSlot(slot);
	}
	return 0;
}

bool FillingContainer::hasResource( int type ) const
{
	if (_isCreative) return true;

	int slot = getSlot(type);
	if (slot < 0) return false;

	return true;
}

void FillingContainer::swapSlots( int from, int to )
{
	ItemInstance* tmp = items[to];
	items[to] = items[from];
	items[from] = tmp;
}

bool FillingContainer::add( ItemInstance* item )
{
	if (!item || item->isNull()) return true;

	if (!item->isDamaged()) {
		int lastSize;
		do {
			lastSize = item->count;
			item->count = addResource(*item);
		} while (item->count > 0 && item->count < lastSize);
		return item->count < lastSize;
	}

	int slot = getFreeSlot();
	if (slot >= 0) {
		items[slot] = ItemInstance::clone(item);
		linkEmptySlot(slot);
		item->count = 0;
		return true;
	}

	return false;
}

ItemInstance FillingContainer::removeItem( int slot, int count )
{
	ItemInstance* item = getItem(slot);
	if (item) {
		if (count > item->count)
			count = item->count;
		item->count -= count;

		if (item->count <= 0)
			clearSlot(slot);
	}
	return ItemInstance();
}

void FillingContainer::setItem( int slot, ItemInstance* item )
{
	if (slot < 0 || slot >= numTotalSlots)
		return;

	if (items[slot]) {
		if (item && !item->isNull()) *items[slot] = *item;
		else { delete items[slot]; items[slot] = NULL; }
	} else {
		items[slot] = (item && !item->isNull()) ? new ItemInstance(*item) : NULL;
	}
}

ListTag* FillingContainer::save( ListTag* listTag )
{
	for (int i = 0; i < (int)items.size(); i++) {
		ItemInstance* item = items[i];
		if (item != NULL && !item->isNull()) {
			CompoundTag* tag = new CompoundTag();
			tag->putByte("Slot", (char) i);
            
			if (item->count < 0) item->count = 0;
			if (item->count > 255) item->count = 255;

            ItemInstance iitem(*item);
			iitem.save(tag);
			listTag->add(tag);
		}
	}
	return listTag;
}

void FillingContainer::load( ListTag* inventoryList )
{
	clearInventory();

	for (int i = inventoryList->size()-1; i >= 0; --i) {
		Tag* t = inventoryList->get(i);
		if (t->getId() != Tag::TAG_Compound) continue;

		CompoundTag* tag = (CompoundTag*) t;
        int slot = tag->getByte("Slot") & 0xff;
		if (slot < 0 || slot >= numTotalSlots) continue;

		ItemInstance* item = ItemInstance::fromTag(tag);
		if (item != NULL && !item->isNull()) {
			if (item->id == MAGIX_VAL) {
				delete item;
			} else {
				items[slot] = item;
			}
		}
	}

	for (int i = 0; i < numLinkedSlots; ++i) {
		linkedSlots[i].inventorySlot = i;
	}
}

int FillingContainer::getContainerSize() const
{
	return numTotalSlots;
}

ItemInstance* FillingContainer::getItem( int slot )
{
	if (slot < 0 || slot >= numTotalSlots || slot >= (int)items.size())
		return NULL;

	return items[slot];
}

std::string FillingContainer::getName() const
{
	return "Inventory";
}

int FillingContainer::getMaxStackSize() const
{
	return MAX_INVENTORY_STACK_SIZE;
}

void FillingContainer::dropSlot( int slot, bool onlyClearContainer, bool randomly/*=false*/ )
{
	if (slot < 0 || slot >= (int)items.size()) return;

	if (items[slot] && items[slot]->count) {
		if (!onlyClearContainer)
			doDrop(items[slot]->copy(), randomly);
		items[slot]->count = 0;
		release(slot);
	}
}

void FillingContainer::dropAll(bool onlyClearContainer)
{
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] != NULL) {
			if (!onlyClearContainer)
				doDrop(items[i], true);
			items[i] = NULL;
		}
	}
}

int FillingContainer::getSlot( int tileId, int data) const
{
	for (unsigned int i = 0; i < items.size(); i++) {
		if (items[i] != NULL
		 && items[i]->id == tileId
		 && items[i]->getAuxValue() == data)
			return i;
	}
	return -1;
}

int FillingContainer::getSlot( int tileId ) const
{
	for (unsigned int i = 0; i < items.size(); i++) {
		if (items[i] != NULL && items[i]->id == tileId)
			return i;
	}
	return -1;
}

bool FillingContainer::stillValid( Player* player )
{
	return true;
}

int FillingContainer::getNonEmptySlot( int tileId) const
{
	for (unsigned int i = 0; i < items.size(); i++) {
		if (items[i] != NULL && items[i]->id == tileId && items[i]->count > 0) return i;
	}
	return -1;
}

int FillingContainer::getNonEmptySlot( int tileId, int data) const
{
	for (unsigned int i = 0; i < items.size(); i++) {
		if (items[i] != NULL
			&& items[i]->id == tileId
			&& items[i]->getAuxValue() == data
			&& items[i]->count > 0)
			return i;
	}
	return -1;
}

int FillingContainer::getNumEmptySlots() {
	int numEmpty = 0;
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (!items[i] || items[i]->isNull())
			++numEmpty;
	}
	return numEmpty;
}

int FillingContainer::getNumLinkedSlots() {
	return numLinkedSlots;
}

int FillingContainer::getSlotWithRemainingSpace( const ItemInstance& item )
{
	for (unsigned int i = 0; i < items.size(); i++) {
		if (items[i] != NULL && items[i]->id == item.id
		 && items[i]->isStackable()
		 && items[i]->count < items[i]->getMaxStackSize()
		 && items[i]->count < getMaxStackSize()
		 && (!items[i]->isStackedByData() || items[i]->getAuxValue() == item.getAuxValue()))
		{
			return i;
		}
	}
	return -1;
}

int FillingContainer::addResource( const ItemInstance& itemInstance )
{
	int type = itemInstance.id;
	int count = itemInstance.count;

	if (itemInstance.getMaxStackSize() == 1) {
		int slot = getFreeSlot();
		if (slot < 0) return count;
		if (items[slot] == NULL) {
			items[slot] = ItemInstance::clone(&itemInstance);
			linkEmptySlot(slot);
		} else if (items[slot]->isNull()) {
			*items[slot] = itemInstance;
			linkEmptySlot(slot);
		}
		return 0;
	}

	int slot = getSlotWithRemainingSpace(itemInstance);
	if (slot < 0) slot = getFreeSlot();
	if (slot < 0) return count;
	if (items[slot] == NULL) {
		items[slot] = new ItemInstance(type, 0, itemInstance.getAuxValue());
	} else if (items[slot]->isNull()) {
		*items[slot] = ItemInstance(type, 0, itemInstance.getAuxValue());
	}

	linkEmptySlot(slot);

	int toAdd = count;

	if (toAdd > items[slot]->getMaxStackSize() - items[slot]->count) {
		toAdd = items[slot]->getMaxStackSize() - items[slot]->count;
	}

	if (toAdd > getMaxStackSize() - items[slot]->count) {
		toAdd = getMaxStackSize() - items[slot]->count;
	}

	if (toAdd == 0) return count;

	count -= toAdd;
	items[slot]->count += toAdd;

	return count;
}

int FillingContainer::getFreeSlot() const
{
	for (unsigned int i = 0; i < items.size(); i++) {
		if (items[i] == NULL || items[i]->isNull()) return i;
	}
	return -1;
}

void FillingContainer::release( int slot )
{
	if (slot >= 0 && slot < (int)items.size() && items[slot]) {
		delete items[slot];
		items[slot] = NULL;
	}
}

void FillingContainer::clearSlot( int slot )
{
	if (slot < 0 || slot >= numTotalSlots)
		return;

	release(slot);
}

int FillingContainer::addItem(ItemInstance* item) {
	for (unsigned int i = 0; i < items.size(); ++i)
		if (!items[i]) {
			items[i] = item;
			return i;
		}

	int newSize = items.size() + 1;
	if (_isCreative && newSize > numTotalSlots)
		numTotalSlots = newSize;
	if (newSize <= numTotalSlots) {
		items.push_back(item);
		return newSize-1;
	}
	delete item;
	return 0;
}

void FillingContainer::fixBackwardCompabilityItem( ItemInstance& item )
{
	if (item.id == Tile::stoneSlabHalf->id)
		item.setAuxValue(item.getAuxValue() & StoneSlabTile::TYPE_MASK);
}

void FillingContainer::replace( std::vector<ItemInstance> newItems, int maxCount /* = -1 */)
{
	clearInventory();
	maxCount = maxCount < 0 ? newItems.size() : Mth::Min(newItems.size(), maxCount);

	int base = numLinkedSlots;
	const int iMax = Mth::Min(getContainerSize() - base, maxCount);
	for (int i = 0; i < iMax; ++i) {
		replaceSlot(base + i, newItems[i].isNull()? NULL : &newItems[i]);
	}
}

void FillingContainer::replaceSlot( int slotId, ItemInstance* ins )
{
	if (slotId < 0 || slotId >= numTotalSlots) return;

	if (ins) {
		if (!items[slotId])
			items[slotId] = new ItemInstance();
		*items[slotId] = *ins;
	} else {
		release(slotId);
	}
}

FillingContainer::ItemList* FillingContainer::getSlotList( int& slot )
{
	ItemList* pile = &items;
	return pile;
}

bool FillingContainer::linkSlot(int selectionSlot, int inventorySlot, bool propagate) {
	if (selectionSlot < 0 || selectionSlot >= numLinkedSlots)
		return false;
	if (inventorySlot < 0 || inventorySlot >= numTotalSlots)
		return false;

	linkedSlots[selectionSlot].inventorySlot = inventorySlot;
	return true;
}

bool FillingContainer::linkEmptySlot( int inventorySlot )
{
	return false;
}

void FillingContainer::compressLinkedSlotList(int slot)
{
}

void FillingContainer::doDrop( ItemInstance* item, bool randomly )
{
	delete item;
}

ItemInstance* FillingContainer::getLinked( int slot )
{
	if (slot >= 0 && slot < (int)items.size()) {
		return items[slot];
	}
	return NULL;
}
