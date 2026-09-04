#include "ClassicChestScreen.h"

#include <cmath>
#include <algorithm>
#include <chrono>

#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../renderer/gles.h"
#include "../../renderer/entity/ItemRenderer.h"
#include "../../sound/SoundEngine.h"
#include "../Font.h"
#include "../components/NinePatch.h"
#include "../../../world/entity/player/Player.h"
#include "../../../world/entity/player/Inventory.h"
#include "../../../world/inventory/FillingContainer.h"
#include "../../../world/level/tile/entity/ChestTileEntity.h"
#include "../../../platform/input/Keyboard.h"

static int getClassicChestCurrentTimeMillis() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return static_cast<int>(ms & 0x7FFFFFFF);
}

ClassicChestScreen::ClassicChestScreen(Player* player, ChestTileEntity* chest) :
    player(player),
    chest(chest),
    btnClose(1),
    guiPanelBg(NULL),
    guiSlot(NULL),
    panelX(0),
    panelY(0),
    panelWidth(176),
    panelHeight(166),
    chestGridX(0),
    chestGridY(0),
    invX(0),
    invY(0),
    hotbarX(0),
    hotbarY(0),
    carriedItem(NULL),
    dragMode(CLASSIC_CHEST_DRAG_NONE),
    dragItemOrigCount(0),
    lastClickedButton(-1),
    lastClickTime(0),
    lastClickedSlot{CLASSIC_CHEST_SLOT_NONE, -1},
    lastMouseX(0),
    lastMouseY(0),
    chestWasOpened(false)
{
}

ClassicChestScreen::~ClassicChestScreen() {
    delete guiPanelBg;
    delete guiSlot;

    delete carriedItem;
    carriedItem = NULL;

    if (chest && chest->clientSideOnly) {
        delete chest;
        chest = NULL;
    }
}

void ClassicChestScreen::init() {
    super::init();

    if (chest && !chestWasOpened) {
        chest->startOpen();
        chestWasOpened = true;
    }

    NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");
    guiPanelBg = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
    guiSlot    = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 3, 3, 18, 18);

    buttons.push_back(&btnClose);
}

void ClassicChestScreen::setupPositions() {
    super::setupPositions();

    panelWidth = 176;
    panelHeight = 166;
    panelX = (width - panelWidth) / 2;
    panelY = (height - panelHeight) / 2;

    chestGridX = panelX + 7;
    chestGridY = panelY + 17;

    invX = panelX + (panelWidth - INV_COLS * SLOT_SIZE) / 2;
    invY = panelY + 84;

    hotbarX = invX;
    hotbarY = panelY + 142;

    btnClose.x = panelX + panelWidth - 18;
    btnClose.y = panelY + 4;
    btnClose.width = 14;
    btnClose.height = 14;
}

void ClassicChestScreen::tick() {
    super::tick();
}

