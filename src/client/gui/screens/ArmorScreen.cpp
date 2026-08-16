#include "ArmorScreen.h"
#include "../../../locale/I18n.h"
#include "../Screen.h"
#include "../components/NinePatch.h"
#include "../../sound/SoundEngine.h"
#include "../../Minecraft.h"
#include "../Gui.h"
#include "crafting/WorkbenchScreen.h"
#include "../../player/LocalPlayer.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/entity/ItemRenderer.h"
#include "../../../world/item/Item.h"
#include "../../../world/item/ItemCategory.h"
#include "../../../world/item/ItemInstance.h"
#include "../../../locale/I18n.h"
#include "../../../world/entity/player/Player.h"
#include "../../../world/entity/player/Inventory.h"
#include "../../../world/entity/item/ItemEntity.h"
#include "../../../world/level/Level.h"
#include "../../../network/RakNetInstance.h"
#include "../../renderer/entity/EntityRenderDispatcher.h"
#include "../../renderer/GuiShader.h"
#include "../../../world/item/ArmorItem.h"

static void setIfNotSet(bool& ref, bool condition) {
	ref = (ref || condition);
}

const int   descFrameWidth = 100;

const int rgbActive = 0xfff0f0f0;
const int rgbInactive = 0xc0635558;
const int rgbInactiveShadow = 0xc0aaaaaa;

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


ArmorScreen::ArmorScreen():
	inventoryPane(NULL),
	btnArmor0(0),
	btnArmor1(1),
	btnArmor2(2),
	btnArmor3(3),
	btnClose(4, ""),
	bCraft(6, I18n::get("gui.craft")),
	bHeader (5, I18n::get("gui.inventory")),
	guiBackground(NULL),
	guiSlot(NULL),
	guiPaneFrame(NULL),
	guiPlayerBg(NULL),
	doRecreatePane(false),
	descWidth(90)
	//guiSlotItem(NULL),
	//guiSlotItemSelected(NULL)
{
	//LOGI("Creating ArmorScreen with %p, %d\n", furnace, furnace->runningId);
}

ArmorScreen::~ArmorScreen() {
	delete inventoryPane;

	delete guiBackground;
	delete guiSlot;
	delete guiPaneFrame;
	delete guiPlayerBg;
}

void ArmorScreen::init() {
	super::init();

	player = minecraft->player;

	ImageDef def;
	def.name = "gui/spritesheet.png";
	def.x = 0;
	def.y = 1;
	def.width = def.height = 18;
	def.setSrc(IntRectangle(60, 0, 18, 18));
	btnClose.setImageDef(def, true);
	btnClose.scaleWhenPressed = false;

	buttons.push_back(&bHeader);
	buttons.push_back(&btnClose);
	buttons.push_back(&bCraft);

	armorButtons[0] = &btnArmor0;
	armorButtons[1] = &btnArmor1;
	armorButtons[2] = &btnArmor2;
	armorButtons[3] = &btnArmor3;
	for (int i = 0; i < NUM_ARMORBUTTONS; ++i)
		buttons.push_back(armorButtons[i]);

	// GUI - nine patches
	NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");

	guiBackground   = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
	guiSlot         = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 3, 3, 20, 20);
	guiPaneFrame    = builder.createSymmetrical(IntRectangle(28, 42, 4, 4), 1, 1)->setExcluded(1 << 4);
	guiPlayerBg     = builder.createSymmetrical(IntRectangle(0, 20, 8, 8), 3, 3);

	updateItems();
}

void ArmorScreen::setupPositions() {
	// Left  - Categories
	bHeader.x = bHeader.y = 0;
	bCraft.y = 0;
	bCraft.x = 0;
	bCraft.width = 48;
	bHeader.width = width - bCraft.width; // make room for craft button

	btnClose.width = btnClose.height = 19;
	btnClose.x = width - btnClose.width;
	btnClose.y = 0;

	// Inventory pane
	const int maxWidth = (int)(width/1.8f) - Bx - Bx;
	const int InventoryColumns = maxWidth / ItemSize;
	const int realWidth = InventoryColumns * ItemSize;
	const int paneWidth = realWidth + Bx + Bx;
	const int realBx = (paneWidth - realWidth) / 2;

	inventoryPaneRect = IntRectangle(realBx,
#ifdef __APPLE__
		26 + By - ((width==240)?1:0), realWidth, ((width==240)?1:0) + height-By-By-28);
#else
		26 + By, realWidth, height-By-By-28);
