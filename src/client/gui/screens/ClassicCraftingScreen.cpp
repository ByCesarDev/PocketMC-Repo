#include "ClassicCraftingScreen.h"
#include "../Font.h"
#include "../components/NinePatch.h"
#include "../../renderer/entity/ItemRenderer.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../renderer/gles.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../sound/SoundEngine.h"
#include "../../../world/entity/player/Player.h"
#include "../../../world/entity/player/Inventory.h"
#include "../../../world/inventory/CraftingContainer.h"
#include "../../../world/item/crafting/Recipes.h"
#include "../../../world/item/Item.h"
#include "../../../util/Mth.h"
#include "../../../platform/input/Keyboard.h"
#include <sstream>
#include <algorithm>
#include <chrono>

static int getClassicCurrentTimeMillis() {
    using namespace std::chrono;
    return (int)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

ClassicCraftingScreen::ClassicCraftingScreen() :
    player(NULL),
    btnClose(1),
    guiPanelBg(NULL),
    guiSlot(NULL),
    panelX(0), panelY(0),
    panelWidth(196), panelHeight(180),
    craftGridX(0), craftGridY(0),
    arrowX(0), arrowY(0),
    outputX(0), outputY(0),
    invX(0), invY(0),
    hotbarX(0), hotbarY(0),
    craftResult(NULL),
    carriedItem(NULL),
    dragMode(CLASSIC_DRAG_NONE),
    dragItemOrigCount(0),
    lastClickedButton(-1),
    lastClickTime(0),
    lastClickedSlot{CLASSIC_SLOT_NONE, -1},
    lastMouseX(0),
    lastMouseY(0)
{
    for (int i = 0; i < CRAFT_GRID_SIZE; ++i) {
        craftSlots[i] = NULL;
    }
}

ClassicCraftingScreen::~ClassicCraftingScreen() {
    delete guiPanelBg;
    delete guiSlot;
    delete guiSlotResult;

    for (int i = 0; i < CRAFT_GRID_SIZE; ++i) {
        delete craftSlots[i];
        craftSlots[i] = NULL;
    }

    delete craftResult;
    craftResult = NULL;

    delete carriedItem;
    carriedItem = NULL;
}

void ClassicCraftingScreen::init() {
    super::init();

    player = minecraft->player;

    NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");
    guiPanelBg    = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
    guiSlot       = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 3, 3, 18, 18);
    guiSlotResult = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 3, 3, 26, 26);

    buttons.push_back(&btnClose);
}

void ClassicCraftingScreen::setupPositions() {
    super::setupPositions();

    panelWidth = 176;
    panelHeight = 166;
    panelX = (width - panelWidth) / 2;
    panelY = (height - panelHeight) / 2;

    craftGridX = panelX + 30;
    craftGridY = panelY + 17;

    arrowX = panelX + 90;
    arrowY = panelY + 35;

    outputX = panelX + 124;
    outputY = panelY + 31;

    invX = panelX + (panelWidth - INV_COLS * SLOT_SIZE) / 2;
    invY = panelY + 84;

    hotbarX = invX;
    hotbarY = panelY + 142;

    btnClose.x = panelX + panelWidth - 18;
    btnClose.y = panelY + 4;
    btnClose.width = 14;
    btnClose.height = 14;
}

void ClassicCraftingScreen::tick() {
    super::tick();
}