void ClassicChestScreen::render(int xm, int ym, float a) {
    renderBackground();

    Tesselator& t = Tesselator::instance;

    // 1. Draw main panel background
    if (guiPanelBg) {
        guiPanelBg->setSize((float)panelWidth, (float)panelHeight);
        guiPanelBg->draw(t, (float)panelX, (float)panelY);
    }

    // 2. Draw section labels
    std::string title = (chest && !chest->getName().empty()) ? chest->getName() : "Chest";
    minecraft->font->draw(title, chestGridX + 1, panelY + 6, 0x404040);
    minecraft->font->draw("Inventory", invX + 1, invY - 11, 0x404040);

    ClassicChestSlotLocation hoverLoc = CLASSIC_CHEST_SLOT_NONE;
    int hoverIndex = -1;
    getSlotAt(xm, ym, hoverLoc, hoverIndex);

    // 3. Draw 3x9 Chest Grid
    fill(chestGridX - 1, chestGridY - 1, chestGridX + CHEST_COLS * SLOT_SIZE + 1, chestGridY + CHEST_ROWS * SLOT_SIZE + 1, 0xFF373737);
    for (int row = 0; row < CHEST_ROWS; ++row) {
        for (int col = 0; col < CHEST_COLS; ++col) {
            int index = row * CHEST_COLS + col;
            int sx = chestGridX + col * SLOT_SIZE;
            int sy = chestGridY + row * SLOT_SIZE;
            bool isHover = (hoverLoc == CLASSIC_CHEST_SLOT_CHEST && hoverIndex == index);
            bool isDrag = containsDragSlot(CLASSIC_CHEST_SLOT_CHEST, index);
            ItemInstance* item = (chest && index < chest->getContainerSize()) ? chest->getItem(index) : NULL;
            drawInventorySlot(t, sx, sy, item, isHover, isDrag);
        }
    }

    // 4. Draw 3x9 Main Inventory Grid
    fill(invX - 1, invY - 1, invX + INV_COLS * SLOT_SIZE + 1, invY + INV_ROWS * SLOT_SIZE + 1, 0xFF373737);
    for (int row = 0; row < INV_ROWS; ++row) {
        for (int col = 0; col < INV_COLS; ++col) {
            int slotIdx = 9 + row * INV_COLS + col;
            int sx = invX + col * SLOT_SIZE;
            int sy = invY + row * SLOT_SIZE;
            ItemInstance* item = player ? player->inventory->getItem(slotIdx) : NULL;
            bool isHover = (hoverLoc == CLASSIC_CHEST_SLOT_INVENTORY && hoverIndex == slotIdx);
            bool isDrag = containsDragSlot(CLASSIC_CHEST_SLOT_INVENTORY, slotIdx);
            drawInventorySlot(t, sx, sy, item, isHover, isDrag);
        }
    }

    // 5. Draw 1x9 Hotbar Grid
    fill(hotbarX - 1, hotbarY - 1, hotbarX + HOTBAR_COLS * SLOT_SIZE + 1, hotbarY + SLOT_SIZE + 1, 0xFF373737);
    for (int col = 0; col < HOTBAR_COLS; ++col) {
        int sx = hotbarX + col * SLOT_SIZE;
        int sy = hotbarY;
        ItemInstance* item = player ? player->inventory->getItem(col) : NULL;
        bool isHover = (hoverLoc == CLASSIC_CHEST_SLOT_HOTBAR && hoverIndex == col);
        bool isDrag = containsDragSlot(CLASSIC_CHEST_SLOT_HOTBAR, col);
        drawInventorySlot(t, sx, sy, item, isHover, isDrag);
    }

    // 6. Render Close Button icon
    {
        float bx = static_cast<float>(btnClose.x);
        float by = static_cast<float>(btnClose.y);
        float bw = static_cast<float>(btnClose.width);
        float bh = static_cast<float>(btnClose.height);

        bool isCloseHovered = (xm >= btnClose.x && xm < btnClose.x + btnClose.width &&
                               ym >= btnClose.y && ym < btnClose.y + btnClose.height);

        const char* closeTex = isCloseHovered ? "gui/close_button_hover_light.png" : "gui/close-btn.png";
        minecraft->textures->loadAndBindTexture(closeTex);
        glColor4f2(1, 1, 1, 1);
        float iconW = 6.0f;
        float iconH = 6.0f;
        float ix = bx + (bw - iconW) / 2.0f;
        float iy = by + (bh - iconH) / 2.0f;
        t.begin();
        t.colorABGR(0xFFFFFFFF);
        t.vertexUV(ix,         iy + iconH, 0, 0.0f, 1.0f);
        t.vertexUV(ix + iconW, iy + iconH, 0, 1.0f, 1.0f);
        t.vertexUV(ix + iconW, iy,         0, 1.0f, 0.0f);
        t.vertexUV(ix,         iy,         0, 0.0f, 0.0f);
        t.draw();
    }

    // 7. Render carried item on cursor
    renderCarriedItem(xm, ym);

    // 8. Render tooltip if not holding an item
    if (!carriedItem || carriedItem->isNull()) {
        renderHoverTooltip(xm, ym);
    }
}

