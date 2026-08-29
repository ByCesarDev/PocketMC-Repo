#include "TouchIngameBlockSelectionScreen.h"
#include "../crafting/WorkbenchScreen.h"
#include "../../Screen.h"
#include "../../components/ImageButton.h"
#include "../../components/InventoryPane.h"
#include "../../../gamemode/GameMode.h"
#include "../../../renderer/TileRenderer.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/gles.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/Textures.h"
#include "../../../Minecraft.h"
#include "../../../sound/SoundEngine.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../platform/input/Mouse.h"
#include "../../../../util/Mth.h"
#include "../../../../world/item/ItemInstance.h"
#include "../../../../world/entity/player/Player.h"
#include "../../../../locale/I18n.h"
#include "../../../../world/item/crafting/Recipe.h"
#include "../../../player/input/touchscreen/TouchAreaModel.h"
#include "../ArmorScreen.h"
#include "../../../renderer/GameRenderer.h"

#include "../../components/NinePatch.h"
#include "../../../../world/item/ItemCategory.h"

namespace Touch {

#if defined(__APPLE__)
    static const std::string demoVersionString("Not available in the Lite version");
#else
    static const std::string demoVersionString("Not available in the demo version");
#endif

#ifdef __APPLE__
    static const float BorderPixels = 4;
    #ifdef DEMO_MODE
        static const float BlockPixels = 22;
    #else
        static const float BlockPixels = 22;
    #endif
#else
    static const float BorderPixels = 4;
    static const float BlockPixels = 24;
#endif

static const int ItemSize = (int)(BlockPixels + 2*BorderPixels);

static const int Bx = 10; // Border Frame width
static const int By = 6; // Border Frame height

class CategoryButton: public ImageButton {
	typedef ImageButton super;
public:
	CategoryButton(int id, const ImageButton* const* selectedPtr, NinePatchLayer* stateNormal, NinePatchLayer* statePressed)
	:	super(id, ""),
		selectedPtr(selectedPtr),
		stateNormal(stateNormal),
		statePressed(statePressed)
	{}