#endif

	for (int i = 0; i < NUM_ARMORBUTTONS; ++i) {
		Button& b = *armorButtons[i];
		b.x = paneWidth;
        b.y = inventoryPaneRect.y + 24 * i;
		b.width = 20;
		b.height = 20;
	}

	guiPlayerBgRect.y = inventoryPaneRect.y;
	int xx = armorButtons[0]->x + armorButtons[0]->width;
	int xw = width - xx;
	guiPlayerBgRect.x = xx + xw / 10;
	guiPlayerBgRect.w = xw - (xw / 10) * 2;
	guiPlayerBgRect.h = inventoryPaneRect.h;

	guiPaneFrame->setSize((float)inventoryPaneRect.w + 2, (float)inventoryPaneRect.h + 2);
	guiPlayerBg->setSize((float)guiPlayerBgRect.w, (float)guiPlayerBgRect.h);
	guiBackground->setSize((float)width, (float)height);

	updateItems();
	setupInventoryPane();
}

void ArmorScreen::tick() {
	if (inventoryPane)
		inventoryPane->tick();

	if (doRecreatePane) {
		updateItems();
		setupInventoryPane();
		doRecreatePane = false;
	}
}

void ArmorScreen::handleRenderPane(Touch::InventoryPane* pane, Tesselator& t, int xm, int ym, float a) {
	if (pane) {
		pane->render(xm, ym, a);
		guiPaneFrame->draw(t, (float)(pane->rect.x - 1), (float)(pane->rect.y - 1));
	}
}

void ArmorScreen::render(int xm, int ym, float a) {
	//renderBackground();

	Tesselator& t = Tesselator::instance;

    t.addOffset(0, 0, -500);
	guiBackground->draw(t, 0, 0);
    t.addOffset(0, 0, 500);
	glEnable2(GL_ALPHA_TEST);

	// Buttons (Left side + crafting)
	super::render(xm, ym, a);

	handleRenderPane(inventoryPane, t, xm, ym, a);

	t.colorABGR(0xffffffff);
	glColor4f2(1, 1, 1, 1);

	t.addOffset(0, 0, -490);
	guiPlayerBg->draw(t, (float)guiPlayerBgRect.x, (float)guiPlayerBgRect.y);
	t.addOffset(0, 0, 490);
	renderPlayer((float)(guiPlayerBgRect.x + guiPlayerBgRect.w / 2), 0.85f * height);

	for (int i = 0; i < NUM_ARMORBUTTONS; ++i) {
		drawSlotItemAt(t, i, player->getArmor(i), armorButtons[i]->x, armorButtons[i]->y);
	}

    // Draw hotbar on top of the armor/inventory UI
    int screenWidth = (int)(minecraft->width * Gui::InvGuiScale);
    int screenHeight = (int)(minecraft->height * Gui::InvGuiScale);
    int ySlot = screenHeight - 16 - 3;
    minecraft->gui.renderToolBar(a, ySlot, screenWidth);

	glDisable2(GL_ALPHA_TEST);
}

void ArmorScreen::renderHoverTooltip(int xm, int ym) {
	const ItemInstance* hoveredItem = NULL;

	if (inventoryPane && inventoryPane->isInside(xm, ym)) {
		ScrollingPane::GridItem gi = inventoryPane->getItemAt(xm, ym);
		std::vector<const ItemInstance*> invItems = getItems(inventoryPane);
		if (gi.id >= 0 && gi.id < invItems.size()) {
			hoveredItem = invItems[gi.id];
		}
	} else {
		for (int i = 0; i < NUM_ARMORBUTTONS; ++i) {
			if (armorButtons[i] && armorButtons[i]->isInside(xm, ym)) {
				hoveredItem = player->getArmor(i);
				break;
			}
		}
	}

	// Hotbar hover check
	if (!hoveredItem) {
		int ySlot = height - 16 - 3;
		if (ym >= ySlot && ym < height) {
			int xBase = width / 2 - minecraft->gui.getNumSlots() * 10;
			int slot = (xm - xBase) / 20;
			if (slot >= 0 && slot < minecraft->gui.getNumSlots()) {
				hoveredItem = player->inventory->getItem(slot);
			}
		}
	}

	if (hoveredItem && !hoveredItem->isNull()) {
		renderTooltip(hoveredItem->getName(), xm, ym);
	}
}