void ClassicChestScreen::drawInventorySlot(Tesselator& t, int x, int y, const ItemInstance* item, bool isHovered, bool isDragTarget) {
    float xx = static_cast<float>(x);
    float yy = static_cast<float>(y);

    if (guiSlot) {
        guiSlot->draw(t, xx, yy);
    }

    // Bedrock-style hover highlight
    if (isHovered || isDragTarget) {
        minecraft->textures->loadAndBindTexture("gui/slot_disabled_hover.png");
        glColor4f2(1, 1, 1, 1);
        t.begin();
        t.colorABGR(0xFFFFFFFF);
        t.vertexUV(xx,         yy + 17.0f, 0, 0.0f, 1.0f);
        t.vertexUV(xx + 18.0f, yy + 17.0f, 0, 1.0f, 1.0f);
        t.vertexUV(xx + 18.0f, yy,         0, 1.0f, 0.0f);
        t.vertexUV(xx,         yy,         0, 0.0f, 0.0f);
        t.draw();
        glColor4f2(1, 1, 1, 1);
    }

    // Render Item inside slot
    if (item && !item->isNull()) {
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, item, xx + 1.0f, yy + 1.0f, true);
        ItemRenderer::renderGuiItemDecorations(minecraft->font, item, xx + 1.0f, yy + 1.0f);
    }
}

void ClassicChestScreen::renderCarriedItem(int xm, int ym) {
    if (carriedItem && !carriedItem->isNull()) {
        int displayCount = carriedItem->count;
        if (dragMode != CLASSIC_CHEST_DRAG_NONE && !draggedSlots.empty()) {
            if (dragMode == CLASSIC_CHEST_DRAG_LEFT) {
                int countPerSlot = dragItemOrigCount / (int)draggedSlots.size();
                int remaining = dragItemOrigCount - countPerSlot * (int)draggedSlots.size();
                displayCount = remaining;
            } else if (dragMode == CLASSIC_CHEST_DRAG_RIGHT) {
                int remaining = dragItemOrigCount - (int)draggedSlots.size();
                displayCount = remaining > 0 ? remaining : 0;
            }
        }

        if (displayCount > 0) {
            ItemInstance tempStack = *carriedItem;
            tempStack.count = displayCount;
            ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &tempStack, (float)(xm - 8), (float)(ym - 8), true);
            ItemRenderer::renderGuiItemDecorations(minecraft->font, &tempStack, (float)(xm - 8), (float)(ym - 8));
        } else {
            ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, carriedItem, (float)(xm - 8), (float)(ym - 8), true);
        }
    }
}

void ClassicChestScreen::renderHoverTooltip(int xm, int ym) {
    ClassicChestSlotLocation loc = CLASSIC_CHEST_SLOT_NONE;
    int index = -1;
    if (!getSlotAt(xm, ym, loc, index)) return;

    ItemInstance* item = getSlotItem(loc, index);
    if (!item || item->isNull()) return;

    std::string name = item->getName();
    if (name.empty()) return;

    int textW = minecraft->font->width(name);
    int boxX = xm + 10;
    int boxY = ym - 12;

    if (boxX + textW + 6 > width) {
        boxX = xm - textW - 10;
    }
    if (boxY < 4) {
        boxY = 4;
    }

    fill(boxX - 3, boxY - 3, boxX + textW + 3, boxY + 11, 0xF0100010);
    fill(boxX - 2, boxY - 2, boxX + textW + 2, boxY + 10, 0x505000FF);
    minecraft->font->draw(name, boxX, boxY, 0xFFFFFFFF);
}

bool ClassicChestScreen::renderGameBehind() {
    return true;
}

void ClassicChestScreen::buttonClicked(Button* button) {
    if (button == &btnClose) {
        minecraft->setScreen(NULL);
    }
}

bool ClassicChestScreen::getSlotAt(int x, int y, ClassicChestSlotLocation& outLoc, int& outIndex) {
    outLoc = CLASSIC_CHEST_SLOT_NONE;
    outIndex = -1;

    // 1. 3x9 Chest Grid
    if (x >= chestGridX && x < chestGridX + CHEST_COLS * SLOT_SIZE &&
        y >= chestGridY && y < chestGridY + CHEST_ROWS * SLOT_SIZE) {
        int col = (x - chestGridX) / SLOT_SIZE;
        int row = (y - chestGridY) / SLOT_SIZE;
        if (col >= 0 && col < CHEST_COLS && row >= 0 && row < CHEST_ROWS) {
            outLoc = CLASSIC_CHEST_SLOT_CHEST;
            outIndex = row * CHEST_COLS + col;
            return true;
        }
    }

    // 2. 3x9 Main Inventory Grid
    if (x >= invX && x < invX + INV_COLS * SLOT_SIZE &&
        y >= invY && y < invY + INV_ROWS * SLOT_SIZE) {
        int col = (x - invX) / SLOT_SIZE;
        int row = (y - invY) / SLOT_SIZE;
        if (col >= 0 && col < INV_COLS && row >= 0 && row < INV_ROWS) {
            outLoc = CLASSIC_CHEST_SLOT_INVENTORY;
            outIndex = 9 + row * INV_COLS + col;
            return true;
        }
    }

    // 3. 1x9 Hotbar Grid
    if (x >= hotbarX && x < hotbarX + HOTBAR_COLS * SLOT_SIZE &&
        y >= hotbarY && y < hotbarY + SLOT_SIZE) {
        int col = (x - hotbarX) / SLOT_SIZE;
        if (col >= 0 && col < HOTBAR_COLS) {
            outLoc = CLASSIC_CHEST_SLOT_HOTBAR;
            outIndex = col;
            return true;
        }
    }

    return false;
}