	void renderBg(Minecraft* minecraft, int xm, int ym) override {
		bool hovered = active && (minecraft->useTouchscreen()?
			(_currentlyDown && xm >= x && ym >= y && xm < x + width && ym < y + height) : isInside(xm, ym));

		if (hovered || *selectedPtr == this)
			statePressed->draw(Tesselator::instance, (float)x, (float)y);
		else
			stateNormal->draw(Tesselator::instance, (float)x, (float)y);
	}
	bool isSecondImage(bool hovered) override { return false; }

private:
	const ImageButton* const* selectedPtr;
	NinePatchLayer* stateNormal;
	NinePatchLayer* statePressed;
};

//
// Block selection screen
//
IngameBlockSelectionScreen::IngameBlockSelectionScreen() 
:	selectedItem(0),
	_blockList(NULL),
	_pendingClose(false),
	bArmor  (4, I18n::get("gui.armor")),
	bDone   (3, I18n::get("gui.done")),
	//bDone   (3, "Done"),
	bMenu   (2, I18n::get("gui.menu")),
	bCraft  (1, I18n::get("gui.craft")),
	bHeader (0, I18n::get("gui.selectBlocks")),
	currentCategory(0),
	numCategories(4),
	selectedCategoryButton(NULL),
	guiBackground(NULL),
	guiSlotCategory(NULL),
	guiSlotCategorySelected(NULL),
	guiPaneFrame(NULL)
{
	for (int i = 0; i < numCategories; ++i) {
		categoryBitmasks.push_back(1 << i);
		categoryIcons.push_back(i);
	}
}

IngameBlockSelectionScreen::~IngameBlockSelectionScreen()
{
	delete _blockList;
	for (unsigned int i = 0; i < _categoryButtons.size(); ++i) {
		delete _categoryButtons[i];
	}
	delete guiBackground;
	delete guiSlotCategory;
	delete guiSlotCategorySelected;
	delete guiPaneFrame;

	for (size_t i = 0; i < creativeItemsCache.size(); ++i) {
		delete creativeItemsCache[i];
	}
	creativeItemsCache.clear();
}

void IngameBlockSelectionScreen::updateCreativeItems() {
	for (size_t i = 0; i < creativeItemsCache.size(); ++i) {
		delete creativeItemsCache[i];
	}
	creativeItemsCache.clear();

	if (minecraft->isCreativeMode()) {
		int targetMask = categoryBitmasks[currentCategory];

		for (int i = 0; i < 256; ++i) {
			if (Tile::tiles[i] != NULL) {
				Item* it = Item::items[i];
				int cat = it ? it->category : 1;
				if (cat <= 0) cat = 8;
				bool match = (targetMask == 8) ? (cat == 8 || cat >= 16) : (cat == targetMask);
				if (match) {
					creativeItemsCache.push_back(new ItemInstance(Tile::tiles[i], 1));
				}
			}
		}
		for (int i = 256; i < 512; ++i) {
			if (Item::items[i] != NULL) {
				Item* it = Item::items[i];
				int cat = it->category;
				if (cat <= 0) cat = 8;
				bool match = (targetMask == 8) ? (cat == 8 || cat >= 16) : (cat == targetMask);
				if (match) {
					creativeItemsCache.push_back(new ItemInstance(Item::items[i], 1));
				}
			}
		}
	}
}

void IngameBlockSelectionScreen::init()
{
	delete _blockList;
	_blockList = NULL;
	for (unsigned int i = 0; i < _categoryButtons.size(); ++i) {
		delete _categoryButtons[i];
	}
	_categoryButtons.clear();
	buttons.clear();

	delete guiBackground; guiBackground = NULL;
	delete guiSlotCategory; guiSlotCategory = NULL;
	delete guiSlotCategorySelected; guiSlotCategorySelected = NULL;
	delete guiPaneFrame; guiPaneFrame = NULL;

	Inventory* inventory = minecraft->player->inventory;

	if (minecraft->isCreativeMode()) {
		NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");
		guiBackground   = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
		guiSlotCategory = builder.createSymmetrical(IntRectangle(8, 32, 8, 8), 2, 2);
		guiSlotCategorySelected = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 2, 2);
		guiPaneFrame    = builder.createSymmetrical(IntRectangle(0, 20, 8, 8), 1, 2)->setExcluded(1 << 4);

		for (int i = 0; i < numCategories; ++i) {
			ImageButton* button = new CategoryButton(100 + i, (const ImageButton**)&selectedCategoryButton, guiSlotCategory, guiSlotCategorySelected);
			_categoryButtons.push_back( button );
			buttons.push_back( button );
		}
		selectedCategoryButton = _categoryButtons[currentCategory];
	}

	int categoryWidth = 0;
	if (minecraft->isCreativeMode()) {
		int categoriesHeight = height - (24 + By) - By;
		categoryWidth = categoriesHeight / 4;
	}

	int paneLeft = categoryWidth + (minecraft->isCreativeMode() ? (int)BorderPixels * 2 : Bx);
	int maxWidth = width - paneLeft - Bx;
	InventoryColumns = maxWidth / ItemSize;
	const int realWidth = InventoryColumns * ItemSize;
	const int realBx = paneLeft + (maxWidth - realWidth) / 2;

	IntRectangle rect(realBx,
#ifdef __APPLE__
		24 + By - ((width==240)?1:0), realWidth, ((width==240)?1:0) + height-By-By-20-24);
#else
		24 + By, realWidth, height-By-By-20-24);