void ArmorScreen::buttonClicked(Button* button) {
	if (button == &btnClose) {
		minecraft->setScreen(NULL);
	}

	if (button->id >= 0 && button->id <= 3) {
		takeAndClearSlot(button->id);
	}

	if (button->id == bCraft.id) {
		minecraft->setScreen(new WorkbenchScreen(Recipe::SIZE_2X2));
	}
}

bool ArmorScreen::addItem(const Touch::InventoryPane* forPane, int itemIndex) {
	// If the pane is the inventory pane, translate the index into the player's inventory
	if (forPane == inventoryPane) {
		int slotIndex = Inventory::MAX_SELECTION_SIZE + itemIndex;
		ItemInstance* instance = minecraft->player->inventory->getItem(slotIndex);
		if (!instance) return false;

		// If it's an armor item, equip into armor slot
		if (ItemInstance::isArmorItem(instance)) {
			ArmorItem* item = (ArmorItem*) instance->getItem();
			ItemInstance* old = player->getArmor(item->slot);
			ItemInstance oldArmor;

			if (ItemInstance::isArmorItem(old)) {
				oldArmor = *old;
			}

			player->setArmor(item->slot, instance);

			// remove the moved item from inventory
			minecraft->player->inventory->removeItem(instance);

			if (!oldArmor.isNull()) {
				if (!player->inventory->add(&oldArmor)) {
					player->drop(new ItemInstance(oldArmor), false);
				}
			}

			doRecreatePane = true;
			return true;
		}

		// If it's not armor, move it to the selection (hotbar) like the block selection screen
		minecraft->player->inventory->moveToSelectionSlot(0, slotIndex, true);
		minecraft->player->inventory->selectSlot(0);
		minecraft->gui.flashSlot(minecraft->player->inventory->selected);
		if (minecraft->soundEngine)
			minecraft->soundEngine->playUI("random.click", 1, 1);
		return true;
	}

	// Otherwise, default to previous behaviour (if other panes are ever used)
	const ItemInstance* instance = armorItems[itemIndex];
	if (!ItemInstance::isArmorItem(instance))
		return false;

	ArmorItem* item = (ArmorItem*) instance->getItem();
	ItemInstance* old = player->getArmor(item->slot);
	ItemInstance oldArmor;

	if (ItemInstance::isArmorItem(old)) {
		oldArmor = *old;
	}

	player->setArmor(item->slot, instance);

	player->inventory->removeItem(instance);
	//@attn: this is hugely important
	armorItems[itemIndex] = NULL;

	if (!oldArmor.isNull()) {
		if (!player->inventory->add(&oldArmor)) {
			player->drop(new ItemInstance(oldArmor), false);
		}
	}

	doRecreatePane = true;
	return true;
}

bool ArmorScreen::isAllowed( int slot ) {
	return true;
}

bool ArmorScreen::renderGameBehind() {
	return false;
}

std::vector<const ItemInstance*> ArmorScreen::getItems( const Touch::InventoryPane* forPane ) {
	if (forPane == inventoryPane) {
		std::vector<const ItemInstance*> inv;
		for (int i = Inventory::MAX_SELECTION_SIZE, j = 0; i < minecraft->player->inventory->getContainerSize(); ++i, ++j)
			inv.push_back(minecraft->player->inventory->getItem(i));
		return inv;
	}
	return armorItems;
}

void ArmorScreen::updateItems() {
	armorItems.clear();

	for (int i = Inventory::MAX_SELECTION_SIZE; i < minecraft->player->inventory->getContainerSize(); ++i) {
		ItemInstance* item = minecraft->player->inventory->getItem(i);
		if (ItemInstance::isArmorItem(item))
			armorItems.push_back(item);
	}
}

bool ArmorScreen::canMoveToSlot(int slot, const ItemInstance* item) {
	return ItemInstance::isArmorItem(item)
		&& ((ArmorItem*)item)->slot == slot;
}