void ClassicCraftingScreen::render(int xm, int ym, float a) {
    renderBackground();

    Tesselator& t = Tesselator::instance;

    // 1. Draw main panel background
    if (guiPanelBg) {
        guiPanelBg->setSize((float)panelWidth, (float)panelHeight);
        guiPanelBg->draw(t, (float)panelX, (float)panelY);
    }

    // 2. Draw section labels
    minecraft->font->draw("Crafting", craftGridX - 2, panelY + 6, 0x404040);
    minecraft->font->draw("Inventory", invX + 1, invY - 11, 0x404040);

    ClassicSlotLocation hoverLoc = CLASSIC_SLOT_NONE;
    int hoverIndex = -1;
    getSlotAt(xm, ym, hoverLoc, hoverIndex);

    // 3. Draw 3x3 Crafting Grid
    // Background under the 3x3 matrix
    fill(craftGridX - 1, craftGridY - 1, craftGridX + 3 * SLOT_SIZE + 1, craftGridY + 3 * SLOT_SIZE + 1, 0xFF373737);
    for (int row = 0; row < CRAFT_GRID_ROWS; ++row) {
        for (int col = 0; col < CRAFT_GRID_COLS; ++col) {
            int index = row * CRAFT_GRID_COLS + col;
            int sx = craftGridX + col * SLOT_SIZE;
            int sy = craftGridY + row * SLOT_SIZE;
            bool isHover = (hoverLoc == CLASSIC_SLOT_CRAFT_INPUT && hoverIndex == index);
            bool isDrag = containsDragSlot(CLASSIC_SLOT_CRAFT_INPUT, index);
            drawInventorySlot(t, sx, sy, craftSlots[index], isHover, isDrag);
        }
    }

    // 4. Draw crafting arrow
    minecraft->textures->loadAndBindTexture("gui/arrow.png");
    glColor4f2(1, 1, 1, 1);
    float ax = static_cast<float>(arrowX);
    float ay = static_cast<float>(arrowY);
    float aw = 22.0f;
    float ah = 15.0f;
    t.begin();
    t.colorABGR(0xFFFFFFFF);
    t.vertexUV(ax,      ay + ah, 0, 0.0f, 1.0f);
    t.vertexUV(ax + aw, ay + ah, 0, 1.0f, 1.0f);
    t.vertexUV(ax + aw, ay,      0, 1.0f, 0.0f);
    t.vertexUV(ax,      ay,      0, 0.0f, 0.0f);
    t.draw();

    // 5. Draw Output Result Slot (26x26 big slot)
    fill(outputX - 1, outputY - 1, outputX + 26 + 1, outputY + 26 + 1, 0xFF373737);
    bool isOutputHover = (hoverLoc == CLASSIC_SLOT_CRAFT_RESULT);
    drawResultSlot(t, outputX, outputY, craftResult, isOutputHover);

    // 6. Draw 3x9 Main Inventory Grid
    fill(invX - 1, invY - 1, invX + INV_COLS * SLOT_SIZE + 1, invY + INV_ROWS * SLOT_SIZE + 1, 0xFF373737);
    for (int row = 0; row < INV_ROWS; ++row) {
        for (int col = 0; col < INV_COLS; ++col) {
            int slotIdx = 9 + row * INV_COLS + col;
            int sx = invX + col * SLOT_SIZE;
            int sy = invY + row * SLOT_SIZE;
            ItemInstance* item = player ? player->inventory->getItem(slotIdx) : NULL;
            bool isHover = (hoverLoc == CLASSIC_SLOT_INVENTORY && hoverIndex == slotIdx);
            bool isDrag = containsDragSlot(CLASSIC_SLOT_INVENTORY, slotIdx);
            drawInventorySlot(t, sx, sy, item, isHover, isDrag);
        }
    }

    // 7. Draw 1x9 Hotbar Grid
    fill(hotbarX - 1, hotbarY - 1, hotbarX + HOTBAR_COLS * SLOT_SIZE + 1, hotbarY + SLOT_SIZE + 1, 0xFF373737);
    for (int col = 0; col < HOTBAR_COLS; ++col) {
        int sx = hotbarX + col * SLOT_SIZE;
        int sy = hotbarY;
        ItemInstance* item = player ? player->inventory->getItem(col) : NULL;
        bool isHover = (hoverLoc == CLASSIC_SLOT_HOTBAR && hoverIndex == col);
        bool isDrag = containsDragSlot(CLASSIC_SLOT_HOTBAR, col);
        drawInventorySlot(t, sx, sy, item, isHover, isDrag);
    }

    // 8. Render Close Button icon
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
        float iconW = 7.0f;
        float iconH = 7.0f;
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

    // 9. Render carried item on cursor
    renderCarriedItem(xm, ym);

    // 10. Render tooltip if not holding an item
    if (!carriedItem || carriedItem->isNull()) {
        renderHoverTooltip(xm, ym);
    }
}