ItemInstance* ClassicChestScreen::getSlotItem(ClassicChestSlotLocation loc, int index) {
    if (loc == CLASSIC_CHEST_SLOT_CHEST) {
        if (chest && index >= 0 && index < chest->getContainerSize()) {
            return chest->getItem(index);
        }
    } else if (loc == CLASSIC_CHEST_SLOT_INVENTORY || loc == CLASSIC_CHEST_SLOT_HOTBAR) {
        if (player && player->inventory && index >= 0 && index < player->inventory->getContainerSize()) {
            return player->inventory->getItem(index);
        }
    }
    return NULL;
}

void ClassicChestScreen::setSlotItem(ClassicChestSlotLocation loc, int index, ItemInstance* item) {
    if (loc == CLASSIC_CHEST_SLOT_CHEST) {
        if (chest && index >= 0 && index < chest->getContainerSize()) {
            if (item && !item->isNull()) {
                chest->setItem(index, item);
            } else {
                chest->clearSlot(index);
            }
            chest->setChanged();
        }
    } else if (loc == CLASSIC_CHEST_SLOT_INVENTORY || loc == CLASSIC_CHEST_SLOT_HOTBAR) {
        if (player && player->inventory && index >= 0 && index < player->inventory->getContainerSize()) {
            if (item && !item->isNull()) {
                player->inventory->setItem(index, item);
            } else {
                player->inventory->clearSlot(index);
            }
        }
    }
}

void ClassicChestScreen::clearSlot(ClassicChestSlotLocation loc, int index) {
    setSlotItem(loc, index, NULL);
}

int ClassicChestScreen::getSlotCapacity(ClassicChestSlotLocation loc, int index, const ItemInstance* item) {
    if (!item || item->isNull()) return 64;
    return item->getMaxStackSize();
}

bool ClassicChestScreen::canPlaceInSlot(ClassicChestSlotLocation loc, int index, const ItemInstance* item) {
    return true;
}

void ClassicChestScreen::handleLeftClick(ClassicChestSlotLocation loc, int index) {
    ItemInstance* slotItem = getSlotItem(loc, index);

    // 1. Cursor is empty -> Pick up slot stack
    if (!carriedItem || carriedItem->isNull()) {
        if (slotItem && !slotItem->isNull()) {
            carriedItem = new ItemInstance(*slotItem);
            clearSlot(loc, index);
        }
        return;
    }

    // 2. Cursor has item, slot is empty -> Place all
    if (!slotItem || slotItem->isNull()) {
        int cap = getSlotCapacity(loc, index, carriedItem);
        int toPlace = std::min(carriedItem->count, cap);

        ItemInstance placed = *carriedItem;
        placed.count = toPlace;
        setSlotItem(loc, index, &placed);

        carriedItem->count -= toPlace;
        if (carriedItem->count <= 0) {
            delete carriedItem;
            carriedItem = NULL;
        }
        return;
    }

    // 3. Both have items -> Merge if stackable, otherwise Swap
    if (ItemInstance::isStackable(slotItem, carriedItem)) {
        int cap = getSlotCapacity(loc, index, slotItem);
        int space = cap - slotItem->count;
        if (space > 0) {
            int toAdd = std::min(space, carriedItem->count);
            slotItem->count += toAdd;
            carriedItem->count -= toAdd;
            setSlotItem(loc, index, slotItem);

            if (carriedItem->count <= 0) {
                delete carriedItem;
                carriedItem = NULL;
            }
        }
    } else {
        // Swap items
        ItemInstance temp = *slotItem;
        setSlotItem(loc, index, carriedItem);
        *carriedItem = temp;
    }
}