void ArmorScreen::setupInventoryPane() {
	// IntRectangle(0, 0, 100, 100)
	if (inventoryPane) delete inventoryPane;
	int invCount = minecraft->player->inventory->getContainerSize() - Inventory::MAX_SELECTION_SIZE;
	inventoryPane = new Touch::InventoryPane(this, minecraft, inventoryPaneRect, inventoryPaneRect.w, BorderPixels, invCount, ItemSize, (int)BorderPixels);
	inventoryPane->fillMarginX = 0;
	inventoryPane->fillMarginY = 0;
	//LOGI("Creating new pane: %d %p\n", inventoryItems.size(), inventoryPane);
}

void ArmorScreen::drawSlotItemAt( Tesselator& t, int slot, const ItemInstance* item, int x, int y)
{
	float xx = (float)x;
	float yy = (float)y;

	guiSlot->draw(t, xx, yy);

	if (item && !item->isNull()) {
		ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, item, xx + 2, yy, true);
        glDisable2(GL_TEXTURE_2D);
        ItemRenderer::renderGuiItemDecorations(item, xx + 2, yy + 3);
        glEnable2(GL_TEXTURE_2D);
		//minecraft->gui.renderSlotText(item, xx + 3, yy + 3, true, true);
	} else {
        minecraft->textures->loadAndBindTexture("gui/items.png");
        blit(x + 2, y, 15 * 16, slot * 16, 16, 16, 16, 16);
    }
}

void ArmorScreen::takeAndClearSlot( int slot ) {
	ItemInstance* item = player->getArmor(slot);
	if (!item)
		return;

	int oldSize = minecraft->player->inventory->getNumEmptySlots();

	if (!minecraft->player->inventory->add(item))
		minecraft->player->drop(new ItemInstance(*item), false);

	player->setArmor(slot, NULL);

	int newSize = minecraft->player->inventory->getNumEmptySlots();
	setIfNotSet(doRecreatePane, newSize != oldSize);
}

void ArmorScreen::renderPlayer(float xo, float yo) {
	GuiShader::unbind();
	// Push GL and player state
	glPushMatrix();

	glTranslatef(xo, yo, -200);
	float ss = 45;
	glScalef(-ss, ss, ss);

	glRotatef(180, 0, 0, 1);
	//glDisable(GL_DEPTH_TEST);

	Player* player = (Player*) minecraft->player;
	float oybr = player->yBodyRot;
	float oyr = player->yRot;
	float oxr = player->xRot;

	float t = getTimeS();

	float xd = 10 * Mth::sin(t);//(xo + 51) - xm;
	float yd = 10 * Mth::cos(t * 0.05f);//(yo + 75 - 50) - ym;

	glRotatef(45 + 90, 0, 1, 0);
	glRotatef(-45 - 90, 0, 1, 0);

	const float xtan = Mth::atan(xd / 40.0f) * +20;
	const float ytan = Mth::atan(yd / 40.0f) * -20;

	glRotatef(ytan, 1, 0, 0);

	player->yBodyRot = xtan;
	player->yRot = xtan + xtan;
	player->xRot = ytan;
	glTranslatef(0, player->heightOffset, 0);

	// Push walking anim
	float oldWAP = player->walkAnimPos;
	float oldWAS = player->walkAnimSpeed;
	float oldWASO = player->walkAnimSpeedO;

	// Set new walking anim
	player->walkAnimSpeedO = player->walkAnimSpeed = 0.25f;
	player->walkAnimPos = getTimeS() * player->walkAnimSpeed * SharedConstants::TicksPerSecond;

	EntityRenderDispatcher* rd = EntityRenderDispatcher::getInstance();
	rd->playerRotY = 180;
	rd->render(player, 0, 0, 0, 0, 1);

	// Pop walking anim
	player->walkAnimPos = oldWAP;
	player->walkAnimSpeed = oldWAS;
	player->walkAnimSpeedO = oldWASO;

	//glEnable(GL_DEPTH_TEST);
	// Pop GL and player state
	player->yBodyRot = oybr;
	player->yRot = oyr;
	player->xRot = oxr;

	glPopMatrix();
	GuiShader::bind();
}