#endif

	updateCreativeItems();
	int countItems = getItems(NULL).size();

	_blockList = new InventoryPane(this, minecraft, rect, width, BorderPixels, countItems, ItemSize, (int)BorderPixels);
	_blockList->fillMarginX = realBx;

	InventorySize = countItems;
	InventoryRows = 1 + (InventorySize-1) / InventoryColumns;

	ImageDef def;
	def.name = "gui/spritesheet.png";
	def.x = 0;
	def.y = 1;
	def.width = def.height = 18;
	def.setSrc(IntRectangle(60, 0, 18, 18));
	bDone.setImageDef(def, true);
	bDone.width = bDone.height = 19;
	bDone.scaleWhenPressed = false;

	buttons.push_back(&bHeader);
	buttons.push_back(&bDone);
	if (!minecraft->isCreativeMode()) {
		buttons.push_back(&bCraft);
		buttons.push_back(&bArmor);
	}
}

void IngameBlockSelectionScreen::setupPositions() {
	bHeader.y = bDone.y = bCraft.y = 0;
	bDone.x   = width -  bDone.width;
	bCraft.x  = 0;
	bCraft.width = bArmor.width = 48;
	bArmor.x = bCraft.width;

	if (minecraft->isCreativeMode()) {
		bHeader.x = 0;
		bHeader.width = width;
		bHeader.xText = width/2; // Center of the screen

		int categoriesHeight = height - (24 + By) - By;
		int buttonHeight = categoriesHeight / 4;

		int paneLeft = buttonHeight + (int)BorderPixels * 2;
		int maxWidth = width - paneLeft - Bx;
		int invCols = maxWidth / ItemSize;
		int realWidth = invCols * ItemSize;
		int realBx = paneLeft + (maxWidth - realWidth) / 2;

		for (unsigned int c = 0; c < _categoryButtons.size(); ++c) {
			ImageButton* button = _categoryButtons[c];
			button->x = realBx - buttonHeight - (int)BorderPixels;
			button->y = 24 + (int)By + c * (1 + buttonHeight);
			button->width = buttonHeight;
			button->height = buttonHeight;

			int icon = categoryIcons[c];
			ImageDef def;
			def.x = 0;
			def.width = def.height = (float)buttonHeight;
			def.name = "gui/spritesheet.png";
			def.setSrc(IntRectangle(32 * (icon/2), 64 + (icon&1) * 32, 32, 32));
			button->setImageDef(def, false);
		}

		guiBackground->setSize((float)width, (float)height);
		guiSlotCategory->setSize((float)buttonHeight, (float)buttonHeight);
		guiSlotCategorySelected->setSize((float)buttonHeight, (float)buttonHeight);
		if (guiPaneFrame) {
			guiPaneFrame->setSize((float)_blockList->rect.w + 2, (float)_blockList->rect.h + 2);
		}
	} else {
		bHeader.x = bCraft.width + bArmor.width;
		bHeader.width = width - bCraft.width - bArmor.width;
		bHeader.xText = bHeader.x + (bHeader.width - bDone.width) /2;
	}

	clippingArea.x = 0;
	clippingArea.w = minecraft->width;
	clippingArea.y = 0;
	clippingArea.h = (int)(Gui::GuiScale * 24);
}

void IngameBlockSelectionScreen::removed()
{
	minecraft->gui.inventoryUpdated();
}

int IngameBlockSelectionScreen::getSlotPosX(int slotX) {
	return width / 2 - InventoryColumns * 10 + slotX * 20 + 2;
}

int IngameBlockSelectionScreen::getSlotPosY(int slotY) {
	return height - 16 - 3 - 22 * 2 - 22 * slotY;
}

int IngameBlockSelectionScreen::getSlotHeight() {
	return 22;
}

void IngameBlockSelectionScreen::mouseClicked(int x, int y, int buttonNum) {
	_pendingClose = _blockList->_clickArea->isInside((float)x, (float)y);
	if (!_pendingClose)
		super::mouseClicked(x, y, buttonNum);
}