void ClassicCraftingScreen::drawInventorySlot(Tesselator& t, int x, int y, const ItemInstance* item, bool isHovered, bool isDragTarget) {
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

void ClassicCraftingScreen::drawResultSlot(Tesselator& t, int x, int y, const ItemInstance* item, bool isHovered) {
    float xx = static_cast<float>(x);
    float yy = static_cast<float>(y);

    if (guiSlotResult) {
        guiSlotResult->draw(t, xx, yy);
    }

    // Bedrock-style hover highlight for 26x26 slot
    if (isHovered) {
        minecraft->textures->loadAndBindTexture("gui/slot_disabled_hover.png");
        glColor4f2(1, 1, 1, 1);
        t.begin();
        t.colorABGR(0xFFFFFFFF);
        t.vertexUV(xx,         yy + 26.0f, 0, 0.0f, 1.0f);
        t.vertexUV(xx + 26.0f, yy + 26.0f, 0, 1.0f, 1.0f);
        t.vertexUV(xx + 26.0f, yy,         0, 1.0f, 0.0f);
        t.vertexUV(xx,         yy,         0, 0.0f, 0.0f);
        t.draw();
        glColor4f2(1, 1, 1, 1);
    }

    // Render Item inside slot (centered in 26x26 slot -> 5px offset)
    if (item && !item->isNull()) {
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, item, xx + 5.0f, yy + 5.0f, true);
        ItemRenderer::renderGuiItemDecorations(minecraft->font, item, xx + 5.0f, yy + 5.0f);
    }
}