void ClassicChestScreen::handleRightClick(ClassicChestSlotLocation loc, int index) {
    ItemInstance* slotItem = getSlotItem(loc, index);

    // 1. Cursor is empty -> Pick up half
    if (!carriedItem || carriedItem->isNull()) {
        if (slotItem && !slotItem->isNull()) {
            int takeCount = (slotItem->count + 1) / 2;
            int remainCount = slotItem->count - takeCount;

            carriedItem = new ItemInstance(*slotItem);
            carriedItem->count = takeCount;

            if (remainCount > 0) {
                slotItem->count = remainCount;
                setSlotItem(loc, index, slotItem);
            } else {
                clearSlot(loc, index);
            }
        }
        return;
    }

    // 2. Cursor has item -> Place 1
    if (!slotItem || slotItem->isNull()) {
        ItemInstance single = *carriedItem;
        single.count = 1;
        setSlotItem(loc, index, &single);

        carriedItem->count -= 1;
        if (carriedItem->count <= 0) {
            delete carriedItem;
            carriedItem = NULL;
        }
        return;
    }

    // 3. Both have items -> Add 1 if stackable
    if (ItemInstance::isStackable(slotItem, carriedItem)) {
        int cap = getSlotCapacity(loc, index, slotItem);
        if (slotItem->count < cap) {
            slotItem->count += 1;
            setSlotItem(loc, index, slotItem);

            carriedItem->count -= 1;
            if (carriedItem->count <= 0) {
                delete carriedItem;
                carriedItem = NULL;
            }
        }
    }
}

bool ClassicChestScreen::moveStackToRange(FillingContainer* container, ItemInstance*& source, int begin, int end) {
    if (!container || !source || source->isNull() || source->count <= 0) return false;

    bool movedAny = false;

    // First pass: Merge with existing matching stacks
    for (int i = begin; i < end && source->count > 0; ++i) {
        ItemInstance* target = container->getItem(i);
        if (target && !target->isNull() && ItemInstance::isStackable(target, source)) {
            int cap = target->getMaxStackSize();
            int space = cap - target->count;
            if (space > 0) {
                int toAdd = std::min(space, source->count);
                target->count += toAdd;
                source->count -= toAdd;
                container->setItem(i, target);
                movedAny = true;
            }
        }
    }

    // Second pass: Place into first empty slot
    for (int i = begin; i < end && source->count > 0; ++i) {
        ItemInstance* target = container->getItem(i);
        if (!target || target->isNull()) {
            int cap = source->getMaxStackSize();
            int toPlace = std::min(source->count, cap);

            ItemInstance placed = *source;
            placed.count = toPlace;
            container->setItem(i, &placed);

            source->count -= toPlace;
            movedAny = true;
        }
    }

    return movedAny;
}

bool ClassicChestScreen::moveStackToChest(ItemInstance*& source) {
    if (!chest || !source || source->isNull()) return false;
    bool moved = moveStackToRange(chest, source, 0, chest->getContainerSize());
    if (moved) chest->setChanged();
    return moved;
}

bool ClassicChestScreen::moveStackToPlayerInv(ItemInstance*& source) {
    if (!player || !player->inventory || !source || source->isNull()) return false;

    // First try main inventory (9..35), then hotbar (0..8)
    bool moved = moveStackToRange(player->inventory, source, 9, 36);
    if (source && source->count > 0) {
        moved |= moveStackToRange(player->inventory, source, 0, 9);
    }
    return moved;
}

void ClassicChestScreen::quickMove(ClassicChestSlotLocation loc, int index) {
    ItemInstance* slotItem = getSlotItem(loc, index);
    if (!slotItem || slotItem->isNull()) return;

    ItemInstance* stack = new ItemInstance(*slotItem);

    if (loc == CLASSIC_CHEST_SLOT_CHEST) {
        // Move from chest to player inventory
        if (moveStackToPlayerInv(stack)) {
            if (stack && stack->count > 0) {
                setSlotItem(loc, index, stack);
            } else {
                clearSlot(loc, index);
            }
        }
    } else if (loc == CLASSIC_CHEST_SLOT_INVENTORY || loc == CLASSIC_CHEST_SLOT_HOTBAR) {
        // Move from player inventory to chest
        if (moveStackToChest(stack)) {
            if (stack && stack->count > 0) {
                setSlotItem(loc, index, stack);
            } else {
                clearSlot(loc, index);
            }
        }
    }

    delete stack;
}