void IngameBlockSelectionScreen::mouseReleased(int x, int y, int buttonNum) {
	if (_pendingClose && _blockList->_clickArea->isInside((float)x, (float)y))
		minecraft->setScreen(NULL);
	else
		super::mouseReleased(x, y, buttonNum);
}

void IngameBlockSelectionScreen::mouseWheel(int dx, int dy, int xm, int ym)
{
	if (dy == 0) return;
	if (_blockList) {
		float amount = -dy * getSlotHeight();
		_blockList->scrollBy(0, amount);
	}
	int cols = InventoryColumns;
	int maxIndex = InventorySize - 1;
	int idx = selectedItem;
	if (dy > 0) {
		if (idx >= cols) idx -= cols;
	} else {
		if (idx + cols <= maxIndex) idx += cols;
	}
	selectedItem = idx;
}

bool IngameBlockSelectionScreen::addItem(const InventoryPane* pane, int itemId)
{
	Inventory* inventory = minecraft->player->inventory;
	auto items = getItems(pane);
	if (itemId < 0 || itemId >= (int)items.size() || items[itemId] == NULL)
		return false;

	const ItemInstance* selected = items[itemId];

	if (minecraft->isCreativeMode()) {
		int targetSlot = inventory->selected;
		ItemInstance* cur = inventory->getItem(targetSlot);
		if (cur && !cur->isNull()) {
			for (int i = 0; i < Inventory::MAX_SELECTION_SIZE; ++i) {
				ItemInstance* slotItem = inventory->getItem(i);
				if (!slotItem || slotItem->isNull()) {
					targetSlot = i;
					break;
				}
			}
		}
		int maxStack = 64;
		if (Item::items[selected->id]) maxStack = Item::items[selected->id]->getMaxStackSize();
		if (maxStack <= 0) maxStack = 64;

		inventory->setItem(targetSlot, new ItemInstance(selected->id, maxStack, selected->getAuxValue()));
		inventory->selectSlot(targetSlot);
	} else {
		int realInventoryIndex = itemId + Inventory::MAX_SELECTION_SIZE;
		if (realInventoryIndex < inventory->getContainerSize() && inventory->getItem(realInventoryIndex)) {
			inventory->moveToSelectionSlot(0, realInventoryIndex, true);
			inventory->selectSlot(0);
		}
	}

#ifdef __APPLE__
	minecraft->soundEngine->playUI("random.pop", 0.3f, 0.3f);
#else
	minecraft->soundEngine->playUI("random.pop2", 1.0f, 0.3f);
#endif

	minecraft->gui.flashSlot(inventory->selected);
	return true;
}

void IngameBlockSelectionScreen::tick()
{
	_blockList->tick();
	super::tick();
}

void IngameBlockSelectionScreen::render( int xm, int ym, float a )
{
	glDisable2(GL_DEPTH_TEST);
	glEnable2(GL_BLEND);

	if (minecraft->isCreativeMode()) {
		fill(0, 0, width, height, 0xff141414);
	}

	IntRectangle& bbox = _blockList->rect;
	Tesselator& t = Tesselator::instance;

	if (!minecraft->isCreativeMode()) {
		// render frame for survival
		t.colorABGR(0xffffffff);
		minecraft->textures->loadAndBindTexture("gui/itemframe.png");
		glEnable2(GL_BLEND);
		glColor4f2(1, 1, 1, 1);
		glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		blit(0, bbox.y-By, 0, 0, width, bbox.h+By+By, 215, 256);
		glDisable2(GL_BLEND);
	}

	_blockList->render(xm, ym, a);
	Screen::render(xm, ym, a);

	// Draw hotbar on top of the inventory
	if (minecraft->isCreativeMode() && !minecraft->options.getBooleanValue(OPTIONS_HIDEGUI)) {
		minecraft->gameRenderer->setupGuiScreen(false);
		int screenWidth = (int)(minecraft->width * Gui::InvGuiScale);
		int screenHeight = (int)(minecraft->height * Gui::InvGuiScale);
		int ySlot = screenHeight - 16 - 3;
		minecraft->gui.renderToolBar(a, ySlot, screenWidth);
	}

	glEnable2(GL_DEPTH_TEST);
}