void ClassicCraftingScreen::renderCarriedItem(int xm, int ym) {
    if (carriedItem && !carriedItem->isNull()) {
        int displayCount = carriedItem->count;
        if (dragMode != CLASSIC_DRAG_NONE && !draggedSlots.empty()) {
            if (dragMode == CLASSIC_DRAG_LEFT) {
                int countPerSlot = dragItemOrigCount / (int)draggedSlots.size();
                int remaining = dragItemOrigCount - countPerSlot * (int)draggedSlots.size();
                displayCount = remaining;
            } else if (dragMode == CLASSIC_DRAG_RIGHT) {
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

void ClassicCraftingScreen::renderHoverTooltip(int xm, int ym) {
    ClassicSlotLocation loc = CLASSIC_SLOT_NONE;
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

bool ClassicCraftingScreen::renderGameBehind() {
    return true;
}

void ClassicCraftingScreen::buttonClicked(Button* button) {
    if (button == &btnClose) {
        minecraft->setScreen(NULL);
    }
}

bool ClassicCraftingScreen::getSlotAt(int x, int y, ClassicSlotLocation& outLoc, int& outIndex) {
    outLoc = CLASSIC_SLOT_NONE;
    outIndex = -1;

    // 1. 3x3 Crafting Grid
    if (x >= craftGridX && x < craftGridX + 3 * SLOT_SIZE &&
        y >= craftGridY && y < craftGridY + 3 * SLOT_SIZE) {
        int col = (x - craftGridX) / SLOT_SIZE;
        int row = (y - craftGridY) / SLOT_SIZE;
        if (col >= 0 && col < CRAFT_GRID_COLS && row >= 0 && row < CRAFT_GRID_ROWS) {
            outLoc = CLASSIC_SLOT_CRAFT_INPUT;
            outIndex = row * CRAFT_GRID_COLS + col;
            return true;
        }
    }

    // 2. Output Result Slot (26x26)
    if (x >= outputX && x < outputX + 26 &&
        y >= outputY && y < outputY + 26) {
        outLoc = CLASSIC_SLOT_CRAFT_RESULT;
        outIndex = 0;
        return true;
    }

    // 3. 3x9 Main Inventory Grid
    if (x >= invX && x < invX + INV_COLS * SLOT_SIZE &&
        y >= invY && y < invY + INV_ROWS * SLOT_SIZE) {
        int col = (x - invX) / SLOT_SIZE;
        int row = (y - invY) / SLOT_SIZE;
        if (col >= 0 && col < INV_COLS && row >= 0 && row < INV_ROWS) {
            outLoc = CLASSIC_SLOT_INVENTORY;
            outIndex = 9 + row * INV_COLS + col;
            return true;
        }
    }

    // 4. 1x9 Hotbar Grid
    if (x >= hotbarX && x < hotbarX + HOTBAR_COLS * SLOT_SIZE &&
        y >= hotbarY && y < hotbarY + SLOT_SIZE) {
        int col = (x - hotbarX) / SLOT_SIZE;
        if (col >= 0 && col < HOTBAR_COLS) {
            outLoc = CLASSIC_SLOT_HOTBAR;
            outIndex = col;
            return true;
        }
    }

    return false;
}

ItemInstance* ClassicCraftingScreen::getSlotItem(ClassicSlotLocation loc, int index) {
    if (loc == CLASSIC_SLOT_CRAFT_INPUT) {
        if (index >= 0 && index < CRAFT_GRID_SIZE) return craftSlots[index];
    } else if (loc == CLASSIC_SLOT_CRAFT_RESULT) {
        return craftResult;
    } else if (loc == CLASSIC_SLOT_INVENTORY || loc == CLASSIC_SLOT_HOTBAR) {
        if (player && player->inventory && index >= 0 && index < player->inventory->getContainerSize()) {
            return player->inventory->getItem(index);
        }
    }
    return NULL;
}

void ClassicCraftingScreen::setSlotItem(ClassicSlotLocation loc, int index, ItemInstance* item) {
    if (loc == CLASSIC_SLOT_CRAFT_INPUT) {
        if (index >= 0 && index < CRAFT_GRID_SIZE) {
            delete craftSlots[index];
            craftSlots[index] = (item && !item->isNull()) ? new ItemInstance(*item) : NULL;
            updateCraftingResult();
        }
    } else if (loc == CLASSIC_SLOT_CRAFT_RESULT) {
        delete craftResult;
        craftResult = (item && !item->isNull()) ? new ItemInstance(*item) : NULL;
    } else if (loc == CLASSIC_SLOT_INVENTORY || loc == CLASSIC_SLOT_HOTBAR) {
        if (player && player->inventory && index >= 0 && index < player->inventory->getContainerSize()) {
            if (item && !item->isNull()) {
                player->inventory->setItem(index, item);
            } else {
                player->inventory->clearSlot(index);
            }
        }
    }
}

void ClassicCraftingScreen::clearSlot(ClassicSlotLocation loc, int index) {
    setSlotItem(loc, index, NULL);
}

int ClassicCraftingScreen::getSlotCapacity(ClassicSlotLocation loc, int index, const ItemInstance* item) {
    if (loc == CLASSIC_SLOT_CRAFT_RESULT) return 0;
    if (!item || item->isNull()) return 64;
    return item->getMaxStackSize();
}

bool ClassicCraftingScreen::canPlaceInSlot(ClassicSlotLocation loc, int index, const ItemInstance* item) {
    if (loc == CLASSIC_SLOT_CRAFT_RESULT) return false;
    return true;
}

void ClassicCraftingScreen::updateCraftingResult() {
    CraftingContainer container(3, 3);
    for (int i = 0; i < CRAFT_GRID_SIZE; ++i) {
        if (craftSlots[i] && !craftSlots[i]->isNull()) {
            container.setItem(i, *craftSlots[i]);
        }
    }
    ItemInstance result = Recipes::getInstance()->getItemFor(&container);
    if (!result.isNull()) {
        if (!craftResult) craftResult = new ItemInstance(result);
        else *craftResult = result;
    } else {
        delete craftResult;
        craftResult = NULL;
    }
}

void ClassicCraftingScreen::consumeCraftingIngredients() {
    for (int i = 0; i < CRAFT_GRID_SIZE; ++i) {
        if (craftSlots[i] && !craftSlots[i]->isNull()) {
            craftSlots[i]->count--;
            if (craftSlots[i]->count <= 0) {
                delete craftSlots[i];
                craftSlots[i] = NULL;
            }
        }
    }
    updateCraftingResult();
}

void ClassicCraftingScreen::handleLeftClick(ClassicSlotLocation loc, int index) {
    if (loc == CLASSIC_SLOT_CRAFT_RESULT) {
        if (!craftResult || craftResult->isNull()) return;

        if (!carriedItem || carriedItem->isNull()) {
            carriedItem = new ItemInstance(*craftResult);
            consumeCraftingIngredients();
        } else if (ItemInstance::isStackable(carriedItem, craftResult)) {
            int space = carriedItem->getMaxStackSize() - carriedItem->count;
            if (space >= craftResult->count) {
                carriedItem->count += craftResult->count;
                consumeCraftingIngredients();
            }
        }
        return;
    }

    ItemInstance* slotItem = getSlotItem(loc, index);

    if (!carriedItem || carriedItem->isNull()) {
        if (slotItem && !slotItem->isNull()) {
            carriedItem = new ItemInstance(*slotItem);
            clearSlot(loc, index);
        }
    } else {
        if (!canPlaceInSlot(loc, index, carriedItem)) return;

        if (!slotItem || slotItem->isNull()) {
            int cap = getSlotCapacity(loc, index, carriedItem);
            if (carriedItem->count <= cap) {
                setSlotItem(loc, index, carriedItem);
                delete carriedItem;
                carriedItem = NULL;
            } else {
                ItemInstance placed = *carriedItem;
                placed.count = cap;
                carriedItem->count -= cap;
                setSlotItem(loc, index, &placed);
            }
        } else if (ItemInstance::isStackable(slotItem, carriedItem)) {
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
            if (slotItem->count <= getSlotCapacity(loc, index, carriedItem)) {
                ItemInstance* temp = new ItemInstance(*slotItem);
                setSlotItem(loc, index, carriedItem);
                delete carriedItem;
                carriedItem = temp;
            }
        }
    }
}

void ClassicCraftingScreen::handleRightClick(ClassicSlotLocation loc, int index) {
    if (loc == CLASSIC_SLOT_CRAFT_RESULT) {
        handleLeftClick(loc, index);
        return;
    }

    ItemInstance* slotItem = getSlotItem(loc, index);

    if (!carriedItem || carriedItem->isNull()) {
        if (slotItem && !slotItem->isNull()) {
            int takeCount = (slotItem->count + 1) / 2;
            int remainCount = slotItem->count - takeCount;

            carriedItem = new ItemInstance(slotItem->id, takeCount, slotItem->getAuxValue());

            if (remainCount > 0) {
                slotItem->count = remainCount;
                setSlotItem(loc, index, slotItem);
            } else {
                clearSlot(loc, index);
            }
        }
    } else {
        if (!canPlaceInSlot(loc, index, carriedItem)) return;

        if (!slotItem || slotItem->isNull()) {
            ItemInstance single = *carriedItem;
            single.count = 1;
            setSlotItem(loc, index, &single);
            carriedItem->count--;
            if (carriedItem->count <= 0) {
                delete carriedItem;
                carriedItem = NULL;
            }
        } else if (ItemInstance::isStackable(slotItem, carriedItem)) {
            int cap = getSlotCapacity(loc, index, slotItem);
            if (slotItem->count < cap) {
                slotItem->count++;
                setSlotItem(loc, index, slotItem);
                carriedItem->count--;
                if (carriedItem->count <= 0) {
                    delete carriedItem;
                    carriedItem = NULL;
                }
            }
        }
    }
}

bool ClassicCraftingScreen::moveStackToRange(ItemInstance*& source, int begin, int end) {
    if (!source || source->isNull()) return false;
    bool changed = false;

    // Phase 1: Merge into existing stacks
    for (int i = begin; i < end && source && source->count > 0; ++i) {
        ItemInstance* target = player->inventory->getItem(i);
        if (target && !target->isNull() && ItemInstance::isStackable(target, source)) {
            int space = target->getMaxStackSize() - target->count;
            if (space > 0) {
                int toAdd = std::min(space, source->count);
                target->count += toAdd;
                source->count -= toAdd;
                player->inventory->setItem(i, target);
                changed = true;
            }
        }
    }

    // Phase 2: Place into empty slots
    for (int i = begin; i < end && source && source->count > 0; ++i) {
        ItemInstance* target = player->inventory->getItem(i);
        if (!target || target->isNull()) {
            player->inventory->setItem(i, source);
            source->count = 0;
            changed = true;
            break;
        }
    }

    if (source && source->count <= 0) {
        delete source;
        source = NULL;
    }
    return changed;
}

void ClassicCraftingScreen::quickMove(ClassicSlotLocation loc, int index) {
    if (loc == CLASSIC_SLOT_CRAFT_RESULT) {
        while (craftResult && !craftResult->isNull()) {
            ItemInstance* resultStack = new ItemInstance(*craftResult);
            int origCount = resultStack->count;

            // Try hotbar first, then main inventory
            bool moved = moveStackToRange(resultStack, 0, 9);
            if (resultStack && resultStack->count > 0) {
                moved |= moveStackToRange(resultStack, 9, 36);
            }

            if (moved) {
                consumeCraftingIngredients();
                delete resultStack;
            } else {
                delete resultStack;
                break;
            }
        }
        return;
    }

    if (loc == CLASSIC_SLOT_CRAFT_INPUT) {
        ItemInstance* item = craftSlots[index];
        if (!item || item->isNull()) return;

        ItemInstance* copy = new ItemInstance(*item);
        bool moved = moveStackToRange(copy, 0, 9);
        if (copy && copy->count > 0) {
            moved |= moveStackToRange(copy, 9, 36);
        }

        if (moved) {
            if (!copy || copy->count <= 0) {
                clearSlot(loc, index);
            } else {
                setSlotItem(loc, index, copy);
            }
        }
        delete copy;
        return;
    }

    if (loc == CLASSIC_SLOT_INVENTORY) {
        ItemInstance* item = player->inventory->getItem(index);
        if (!item || item->isNull()) return;

        ItemInstance* copy = new ItemInstance(*item);
        if (moveStackToRange(copy, 0, 9)) {
            if (!copy || copy->count <= 0) {
                player->inventory->clearSlot(index);
            } else {
                player->inventory->setItem(index, copy);
            }
        }
        delete copy;
        return;
    }

    if (loc == CLASSIC_SLOT_HOTBAR) {
        ItemInstance* item = player->inventory->getItem(index);
        if (!item || item->isNull()) return;

        ItemInstance* copy = new ItemInstance(*item);
        if (moveStackToRange(copy, 9, 36)) {
            if (!copy || copy->count <= 0) {
                player->inventory->clearSlot(index);
            } else {
                player->inventory->setItem(index, copy);
            }
        }
        delete copy;
        return;
    }
}

void ClassicCraftingScreen::swapInventorySlots(int slotA, int slotB) {
    if (!player || !player->inventory) return;
    ItemInstance* a = player->inventory->getItem(slotA);
    ItemInstance* b = player->inventory->getItem(slotB);

    ItemInstance* copyA = (a && !a->isNull()) ? new ItemInstance(*a) : NULL;
    ItemInstance* copyB = (b && !b->isNull()) ? new ItemInstance(*b) : NULL;

    player->inventory->setItem(slotA, copyB);
    player->inventory->setItem(slotB, copyA);

    delete copyA;
    delete copyB;

    if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
}

void ClassicCraftingScreen::dropFromSlot(ClassicSlotLocation loc, int index, bool entireStack) {
    if (!player) return;
    ItemInstance* item = getSlotItem(loc, index);
    if (!item || item->isNull()) return;

    if (entireStack || item->count == 1) {
        player->drop(new ItemInstance(*item), false);
        clearSlot(loc, index);
    } else {
        ItemInstance* single = new ItemInstance(item->id, 1, item->getAuxValue());
        player->drop(single, false);
        item->count--;
        setSlotItem(loc, index, item);
    }

    if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.pop", 1.0f, 1.0f);
}

void ClassicCraftingScreen::dropCarried(bool entireStack) {
    if (!player || !carriedItem || carriedItem->isNull()) return;

    if (entireStack || carriedItem->count == 1) {
        player->drop(carriedItem, false);
        carriedItem = NULL;
    } else {
        ItemInstance* single = new ItemInstance(carriedItem->id, 1, carriedItem->getAuxValue());
        player->drop(single, false);
        carriedItem->count--;
        if (carriedItem->count <= 0) {
            delete carriedItem;
            carriedItem = NULL;
        }
    }

    if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.pop", 1.0f, 1.0f);
}

bool ClassicCraftingScreen::isOutsidePanel(int x, int y) {
    return (x < panelX || x >= panelX + panelWidth ||
            y < panelY || y >= panelY + panelHeight);
}

void ClassicCraftingScreen::collectMatching(ClassicSlotLocation loc, int index) {
    if (!carriedItem || carriedItem->isNull()) return;

    int maxStack = carriedItem->getMaxStackSize();
    if (carriedItem->count >= maxStack) return;

    // Collect from crafting input
    for (int i = 0; i < CRAFT_GRID_SIZE && carriedItem->count < maxStack; ++i) {
        ItemInstance* item = craftSlots[i];
        if (item && !item->isNull() && ItemInstance::isStackable(item, carriedItem)) {
            int toTake = std::min(maxStack - carriedItem->count, item->count);
            carriedItem->count += toTake;
            item->count -= toTake;
            if (item->count <= 0) {
                clearSlot(CLASSIC_SLOT_CRAFT_INPUT, i);
            } else {
                setSlotItem(CLASSIC_SLOT_CRAFT_INPUT, i, item);
            }
        }
    }

    // Collect from player inventory & hotbar (slots 0..35)
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

bool ClassicCraftingScreen::containsDragSlot(ClassicSlotLocation loc, int index) const {
    ClassicSlotRef ref{loc, index};
    for (size_t i = 0; i < draggedSlots.size(); ++i) {
        if (draggedSlots[i] == ref) return true;
    }
    return false;
}

void ClassicCraftingScreen::executeDragDistribution() {
    if (!carriedItem || carriedItem->isNull() || draggedSlots.empty()) {
        draggedSlots.clear();
        dragMode = CLASSIC_DRAG_NONE;
        return;
    }

    if (dragMode == CLASSIC_DRAG_LEFT) {
        int countPerSlot = dragItemOrigCount / (int)draggedSlots.size();
        if (countPerSlot > 0) {
            for (size_t i = 0; i < draggedSlots.size(); ++i) {
                ClassicSlotLocation loc = draggedSlots[i].location;
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
    } else if (dragMode == CLASSIC_DRAG_RIGHT) {
        for (size_t i = 0; i < draggedSlots.size() && carriedItem->count > 0; ++i) {
            ClassicSlotLocation loc = draggedSlots[i].location;
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
    dragMode = CLASSIC_DRAG_NONE;

    if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
}

void ClassicCraftingScreen::mouseClicked(int x, int y, int buttonNum) {
    super::mouseClicked(x, y, buttonNum);
    lastMouseX = x;
    lastMouseY = y;

    ClassicSlotLocation loc = CLASSIC_SLOT_NONE;
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
    int now = getClassicCurrentTimeMillis();
    ClassicSlotRef clickedRef{loc, index};
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
    if (carriedItem && !carriedItem->isNull() && loc != CLASSIC_SLOT_CRAFT_RESULT) {
        dragMode = (buttonNum == 0) ? CLASSIC_DRAG_LEFT : CLASSIC_DRAG_RIGHT;
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

void ClassicCraftingScreen::mouseMoved(int x, int y, int dx, int dy) {
    super::mouseMoved(x, y, dx, dy);
    lastMouseX = x;
    lastMouseY = y;

    if (dragMode != CLASSIC_DRAG_NONE && carriedItem && !carriedItem->isNull()) {
        ClassicSlotLocation loc = CLASSIC_SLOT_NONE;
        int index = -1;
        if (getSlotAt(x, y, loc, index)) {
            if (loc != CLASSIC_SLOT_CRAFT_RESULT && canPlaceInSlot(loc, index, carriedItem)) {
                ItemInstance* existing = getSlotItem(loc, index);
                if (!existing || existing->isNull() || ItemInstance::isStackable(existing, carriedItem)) {
                    ClassicSlotRef ref{loc, index};
                    if (std::find(draggedSlots.begin(), draggedSlots.end(), ref) == draggedSlots.end()) {
                        draggedSlots.push_back(ref);
                    }
                }
            }
        }
    }
}

void ClassicCraftingScreen::mouseReleased(int x, int y, int buttonNum) {
    super::mouseReleased(x, y, buttonNum);
    lastMouseX = x;
    lastMouseY = y;

    if (dragMode != CLASSIC_DRAG_NONE) {
        if (draggedSlots.size() > 1) {
            executeDragDistribution();
        } else if (draggedSlots.size() == 1) {
            ClassicSlotLocation loc = draggedSlots[0].location;
            int index = draggedSlots[0].index;
            draggedSlots.clear();
            dragMode = CLASSIC_DRAG_NONE;
            if (buttonNum == 0) {
                handleLeftClick(loc, index);
            } else if (buttonNum == 1) {
                handleRightClick(loc, index);
            }
        } else {
            draggedSlots.clear();
            dragMode = CLASSIC_DRAG_NONE;
        }
    }
}

void ClassicCraftingScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    super::mouseWheel(dx, dy, xm, ym);
}

void ClassicCraftingScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE || eventKey == Keyboard::KEY_E || eventKey == 27 || eventKey == 69 || eventKey == 101) {
        minecraft->setScreen(NULL);
        return;
    }

    ClassicSlotLocation loc = CLASSIC_SLOT_NONE;
    int index = -1;
    bool onSlot = getSlotAt(lastMouseX, lastMouseY, loc, index);

    // Number keys 1-9 to swap with hotbar
    if (eventKey >= Keyboard::KEY_1 && eventKey <= Keyboard::KEY_9) {
        int hotbarTarget = eventKey - Keyboard::KEY_1;
        if (onSlot && loc != CLASSIC_SLOT_CRAFT_RESULT) {
            if (loc == CLASSIC_SLOT_INVENTORY) {
                swapInventorySlots(index, hotbarTarget);
            } else if (loc == CLASSIC_SLOT_HOTBAR) {
                swapInventorySlots(index, hotbarTarget);
            } else if (loc == CLASSIC_SLOT_CRAFT_INPUT) {
                ItemInstance* craftItem = craftSlots[index];
                ItemInstance* hotbarItem = player->inventory->getItem(hotbarTarget);

                ItemInstance* copyCraft = (craftItem && !craftItem->isNull()) ? new ItemInstance(*craftItem) : NULL;
                ItemInstance* copyHotbar = (hotbarItem && !hotbarItem->isNull()) ? new ItemInstance(*hotbarItem) : NULL;

                setSlotItem(CLASSIC_SLOT_CRAFT_INPUT, index, copyHotbar);
                player->inventory->setItem(hotbarTarget, copyCraft);

                delete copyCraft;
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

void ClassicCraftingScreen::removed() {
    super::removed();
    if (!player) return;

    // 1. Safe return of carried item on cursor
    if (carriedItem && !carriedItem->isNull()) {
        if (!player->inventory->add(carriedItem)) {
            player->drop(carriedItem, false);
        } else {
            delete carriedItem;
        }
        carriedItem = NULL;
    }

    // 2. Safe return of all 9 crafting input slots
    for (int i = 0; i < CRAFT_GRID_SIZE; ++i) {
        if (craftSlots[i] && !craftSlots[i]->isNull()) {
            if (!player->inventory->add(craftSlots[i])) {
                player->drop(craftSlots[i], false);
            } else {
                delete craftSlots[i];
            }
            craftSlots[i] = NULL;
        }
    }

    delete craftResult;
    craftResult = NULL;
}