void ClassicChestScreen::swapInventorySlots(int slotA, int slotB) {
    if (!player || !player->inventory) return;
    if (slotA < 0 || slotA >= player->inventory->getContainerSize()) return;
    if (slotB < 0 || slotB >= player->inventory->getContainerSize()) return;

    ItemInstance* itemA = player->inventory->getItem(slotA);
    ItemInstance* itemB = player->inventory->getItem(slotB);

    ItemInstance* copyA = (itemA && !itemA->isNull()) ? new ItemInstance(*itemA) : NULL;
    ItemInstance* copyB = (itemB && !itemB->isNull()) ? new ItemInstance(*itemB) : NULL;

    player->inventory->setItem(slotA, copyB);
    player->inventory->setItem(slotB, copyA);

    delete copyA;
    delete copyB;
}

void ClassicChestScreen::dropFromSlot(ClassicChestSlotLocation loc, int index, bool entireStack) {
    if (!player) return;

    ItemInstance* slotItem = getSlotItem(loc, index);
    if (!slotItem || slotItem->isNull()) return;

    int dropCount = entireStack ? slotItem->count : 1;
    ItemInstance dropped = *slotItem;
    dropped.count = dropCount;

    slotItem->count -= dropCount;
    if (slotItem->count <= 0) {
        clearSlot(loc, index);
    } else {
        setSlotItem(loc, index, slotItem);
    }

    player->drop(&dropped, false);
    if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.pop", 1.0f, 1.0f);
}

void ClassicChestScreen::dropCarried(bool entireStack) {
    if (!player || !carriedItem || carriedItem->isNull()) return;

    int dropCount = entireStack ? carriedItem->count : 1;
    ItemInstance dropped = *carriedItem;
    dropped.count = dropCount;

    carriedItem->count -= dropCount;
    player->drop(&dropped, false);

    if (carriedItem->count <= 0) {
        delete carriedItem;
        carriedItem = NULL;
    }

    if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.pop", 1.0f, 1.0f);
}

bool ClassicChestScreen::isOutsidePanel(int x, int y) {
    return (x < panelX || x >= panelX + panelWidth ||
            y < panelY || y >= panelY + panelHeight);
}

void ClassicChestScreen::collectMatching(ClassicChestSlotLocation loc, int index) {
    if (!carriedItem || carriedItem->isNull()) return;

    int maxStack = carriedItem->getMaxStackSize();
    if (carriedItem->count >= maxStack) return;

    // If clicked on chest, collect from chest first
    if (loc == CLASSIC_CHEST_SLOT_CHEST && chest) {
        for (int i = 0; i < chest->getContainerSize() && carriedItem->count < maxStack; ++i) {
            ItemInstance* item = chest->getItem(i);
            if (item && !item->isNull() && ItemInstance::isStackable(item, carriedItem)) {
                int toTake = std::min(maxStack - carriedItem->count, item->count);
                carriedItem->count += toTake;
                item->count -= toTake;
                if (item->count <= 0) {
                    clearSlot(CLASSIC_CHEST_SLOT_CHEST, i);
                } else {
                    setSlotItem(CLASSIC_CHEST_SLOT_CHEST, i, item);
                }
            }
        }
    }

    // Collect from player inventory & hotbar (slots 0..35)
    if (player && player->inventory) {
        for (int i = 0; i < player->inventory->getContainerSize() && carriedItem->count < maxStack; ++i) {
            ItemInstance* item = player->inventory->getItem(i);
            if (item && !item->isNull() && ItemInstance::isStackable(item, carriedItem)) {
                int toTake = std::min(maxStack - carriedItem->count, item->count);
                carriedItem->count += toTake;
                item->count -= toTake;
                if (item->count <= 0) {
                    player->inventory->clearSlot(i);
                } else {
                    player->inventory->setItem(i, item);
                }
            }
        }
    }
}

bool ClassicChestScreen::containsDragSlot(ClassicChestSlotLocation loc, int index) const {
    ClassicChestSlotRef ref{loc, index};
    for (size_t i = 0; i < draggedSlots.size(); ++i) {
        if (draggedSlots[i] == ref) return true;
    }
    return false;
}