void IngameBlockSelectionScreen::renderDemoOverlay() {
#ifdef DEMO_MODE
	fill(	getSlotPosX(0) - 3, getSlotPosY(1) - 3,
			getSlotPosX(9) - 3, getSlotPosY(-1) - 3, 0xa0 << 24);

	const int centerX = (getSlotPosX(4) + getSlotPosX(5)) / 2;
	const int centerY = (getSlotPosY(0) + getSlotPosY(1)) / 2 + 5;
	drawCenteredString(minecraft->font, demoVersionString, centerX, centerY, 0xffffffff);
#endif
}

void IngameBlockSelectionScreen::buttonClicked(Button* button) {
	if (button->id == bDone.id)
		minecraft->setScreen(NULL);

	if (button->id == bMenu.id)
		minecraft->screenChooser.setScreen(SCREEN_PAUSE);

	if (button->id == bCraft.id)
		minecraft->setScreen(new WorkbenchScreen(Recipe::SIZE_2X2));

	if (button == &bArmor)
		minecraft->setScreen(new ArmorScreen());

	if (button->id >= 100 && button->id < 200) {
		int categoryId = button->id - 100;
		currentCategory = categoryId;
		selectedCategoryButton = (ImageButton*)button;

		if (_blockList) {
			delete _blockList;
			_blockList = NULL;
		}

		int categoryWidth = 0;
		int categoriesHeight = height - (24 + By) - By;
		categoryWidth = categoriesHeight / 4;

		int paneLeft = categoryWidth + (int)BorderPixels * 2;
		int maxWidth = width - paneLeft - Bx;
		InventoryColumns = maxWidth / ItemSize;
		const int realWidth = InventoryColumns * ItemSize;
		const int realBx = paneLeft + (maxWidth - realWidth) / 2;

		IntRectangle rect(realBx,
#ifdef __APPLE__
			24 + By - ((width==240)?1:0), realWidth, ((width==240)?1:0) + height-By-By-20-24);
#else
			24 + By, realWidth, height-By-By-20-24);
#endif

		updateCreativeItems();
		int countItems = getItems(NULL).size();
		_blockList = new InventoryPane(this, minecraft, rect, width, BorderPixels, countItems, ItemSize, (int)BorderPixels);
		_blockList->fillMarginX = realBx;

		InventorySize = countItems;
		InventoryRows = 1 + (InventorySize-1) / InventoryColumns;

		if (guiPaneFrame) {
			guiPaneFrame->setSize((float)_blockList->rect.w + 2, (float)_blockList->rect.h + 2);
		}
	}
}

bool IngameBlockSelectionScreen::isAllowed( int slot )
{
	if (slot < 0 || slot >= (int)getItems(NULL).size())
		return false;

#ifdef DEMO_MODE
	if (slot >= (minecraft->isCreativeMode()? 28 : 27)) return false;
#endif
	return true;
}

bool IngameBlockSelectionScreen::hasClippingArea( IntRectangle& out )
{
	out = clippingArea;
	return true;
}

std::vector<const ItemInstance*> IngameBlockSelectionScreen::getItems( const InventoryPane* forPane )
{
	std::vector<const ItemInstance*> out;
	if (minecraft->isCreativeMode()) {
		for (size_t i = 0; i < creativeItemsCache.size(); ++i) {
			out.push_back(creativeItemsCache[i]);
		}
	} else {
		for (int i = Inventory::MAX_SELECTION_SIZE; i < minecraft->player->inventory->getContainerSize(); ++i) {
			ItemInstance* item = minecraft->player->inventory->getItem(i);
			if (item && !item->isNull())
				out.push_back(item);
		}
	}
	return out;
}

}