void ClassicChestScreen::executeDragDistribution() {
    if (!carriedItem || carriedItem->isNull() || draggedSlots.empty()) {
        draggedSlots.clear();
        dragMode = CLASSIC_CHEST_DRAG_NONE;
        return;
    }

    if (dragMode == CLASSIC_CHEST_DRAG_LEFT) {
        int countPerSlot = dragItemOrigCount / (int)draggedSlots.size();
        if (countPerSlot > 0) {
            for (size_t i = 0; i < draggedSlots.size(); ++i) {
                ClassicChestSlotLocation loc = draggedSlots[i].location;
                int idx = draggedSlots[i].index;
                ItemInstance* existing = getSlotItem(loc, idx);

                if (!existing || existing->isNull()) {
                    ItemInstance placed = *carriedItem;
                    placed.count = countPerSlot;
                    setSlotItem(loc, idx, &placed);
                    carriedItem->count -= countPerSlot;
                } else if (ItemInstance::isStackable(existing, carriedItem)) {
                    int cap = getSlotCapacity(loc, idx, existing);
                    int space = cap - existing->count;
                    int toAdd = std::min(space, countPerSlot);
                    existing->count += toAdd;
                    carriedItem->count -= toAdd;
                    setSlotItem(loc, idx, existing);
                }
            }
        }
    } else if (dragMode == CLASSIC_CHEST_DRAG_RIGHT) {
        for (size_t i = 0; i < draggedSlots.size() && carriedItem->count > 0; ++i) {
            ClassicChestSlotLocation loc = draggedSlots[i].location;
            int idx = draggedSlots[i].index;
            ItemInstance* existing = getSlotItem(loc, idx);

            if (!existing || existing->isNull()) {
                ItemInstance placed = *carriedItem;
                placed.count = 1;
                setSlotItem(loc, idx, &placed);
                carriedItem->count -= 1;
            } else if (ItemInstance::isStackable(existing, carriedItem)) {
                int cap = getSlotCapacity(loc, idx, existing);
                if (existing->count < cap) {
                    existing->count += 1;
                    carriedItem->count -= 1;
                    setSlotItem(loc, idx, existing);
                }
            }
        }
    }

    if (carriedItem && carriedItem->count <= 0) {
        delete carriedItem;
        carriedItem = NULL;
    }

    draggedSlots.clear();
    dragMode = CLASSIC_CHEST_DRAG_NONE;

    if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
}

void ClassicChestScreen::mouseClicked(int x, int y, int buttonNum) {
    super::mouseClicked(x, y, buttonNum);
    lastMouseX = x;
    lastMouseY = y;

    ClassicChestSlotLocation loc = CLASSIC_CHEST_SLOT_NONE;
    int index = -1;
    bool onSlot = getSlotAt(x, y, loc, index);

    if (isOutsidePanel(x, y)) {
        dropCarried(buttonNum == 0);
        return;
    }

    if (!onSlot) return;

    bool isShift = Keyboard::isKeyDown(Keyboard::KEY_LSHIFT);

    // Shift + Left Click
    if (isShift && buttonNum == 0) {
        quickMove(loc, index);
        if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
        return;
    }

    // Double-click to gather matching stacks
    int now = getClassicChestCurrentTimeMillis();
    ClassicChestSlotRef clickedRef{loc, index};
    if (buttonNum == 0 && carriedItem && !carriedItem->isNull() &&
        lastClickedButton == 0 && (now - lastClickTime) < 300 && lastClickedSlot == clickedRef) {
        collectMatching(loc, index);
        if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
        lastClickTime = 0;
        return;
    }

    lastClickedButton = buttonNum;
    lastClickTime = now;
    lastClickedSlot = clickedRef;

    // Start Drag Distribution if holding an item
    if (carriedItem && !carriedItem->isNull()) {
        dragMode = (buttonNum == 0) ? CLASSIC_CHEST_DRAG_LEFT : CLASSIC_CHEST_DRAG_RIGHT;
        dragItemOrigCount = carriedItem->count;
        draggedSlots.clear();
        draggedSlots.push_back(clickedRef);
        return;
    }

    if (buttonNum == 0) {
        handleLeftClick(loc, index);
        if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
    } else if (buttonNum == 1) {
        handleRightClick(loc, index);
        if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
    }
}

void ClassicChestScreen::mouseMoved(int x, int y, int dx, int dy) {
    super::mouseMoved(x, y, dx, dy);
    lastMouseX = x;
    lastMouseY = y;

    if (dragMode != CLASSIC_CHEST_DRAG_NONE && carriedItem && !carriedItem->isNull()) {
        ClassicChestSlotLocation loc = CLASSIC_CHEST_SLOT_NONE;
        int index = -1;
        if (getSlotAt(x, y, loc, index)) {
            if (canPlaceInSlot(loc, index, carriedItem)) {
                ItemInstance* existing = getSlotItem(loc, index);
                if (!existing || existing->isNull() || ItemInstance::isStackable(existing, carriedItem)) {
                    ClassicChestSlotRef ref{loc, index};
                    if (std::find(draggedSlots.begin(), draggedSlots.end(), ref) == draggedSlots.end()) {
                        draggedSlots.push_back(ref);
                    }
                }
            }
        }
    }
}

void ClassicChestScreen::mouseReleased(int x, int y, int buttonNum) {
    super::mouseReleased(x, y, buttonNum);
    lastMouseX = x;
    lastMouseY = y;

    if (dragMode != CLASSIC_CHEST_DRAG_NONE) {
        if (draggedSlots.size() > 1) {
            executeDragDistribution();
        } else if (draggedSlots.size() == 1) {
            ClassicChestSlotLocation loc = draggedSlots[0].location;
            int index = draggedSlots[0].index;
            draggedSlots.clear();
            dragMode = CLASSIC_CHEST_DRAG_NONE;
            if (buttonNum == 0) {
                handleLeftClick(loc, index);
            } else if (buttonNum == 1) {
                handleRightClick(loc, index);
            }
        } else {
            draggedSlots.clear();
            dragMode = CLASSIC_CHEST_DRAG_NONE;
        }
    }
}

void ClassicChestScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    super::mouseWheel(dx, dy, xm, ym);
}

void ClassicChestScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE || eventKey == Keyboard::KEY_E || eventKey == 27 || eventKey == 69 || eventKey == 101) {
        minecraft->setScreen(NULL);
        return;
    }

    ClassicChestSlotLocation loc = CLASSIC_CHEST_SLOT_NONE;
    int index = -1;
    bool onSlot = getSlotAt(lastMouseX, lastMouseY, loc, index);

    // Number keys 1-9 to swap with hotbar
    if (eventKey >= Keyboard::KEY_1 && eventKey <= Keyboard::KEY_9) {
        int hotbarTarget = eventKey - Keyboard::KEY_1;
        if (onSlot) {
            if (loc == CLASSIC_CHEST_SLOT_INVENTORY || loc == CLASSIC_CHEST_SLOT_HOTBAR) {
                swapInventorySlots(index, hotbarTarget);
            } else if (loc == CLASSIC_CHEST_SLOT_CHEST && chest && player && player->inventory) {
                ItemInstance* chestItem = chest->getItem(index);
                ItemInstance* hotbarItem = player->inventory->getItem(hotbarTarget);

                ItemInstance* copyChest = (chestItem && !chestItem->isNull()) ? new ItemInstance(*chestItem) : NULL;
                ItemInstance* copyHotbar = (hotbarItem && !hotbarItem->isNull()) ? new ItemInstance(*hotbarItem) : NULL;

                setSlotItem(CLASSIC_CHEST_SLOT_CHEST, index, copyHotbar);
                player->inventory->setItem(hotbarTarget, copyChest);

                delete copyChest;
                delete copyHotbar;
            }
        }
        return;
    }

    // Key Q / Ctrl+Q to drop item
    if ((eventKey == Keyboard::KEY_Q || eventKey == 81 || eventKey == 113) && onSlot) {
        bool ctrl = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL);
        dropFromSlot(loc, index, ctrl);
        return;
    }

    super::keyPressed(eventKey);
}

void ClassicChestScreen::removed() {
    super::removed();

    if (chest && chestWasOpened) {
        chest->stopOpen();
        chestWasOpened = false;
    }

    if (player && carriedItem && !carriedItem->isNull()) {
        if (!player->inventory->add(carriedItem)) {
            player->drop(carriedItem, false);
        } else {
            delete carriedItem;
        }
        carriedItem = NULL;
    }
}
