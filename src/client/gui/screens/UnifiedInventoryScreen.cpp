#include <string>
#include <vector>
#include <cstddef>
#include <algorithm>
#include "UnifiedInventoryScreen.h"
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
#include "../../../world/entity/player/Player.h"
#include "../../../world/entity/player/Inventory.h"
#include "../../../world/level/tile/LeafTile.h"
#include "../../../world/level/tile/TreeTile.h"
#include "../../../world/entity/item/ItemEntity.h"
#include "../../../world/level/Level.h"
#include "../../renderer/entity/EntityRenderDispatcher.h"
#include "../../renderer/GuiShader.h"
#include "../../renderer/gles.h"
#include "../../../world/item/ArmorItem.h"
#include "../../../world/level/tile/Tile.h"
#include "../../../util/Mth.h"
#include "../../../SharedConstants.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/input/Mouse.h"
#include "../../../platform/time.h"

// Category 0=Construccion(1), 1=Equipamiento(2), 2=Items(4), 3=Naturaleza(8), 4=Todos(-1)
static const char* categoryIconPaths[5] = {
    "gui/inv_icons/icon_recipe_construction.png",
    "gui/inv_icons/icon_recipe_equipment.png",
    "gui/inv_icons/icon_recipe_item.png",
    "gui/inv_icons/icon_recipe_nature.png",
    "gui/inv_icons/icon_search_inv.png"
};
static const int categoryBitmasks[5] = { 1, 2, 4, 8, -1 };

namespace Touch {
class UnifiedCategoryButton : public ImageButton {
    typedef ImageButton super;
public:
    UnifiedCategoryButton(int id, const ImageButton* const* selectedPtr,
                          const std::string& texNormal, const std::string& texSelected,
                          Textures* textures)
    :   super(id, ""),
        selectedPtr(selectedPtr),
        texNormal(texNormal),
        texSelected(texSelected),
        textures(textures)
    {}

    void renderBg(Minecraft* minecraft, int xm, int ym) override {
        bool hovered = active && (minecraft->useTouchscreen() ?
            (_currentlyDown && xm >= x && ym >= y && xm < x + width && ym < y + height) : isInside(xm, ym));

        bool isSelected = (*selectedPtr == this);
        const std::string& tex = (hovered || isSelected) ? texSelected : texNormal;
        // Unselected tabs: normal height, rendered behind the panel top edge
        // Selected tab: extends 4px down over the top border of the panel so it merges seamlessly into the white panel
        float drawH = static_cast<float>(height + (isSelected ? 4 : 0));

        textures->loadAndBindTexture(tex);
        glColor4f2(1, 1, 1, 1);

        // Stretched full-texture quad (UV 0→1 avoids any clipping/scaling issue)
        Tesselator& t = Tesselator::instance;
        t.begin();
        t.vertexUV(static_cast<float>(x),         static_cast<float>(y) + drawH, 0, 0.0f, 1.0f);
        t.vertexUV(static_cast<float>(x + width), static_cast<float>(y) + drawH, 0, 1.0f, 1.0f);
        t.vertexUV(static_cast<float>(x + width), static_cast<float>(y),         0, 1.0f, 0.0f);
        t.vertexUV(static_cast<float>(x),         static_cast<float>(y),         0, 0.0f, 0.0f);
        t.draw();
    }
    bool isSecondImage(bool hovered) override { return false; }

private:
    const ImageButton* const* selectedPtr;
    std::string texNormal;
    std::string texSelected;
    Textures* textures;
};
}

UnifiedInventoryScreen::UnifiedInventoryScreen() :
    btnClose(4, ""),
    btnModeDual(200),
    btnModeSingle(201),
    btnArmor{BlankButton(0), BlankButton(1), BlankButton(2), BlankButton(3)},
    btnCraftingTable(10),
    searchBox(0, ""),
    catalogScrollY(0.0f),
    maxCatalogScrollY(0.0f),
    currentCategory(0),
    selectedCategoryButton(NULL),
    craftResultItem(NULL),
    carriedItem(NULL),
    lastMouseX(0),
    lastMouseY(0),
    dragMode(DRAG_NONE),
    isDragging(false),
    lastClickTimeMs(0),
    lastClickLoc(SLOT_LOC_NONE),
    lastClickIndex(-1),
    lastClickButton(-1),
    guiLeftPanelBg(NULL),
    guiRightPanelBg(NULL),
    guiSlot(NULL),
    guiPaneFrame(NULL),
    guiPlayerBg(NULL),
    guiSlotCategory(NULL),
    guiSlotCategorySelected(NULL),
    guiDivider(NULL),
    guiSrvInvBg(NULL),
    guiTabSelected(NULL),
    guiTabUnselected(NULL),
    isCreative(false),
    isDualPane(false),
    player(NULL),
    selectedHotbarSlot(0)
{
    for (int i = 0; i < 4; ++i) {
        craftInputSlots[i] = NULL;
    }
}

UnifiedInventoryScreen::~UnifiedInventoryScreen() {
    delete guiLeftPanelBg;
    delete guiRightPanelBg;
    delete guiSlot;
    delete guiPaneFrame;
    delete guiPlayerBg;
    delete guiSlotCategory;
    delete guiSlotCategorySelected;
    delete guiDivider;
    delete guiSrvInvBg;
    delete guiTabSelected;
    delete guiTabUnselected;

    for (size_t i = 0; i < categoryButtons.size(); ++i) {
        delete categoryButtons[i];
    }
    categoryButtons.clear();

    for (size_t i = 0; i < catalogItems.size(); ++i) {
        delete catalogItems[i];
    }
    catalogItems.clear();

    for (int i = 0; i < 4; ++i) {
        delete craftInputSlots[i];
        craftInputSlots[i] = NULL;
    }

    delete craftResultItem;
    craftResultItem = NULL;

    delete carriedItem;
    carriedItem = NULL;
}

void UnifiedInventoryScreen::init() {
    super::init();

    player = minecraft->player;
    isCreative = minecraft->isCreativeMode();
    isDualPane = isCreative;

    NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");
    guiLeftPanelBg          = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
    guiRightPanelBg         = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
    guiSlot                 = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 3, 3, 18, 18);
    guiPaneFrame            = builder.createSymmetrical(IntRectangle(28, 42, 4, 4), 1, 1)->setExcluded(1 << 4);
    guiPlayerBg             = builder.createSymmetrical(IntRectangle(0, 20, 8, 8), 3, 3);

    // Divider between the creative catalog panel and the survival/player panel
    NinePatchFactory divBuilder(minecraft->textures, "gui/inv_icons/divider_inv_bg.png");
    guiDivider = divBuilder.createSymmetrical(IntRectangle(0, 0, 4, 4), 1, 1);

    // Header bar background on top of the survival inventory
    NinePatchFactory srvBuilder(minecraft->textures, "gui/inv_icons/srv_inv_bg.png");
    guiSrvInvBg = srvBuilder.createSymmetrical(IntRectangle(0, 0, 50, 11), 3, 3);

    // Tab buttons background for header modes
    NinePatchFactory tabBuilder(minecraft->textures, "gui/inv_icons/icon_bg_inv.png");
    guiTabUnselected = tabBuilder.createSymmetrical(IntRectangle(0, 0, 16, 12), 2, 2);

    NinePatchFactory selBuilder(minecraft->textures, "gui/inv_icons/icon_select_inv.png");
    guiTabSelected = selBuilder.createSymmetrical(IntRectangle(0, 0, 16, 18), 2, 2);

    categoryButtons.clear();
    for (int i = 0; i < NUM_CATEGORIES; ++i) {
        ImageButton* button = new Touch::UnifiedCategoryButton(
            100 + i,
            (const ImageButton**)&selectedCategoryButton,
            "gui/inv_icons/icon_bg_inv.png",
            "gui/inv_icons/icon_select_inv.png",
            minecraft->textures
        );
        categoryButtons.push_back(button);
    }
    selectedCategoryButton = categoryButtons[currentCategory];

    if (player && player->inventory) {
        selectedHotbarSlot = player->inventory->selected;
    }

    updateItems();
}

void UnifiedInventoryScreen::setupPositions() {
    int pY = (height - 168) / 2 + 10;

    if (!isCreative) {
        isDualPane = false;
    }

    if (isDualPane) {
        // Dual floating panels side by side, 4px gap (138 + 4 + 174 = 316px total)
        int totalW = 316;
        int startX = (width - totalW) / 2;

        leftPanelRect = IntRectangle(startX, pY, 138, 168);
        rightPanelRect = IntRectangle(startX + 142, pY, 174, 168);

        if (guiLeftPanelBg) {
            guiLeftPanelBg->setSize(static_cast<float>(leftPanelRect.w), static_cast<float>(leftPanelRect.h));
        }

        // Category tab buttons sitting right on top of left panel
        int btnWidth  = 24;
        int btnHeight = 20;

        for (size_t c = 0; c < categoryButtons.size(); ++c) {
            ImageButton* button = categoryButtons[c];
            if (c < 4) {
                // First 4 categories grouped together on the left
                button->x = leftPanelRect.x + static_cast<int>(c) * (btnWidth + 1);
            } else {
                // 5th category (Buscar / Todos) aligned to the far right of the panel
                button->x = leftPanelRect.x + leftPanelRect.w - btnWidth;
            }
            button->y = leftPanelRect.y - btnHeight;
            button->width = btnWidth;
            button->height = btnHeight;

            // Icons framed cleanly and centered inside the button tab
            static const int iconSize    = 12;
            static const int iconOffsetX = (btnWidth - iconSize) / 2;
            static const int iconOffsetY = (btnHeight - iconSize) / 2;
            ImageDef def;
            def.x = iconOffsetX;
            def.y = iconOffsetY;
            def.width = def.height = static_cast<float>(iconSize);
            def.name = categoryIconPaths[c];
            // No setSrc() → ImageButton uses UV 0→1 (full texture)
            button->setImageDef(def, false);
        }

        catalogPaneRect = IntRectangle(leftPanelRect.x + 6, leftPanelRect.y + 18, 126, 144);
    } else {
        // Single Survival Panel centered (174 x 168 px)
        int pW = 174;
        int startX = (width - pW) / 2;

        leftPanelRect = IntRectangle(0, 0, 0, 0);
        rightPanelRect = IntRectangle(startX, pY, pW, 168);
        catalogPaneRect = IntRectangle(0, 0, 0, 0);
    }

    if (guiRightPanelBg) {
        guiRightPanelBg->setSize(static_cast<float>(rightPanelRect.w), static_cast<float>(rightPanelRect.h));
    }

    // Top Header Bar aligned to the right of rightPanelRect (exact Bedrock margins)
    int headerW = isCreative ? 72 : 24;
    int headerH = 20;
    headerBarRect = IntRectangle(rightPanelRect.x + rightPanelRect.w - headerW, rightPanelRect.y - headerH, headerW, headerH);
    if (guiSrvInvBg) {
        guiSrvInvBg->setSize(static_cast<float>(headerBarRect.w), static_cast<float>(headerBarRect.h));
    }

    if (isCreative) {
        // Mode Buttons inside Header Bar (23x16 px)
        btnModeDual.x = headerBarRect.x + 3;
        btnModeDual.y = headerBarRect.y + 3;
        btnModeDual.width = 23;
        btnModeDual.height = 16;

        btnModeSingle.x = headerBarRect.x + 27;
        btnModeSingle.y = headerBarRect.y + 3;
        btnModeSingle.width = 23;
        btnModeSingle.height = 16;
    } else {
        btnModeDual.x = btnModeDual.y = btnModeDual.width = btnModeDual.height = 0;
        btnModeSingle.x = btnModeSingle.y = btnModeSingle.width = btnModeSingle.height = 0;
    }

    // Header Close Button on the right
    btnClose.width = 12;
    btnClose.height = 14;
    btnClose.x = headerBarRect.x + headerBarRect.w - 18;
    btnClose.y = headerBarRect.y + 4;

    // Armor Buttons (Top Left of Right Panel)
    int armorX = rightPanelRect.x + 6;
    int armorY = rightPanelRect.y + 6;
    for (int i = 0; i < NUM_ARMOR; ++i) {
        btnArmor[i].x = armorX;
        btnArmor[i].y = armorY + i * 18;
        btnArmor[i].width = 18;
        btnArmor[i].height = 18;
    }

    // 3D Player background (Center of top half)
    guiPlayerBgRect = IntRectangle(rightPanelRect.x + 28, rightPanelRect.y + 6, 48, 72);
    if (guiPlayerBg) {
        guiPlayerBg->setSize(static_cast<float>(guiPlayerBgRect.w), static_cast<float>(guiPlayerBgRect.h));
    }

    // 2x2 Crafting Area button
    btnCraftingTable.x = rightPanelRect.x + 80;
    btnCraftingTable.y = rightPanelRect.y + 6;
    btnCraftingTable.width = 88;
    btnCraftingTable.height = 72;

    updateItems();
}

void UnifiedInventoryScreen::tick() {
}

void UnifiedInventoryScreen::updateItems() {
    for (size_t i = 0; i < catalogItems.size(); ++i) {
        delete catalogItems[i];
    }
    catalogItems.clear();

    if (isCreative && isDualPane) {
        int targetMask = categoryBitmasks[currentCategory];
        bool showAll = (targetMask == -1);

        int catOrder[4] = { ItemCategory::Structures, ItemCategory::Tools, ItemCategory::FoodArmor, ItemCategory::Decorations };
        int numCatOrder = showAll ? 4 : 1;

        for (int ci = 0; ci < numCatOrder; ++ci) {
            int mask = showAll ? catOrder[ci] : targetMask;

            for (int i = 0; i < 256; ++i) {
                if (Tile::isTileAllowedInCreative(i)) {
                    Item* it = Item::items[i];
                    int cat = it ? it->category : 1;
                    if (cat <= 0) cat = 8;
                    bool match = showAll ? (cat == mask) : ((mask == 8) ? (cat == 8 || cat >= 16) : (cat == mask));
                    if (!match) continue;
                    Tile* tile = Tile::tiles[i];
                    catalogItems.push_back(new ItemInstance(tile, 1));
                }
            }
            for (int i = 256; i < 512; ++i) {
                if (Item::items[i] != NULL) {
                    Item* it = Item::items[i];
                    int cat = it->category;
                    if (cat <= 0) cat = 8;
                    bool match = showAll ? (cat == mask) : ((mask == 8) ? (cat == 8 || cat >= 16) : (cat == mask));
                    if (match)
                        catalogItems.push_back(new ItemInstance(Item::items[i], 1));
                }
            }
        }

        if (showAll) {
            for (int i = 0; i < 256; ++i) {
                if (Tile::isTileAllowedInCreative(i)) {
                    Item* it = Item::items[i];
                    int cat = it ? it->category : 1;
                    if (cat <= 0) cat = 8;
                    if (cat >= 16) {
                        Tile* tile = Tile::tiles[i];
                        catalogItems.push_back(new ItemInstance(tile, 1));
                    }
                }
            }
            for (int i = 256; i < 512; ++i) {
                if (Item::items[i] != NULL) {
                    Item* it = Item::items[i];
                    int cat = it->category;
                    if (cat <= 0) cat = 8;
                    if (cat >= 16)
                        catalogItems.push_back(new ItemInstance(Item::items[i], 1));
                }
            }
        }

        int totalRows = (static_cast<int>(catalogItems.size()) + CAT_COLS - 1) / CAT_COLS;
        int visibleRows = catalogPaneRect.h / 18;
        maxCatalogScrollY = std::max(0.0f, static_cast<float>((totalRows - visibleRows) * 18));
        catalogScrollY = std::min(catalogScrollY, maxCatalogScrollY);
    }
}

void UnifiedInventoryScreen::render(int xm, int ym, float a) {
    lastMouseX = xm;
    lastMouseY = ym;
    Tesselator& t = Tesselator::instance;

    // 1. Dark semi-transparent background overlay over the game level
    fill(0, 0, width, height, 0x80000000);

    // 2. Solid light-gray fill inside each container panel
    if (isDualPane) {
        fill(leftPanelRect.x, leftPanelRect.y, leftPanelRect.x + leftPanelRect.w, leftPanelRect.y + leftPanelRect.h, 0xFFC6C6C6);
    }
    fill(rightPanelRect.x, rightPanelRect.y, rightPanelRect.x + rightPanelRect.w, rightPanelRect.y + rightPanelRect.h, 0xFFC6C6C6);

    // 3. Draw NinePatch background borders for each container separately
    glEnable2(GL_TEXTURE_2D);
    glEnable2(GL_BLEND);
    glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f2(1, 1, 1, 1);
    t.colorABGR(0xffffffff);
    minecraft->textures->loadAndBindTexture("gui/spritesheet.png");

    // 3a. Draw inactive category tabs BEHIND the left panel border
    if (isDualPane) {
        for (size_t c = 0; c < categoryButtons.size(); ++c) {
            if (categoryButtons[c] != selectedCategoryButton) {
                categoryButtons[c]->render(minecraft, xm, ym);
            }
        }
    }

    if (isDualPane && guiLeftPanelBg) {
        guiLeftPanelBg->draw(t, static_cast<float>(leftPanelRect.x), static_cast<float>(leftPanelRect.y));
    }
    if (guiRightPanelBg) {
        guiRightPanelBg->draw(t, static_cast<float>(rightPanelRect.x), static_cast<float>(rightPanelRect.y));
    }

    // 4. Draw divider image between creative catalog panel and player/survival panel
    if (isDualPane && guiDivider) {
        int divX = leftPanelRect.x + leftPanelRect.w;
        int divW = rightPanelRect.x - divX;
        int divY = leftPanelRect.y + 3;
        int divH = leftPanelRect.h - 6;
        guiDivider->setSize(static_cast<float>(divW), static_cast<float>(divH));
        guiDivider->draw(t, static_cast<float>(divX), static_cast<float>(divY));
    }

    // 5. Draw Header Bar Background & Header Buttons (Dual, Single, Close)
    if (guiSrvInvBg) {
        guiSrvInvBg->draw(t, static_cast<float>(headerBarRect.x), static_cast<float>(headerBarRect.y));
    }

    if (isCreative) {
        // Mode Dual Button (recipe_book_icon.png with srv_inv_icon background)
        {
            float bx = static_cast<float>(btnModeDual.x);
            float by = static_cast<float>(btnModeDual.y);
            float bw = static_cast<float>(btnModeDual.width);
            float bh = static_cast<float>(btnModeDual.height);

            const char* bgPath = isDualPane ? "gui/inv_icons/srv_inv_icon_active.png" : "gui/inv_icons/srv_inv_icon.png";
            minecraft->textures->loadAndBindTexture(bgPath);
            glColor4f2(1, 1, 1, 1);
            t.begin();
            t.colorABGR(0xFFFFFFFF);
            t.vertexUV(bx,      by + bh, 0, 0.0f, 1.0f);
            t.vertexUV(bx + bw, by + bh, 0, 1.0f, 1.0f);
            t.vertexUV(bx + bw, by,      0, 1.0f, 0.0f);
            t.vertexUV(bx,      by,      0, 0.0f, 0.0f);
            t.draw();

            minecraft->textures->loadAndBindTexture("gui/inv_icons/recipe_book_icon.png");
            glColor4f2(1, 1, 1, 1);
            float iconW = 17.0f;
            float iconH = 12.0f;
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

        // Mode Single Button (inventory_icon.png with srv_inv_icon background)
        {
            float bx = static_cast<float>(btnModeSingle.x);
            float by = static_cast<float>(btnModeSingle.y);
            float bw = static_cast<float>(btnModeSingle.width);
            float bh = static_cast<float>(btnModeSingle.height);

            const char* bgPath = (!isDualPane) ? "gui/inv_icons/srv_inv_icon_active.png" : "gui/inv_icons/srv_inv_icon.png";
            minecraft->textures->loadAndBindTexture(bgPath);
            glColor4f2(1, 1, 1, 1);
            t.begin();
            t.colorABGR(0xFFFFFFFF);
            t.vertexUV(bx,      by + bh, 0, 0.0f, 1.0f);
            t.vertexUV(bx + bw, by + bh, 0, 1.0f, 1.0f);
            t.vertexUV(bx + bw, by,      0, 1.0f, 0.0f);
            t.vertexUV(bx,      by,      0, 0.0f, 0.0f);
            t.draw();

            minecraft->textures->loadAndBindTexture("gui/inv_icons/inventory_icon.png");
            glColor4f2(1, 1, 1, 1);
            float iconW = 17.0f;
            float iconH = 12.0f;
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
    }

    // Close Button (close-btn.png / close_button_hover_light.png) - 7x7 icon matching Bedrock
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

    glEnable2(GL_ALPHA_TEST);

    if (isDualPane) {
        renderLeftPanel(t, xm, ym, a);
    }

    renderRightPanel(t, xm, ym, a);

    // 6. Draw active selected category tab ON TOP of the left panel
    if (isDualPane && selectedCategoryButton) {
        selectedCategoryButton->render(minecraft, xm, ym);
    }

    renderHoverTooltip(xm, ym);

    // Render floating carried item attached to cursor on top of everything
    renderCarriedItem(xm, ym);

    glDisable2(GL_ALPHA_TEST);
}

void UnifiedInventoryScreen::renderCarriedItem(int xm, int ym) {
    if (carriedItem && !carriedItem->isNull()) {
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, carriedItem, static_cast<float>(xm - 8), static_cast<float>(ym - 8), true);
        ItemRenderer::renderGuiItemDecorations(minecraft->font, carriedItem, static_cast<float>(xm - 8), static_cast<float>(ym - 8));
    }
}

void UnifiedInventoryScreen::renderLeftPanel(Tesselator& t, int xm, int ym, float a) {
    // Fill catalog background with #5d5d5d and no black border frame
    fill(catalogPaneRect.x, catalogPaneRect.y, catalogPaneRect.x + catalogPaneRect.w, catalogPaneRect.y + catalogPaneRect.h, 0xFF5D5D5D);

    // Category title
    std::string catName;
    switch (currentCategory) {
        case 0: catName = I18n::get("itemGroup.construction"); break;
        case 1: catName = I18n::get("itemGroup.equipment"); break;
        case 2: catName = I18n::get("itemGroup.items"); break;
        case 3: catName = I18n::get("itemGroup.nature"); break;
        case 4: catName = I18n::get("itemGroup.all"); break;
        default: catName = I18n::get("itemGroup.all"); break;
    }
    minecraft->font->draw(catName, static_cast<float>(leftPanelRect.x + 8), static_cast<float>(leftPanelRect.y + 6), 0x404040);

    SlotLocation hoveredLoc = SLOT_LOC_NONE;
    int hoveredIndex = -1;
    getSlotAt(xm, ym, hoveredLoc, hoveredIndex);

    // Scissor Test: strictly clip items to the visible catalogPaneRect viewport
    glEnable2(GL_SCISSOR_TEST);
    minecraft->gui.setScissorRect(catalogPaneRect);

    // Draw 7-column Catalog Items (contiguous 18px slots, no gap)
    int startX = catalogPaneRect.x;
    int startY = catalogPaneRect.y;
    int firstRow = static_cast<int>(catalogScrollY) / 18;
    int rowOffset = static_cast<int>(catalogScrollY) % 18;

    int visibleRows = (catalogPaneRect.h / 18) + 2;

    for (int r = 0; r < visibleRows; ++r) {
        int rowIdx = firstRow + r;
        int sy = startY + r * 18 - rowOffset;

        for (int c = 0; c < CAT_COLS; ++c) {
            int itemIdx = rowIdx * CAT_COLS + c;
            if (itemIdx < 0 || itemIdx >= static_cast<int>(catalogItems.size()))
                continue;

            int sx = startX + c * 18;
            const ItemInstance* item = catalogItems[itemIdx];
            bool isHovered = (hoveredLoc == SLOT_LOC_CATALOG && hoveredIndex == itemIdx);
            if (item && !item->isNull()) {
                drawSlotItemAt(t, itemIdx, item, sx, sy, false, isHovered);
            }
        }
    }

    glDisable2(GL_SCISSOR_TEST);
}

void UnifiedInventoryScreen::renderRightPanel(Tesselator& t, int xm, int ym, float a) {
    // 1. Draw Player Background (Solid Black as in Bedrock) and 3D Player
    fill(guiPlayerBgRect.x, guiPlayerBgRect.y, guiPlayerBgRect.x + guiPlayerBgRect.w, guiPlayerBgRect.y + guiPlayerBgRect.h, 0xFF000000);

    renderPlayer(static_cast<float>(guiPlayerBgRect.x + guiPlayerBgRect.w / 2.0f), static_cast<float>(guiPlayerBgRect.y + 65.0f), xm, ym);

    SlotLocation hoveredLoc = SLOT_LOC_NONE;
    int hoveredIndex = -1;
    getSlotAt(xm, ym, hoveredLoc, hoveredIndex);

    // 2. Draw 4 Armor Slots (contiguous 18px step)
    for (int i = 0; i < NUM_ARMOR; ++i) {
        bool isHovered = (hoveredLoc == SLOT_LOC_ARMOR && hoveredIndex == i);
        drawSlotItemAt(t, i, player ? player->getArmor(i) : NULL, btnArmor[i].x, btnArmor[i].y, true, isHovered);
    }

    // 3. Draw 2x2 Crafting Area (I18n "gui.craft")
    int craftX = rightPanelRect.x + 80;
    int craftY = rightPanelRect.y + 6;
    minecraft->font->draw(I18n::get("gui.craft"), static_cast<float>(craftX), static_cast<float>(craftY), 0x404040);

    // 4 Craft Input Slots (contiguous 18px step)
    int craftSlotsY = craftY + 16;
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            int slotIdx = row * 2 + col;
            int sx = craftX + col * 18;
            int sy = craftSlotsY + row * 18;
            bool isHovered = (hoveredLoc == SLOT_LOC_CRAFT_INPUT && hoveredIndex == slotIdx);
            drawSlotItemAt(t, slotIdx, craftInputSlots[slotIdx], sx, sy, false, isHovered);
        }
    }

    // Craft Arrow ->
    minecraft->font->draw("->", static_cast<float>(craftX + 38), static_cast<float>(craftSlotsY + 10), 0x404040);

    // Craft Result Slot
    bool isResultHovered = (hoveredLoc == SLOT_LOC_CRAFT_RESULT && hoveredIndex == 0);
    drawSlotItemAt(t, 0, craftResultItem, craftX + 54, craftSlotsY + 9, false, isResultHovered);

    // 4. Draw 27 Inventory Slots (3 rows x 9 cols, contiguous 18px step)
    int invStartX = rightPanelRect.x + 6;
    int invStartY = rightPanelRect.y + 82;

    for (int row = 0; row < INV_ROWS; ++row) {
        for (int col = 0; col < INV_COLS; ++col) {
            int slotIdx = row * INV_COLS + col;
            int sx = invStartX + col * 18;
            int sy = invStartY + row * 18;

            const ItemInstance* item = NULL;
            int realSlot = Inventory::MAX_SELECTION_SIZE + slotIdx;
            if (player && player->inventory && realSlot < player->inventory->getContainerSize()) {
                item = player->inventory->getItem(realSlot);
            }

            bool isHovered = (hoveredLoc == SLOT_LOC_INVENTORY && hoveredIndex == slotIdx);
            drawSlotItemAt(t, slotIdx, item, sx, sy, false, isHovered);
        }
    }

    // 5. Draw 9 Hotbar Slots (Bottom Row, contiguous 18px step)
    int hotbarY = rightPanelRect.y + 140;
    for (int col = 0; col < HOTBAR_COLS; ++col) {
        int sx = invStartX + col * 18;
        const ItemInstance* item = (player && player->inventory && col < Inventory::MAX_SELECTION_SIZE)
            ? player->inventory->getItem(col) : NULL;

        bool isHovered = (hoveredLoc == SLOT_LOC_HOTBAR && hoveredIndex == col);
        drawSlotItemAt(t, col, item, sx, hotbarY, false, isHovered);
    }
}

void UnifiedInventoryScreen::renderPlayer(float xo, float yo, int xm, int ym) {
    if (!player) return;

    GuiShader::unbind();

    glDisable2(GL_SCISSOR_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(xo, yo, -200.0f);
    float ss = 27.0f;
    glScalef(-ss, ss, ss);
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);

    float oybr = player->yBodyRot;
    float oyr = player->yRot;
    float oxr = player->xRot;

    float xtan = 0.0f;
    float ytan = 0.0f;

    if (!minecraft->useTouchscreen()) {
        float xd = xo - static_cast<float>(xm);
        float yd = (yo - 34.0f) - static_cast<float>(ym);
        xtan = Mth::atan(xd / 40.0f) * 20.0f;
        ytan = Mth::atan(yd / 40.0f) * -20.0f;
    }

    glRotatef(ytan, 1.0f, 0.0f, 0.0f);

    player->yBodyRot = xtan;
    player->yRot = xtan + xtan;
    player->xRot = ytan;
    glTranslatef(0.0f, player->heightOffset, 0.0f);

    float oldWAP = player->walkAnimPos;
    float oldWAS = player->walkAnimSpeed;
    float oldWASO = player->walkAnimSpeedO;

    player->walkAnimPos = player->walkAnimSpeed = player->walkAnimSpeedO = 0.0f;

    EntityRenderDispatcher* rd = EntityRenderDispatcher::getInstance();
    if (rd) {
        rd->playerRotY = 180.0f;
        rd->render(player, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    player->walkAnimPos = oldWAP;
    player->walkAnimSpeed = oldWAS;
    player->walkAnimSpeedO = oldWASO;

    player->yBodyRot = oybr;
    player->yRot = oyr;
    player->xRot = oxr;

    glPopMatrix();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);

    GuiShader::bind();
}

void UnifiedInventoryScreen::drawSlotItemAt(Tesselator& t, int slot, const ItemInstance* item, int x, int y, bool isArmorSlot, bool isHovered) {
    float xx = static_cast<float>(x);
    float yy = static_cast<float>(y);

    if (guiSlot)
        guiSlot->draw(t, xx, yy);

    // Green Bedrock-style hover texture drawn underneath the item/block icon
    if (isHovered) {
        minecraft->textures->loadAndBindTexture("gui/slot_disabled_hover.png");
        glColor4f2(1, 1, 1, 1);
        t.begin();
        t.colorABGR(0xFFFFFFFF);
        t.vertexUV(xx,         yy + 17.0f, 0, 0.0f, 1.0f);
        t.vertexUV(xx + 18.0f, yy + 17.0f, 0, 1.0f, 1.0f);
        t.vertexUV(xx + 18.0f, yy,         0, 1.0f, 0.0f);
        t.vertexUV(xx,         yy,         0, 0.0f, 0.0f);
        t.draw();
    }

    if (item && !item->isNull()) {
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, item, xx + 1.0f, yy + 1.0f, true);
        ItemRenderer::renderGuiItemDecorations(minecraft->font, item, xx + 1.0f, yy + 1.0f);
    } else if (isArmorSlot) {
        minecraft->textures->loadAndBindTexture("gui/items.png");
        blit(x + 1, y + 1, 15 * 16, slot * 16, 16, 16, 16, 16);
    }
}

void UnifiedInventoryScreen::renderHoverTooltip(int xm, int ym) {
    if (carriedItem && !carriedItem->isNull()) return;

    const ItemInstance* hovered = NULL;
    SlotLocation loc;
    int index;

    if (getSlotAt(xm, ym, loc, index)) {
        switch (loc) {
            case SLOT_LOC_CATALOG:
                if (index >= 0 && index < static_cast<int>(catalogItems.size()))
                    hovered = catalogItems[index];
                break;
            case SLOT_LOC_ARMOR:
                if (player) hovered = player->getArmor(index);
                break;
            case SLOT_LOC_CRAFT_INPUT:
                hovered = craftInputSlots[index];
                break;
            case SLOT_LOC_CRAFT_RESULT:
                hovered = craftResultItem;
                break;
            case SLOT_LOC_INVENTORY:
                if (player && player->inventory) {
                    int realSlot = Inventory::MAX_SELECTION_SIZE + index;
                    if (realSlot < player->inventory->getContainerSize())
                        hovered = player->inventory->getItem(realSlot);
                }
                break;
            case SLOT_LOC_HOTBAR:
                if (player && player->inventory && index < Inventory::MAX_SELECTION_SIZE)
                    hovered = player->inventory->getItem(index);
                break;
            default:
                break;
        }
    }

    if (hovered && !hovered->isNull()) {
        renderTooltip(hovered->getName(), xm, ym);
    }
}

bool UnifiedInventoryScreen::renderGameBehind() {
    return true;
}

void UnifiedInventoryScreen::buttonClicked(Button* button) {
    if (button == &btnClose) {
        minecraft->setScreen(NULL);
        return;
    }

    // Category Buttons (Creative) - now 5 categories
    if (button->id >= 100 && button->id < 100 + NUM_CATEGORIES) {
        currentCategory = button->id - 100;
        selectedCategoryButton = categoryButtons[currentCategory];
        catalogScrollY = 0.0f;
        updateItems();
        return;
    }
}

bool UnifiedInventoryScreen::getSlotAt(int x, int y, SlotLocation& outLoc, int& outIndex) {
    // 1. Armor slots (contiguous 18px step)
    int armorX = rightPanelRect.x + 6;
    int armorY = rightPanelRect.y + 6;
    for (int i = 0; i < NUM_ARMOR; ++i) {
        int sy = armorY + i * 18;
        if (x >= armorX && x < armorX + 18 && y >= sy && y < sy + 18) {
            outLoc = SLOT_LOC_ARMOR;
            outIndex = i;
            return true;
        }
    }

    // 2. 2x2 Crafting Input Slots (contiguous 18px step)
    int craftX = rightPanelRect.x + 80;
    int craftSlotsY = rightPanelRect.y + 22;
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            int sx = craftX + col * 18;
            int sy = craftSlotsY + row * 18;
            if (x >= sx && x < sx + 18 && y >= sy && y < sy + 18) {
                outLoc = SLOT_LOC_CRAFT_INPUT;
                outIndex = row * 2 + col;
                return true;
            }
        }
    }

    // 3. Crafting Result Slot
    int resX = craftX + 54;
    int resY = craftSlotsY + 9;
    if (x >= resX && x < resX + 18 && y >= resY && y < resY + 18) {
        outLoc = SLOT_LOC_CRAFT_RESULT;
        outIndex = 0;
        return true;
    }

    // 4. 27 Inventory Slots (contiguous 18px step)
    int invStartX = rightPanelRect.x + 6;
    int invStartY = rightPanelRect.y + 82;
    for (int row = 0; row < INV_ROWS; ++row) {
        for (int col = 0; col < INV_COLS; ++col) {
            int sx = invStartX + col * 18;
            int sy = invStartY + row * 18;
            if (x >= sx && x < sx + 18 && y >= sy && y < sy + 18) {
                outLoc = SLOT_LOC_INVENTORY;
                outIndex = row * INV_COLS + col;
                return true;
            }
        }
    }

    // 5. 9 Hotbar Slots (contiguous 18px step)
    int hotbarY = rightPanelRect.y + 140;
    for (int col = 0; col < HOTBAR_COLS; ++col) {
        int sx = invStartX + col * 18;
        if (x >= sx && x < sx + 18 && y >= hotbarY && y < hotbarY + 18) {
            outLoc = SLOT_LOC_HOTBAR;
            outIndex = col;
            return true;
        }
    }

    // 6. Left Catalog Slots (Creative / Recipe dual mode, contiguous 18px step)
    if (isCreative && isDualPane) {
        int startX = catalogPaneRect.x;
        int startY = catalogPaneRect.y;
        if (x >= catalogPaneRect.x && x < catalogPaneRect.x + catalogPaneRect.w &&
            y >= catalogPaneRect.y && y < catalogPaneRect.y + catalogPaneRect.h) {
            int col = (x - startX) / 18;
            int firstRow = static_cast<int>(catalogScrollY) / 18;
            int rowOffset = static_cast<int>(catalogScrollY) % 18;
            int relativeY = (y - startY + rowOffset);
            int row = firstRow + (relativeY / 18);

            if (col >= 0 && col < CAT_COLS && row >= 0) {
                int itemIdx = row * CAT_COLS + col;
                if (itemIdx >= 0 && itemIdx < static_cast<int>(catalogItems.size())) {
                    outLoc = SLOT_LOC_CATALOG;
                    outIndex = itemIdx;
                    return true;
                }
            }
        }
    }

    outLoc = SLOT_LOC_NONE;
    outIndex = -1;
    return false;
}

ItemInstance* UnifiedInventoryScreen::getSlotItem(SlotLocation loc, int index) {
    if (!player) return NULL;
    switch (loc) {
        case SLOT_LOC_HOTBAR:
            if (player->inventory && index >= 0 && index < Inventory::MAX_SELECTION_SIZE)
                return player->inventory->getItem(index);
            break;
        case SLOT_LOC_INVENTORY:
            if (player->inventory && index >= 0) {
                int realSlot = Inventory::MAX_SELECTION_SIZE + index;
                if (realSlot < player->inventory->getContainerSize())
                    return player->inventory->getItem(realSlot);
            }
            break;
        case SLOT_LOC_ARMOR:
            if (index >= 0 && index < NUM_ARMOR)
                return player->getArmor(index);
            break;
        case SLOT_LOC_CRAFT_INPUT:
            if (index >= 0 && index < 4)
                return craftInputSlots[index];
            break;
        case SLOT_LOC_CRAFT_RESULT:
            return craftResultItem;
        case SLOT_LOC_CATALOG:
            if (index >= 0 && index < static_cast<int>(catalogItems.size()))
                return catalogItems[index];
            break;
        default:
            break;
    }
    return NULL;
}

void UnifiedInventoryScreen::setSlotItem(SlotLocation loc, int index, ItemInstance* item) {
    if (!player) return;
    switch (loc) {
        case SLOT_LOC_HOTBAR:
            if (player->inventory && index >= 0 && index < Inventory::MAX_SELECTION_SIZE)
                player->inventory->setItem(index, item);
            break;
        case SLOT_LOC_INVENTORY:
            if (player->inventory && index >= 0) {
                int realSlot = Inventory::MAX_SELECTION_SIZE + index;
                if (realSlot < player->inventory->getContainerSize())
                    player->inventory->setItem(realSlot, item);
            }
            break;
        case SLOT_LOC_ARMOR:
            if (index >= 0 && index < NUM_ARMOR)
                player->setArmor(index, item);
            break;
        case SLOT_LOC_CRAFT_INPUT:
            if (index >= 0 && index < 4) {
                craftInputSlots[index] = item;
                updateCraftingResult();
            }
            break;
        default:
            break;
    }
}

void UnifiedInventoryScreen::clearSlot(SlotLocation loc, int index) {
    if (!player) return;
    switch (loc) {
        case SLOT_LOC_HOTBAR:
            if (player->inventory && index >= 0 && index < Inventory::MAX_SELECTION_SIZE)
                player->inventory->clearSlot(index);
            break;
        case SLOT_LOC_INVENTORY:
            if (player->inventory && index >= 0) {
                int realSlot = Inventory::MAX_SELECTION_SIZE + index;
                if (realSlot < player->inventory->getContainerSize())
                    player->inventory->clearSlot(realSlot);
            }
            break;
        case SLOT_LOC_ARMOR:
            if (index >= 0 && index < NUM_ARMOR)
                player->setArmor(index, NULL);
            break;
        case SLOT_LOC_CRAFT_INPUT:
            if (index >= 0 && index < 4) {
                delete craftInputSlots[index];
                craftInputSlots[index] = NULL;
                updateCraftingResult();
            }
            break;
        default:
            break;
    }
}

int UnifiedInventoryScreen::getSlotCapacity(SlotLocation loc, int index, const ItemInstance* item) {
    if (!item || item->isNull()) return 0;
    if (!canPlaceInSlot(loc, index, item)) return 0;
    ItemInstance* existing = getSlotItem(loc, index);
    int maxStack = std::min(item->getMaxStackSize(), player ? player->inventory->getMaxStackSize() : 64);
    if (!existing || existing->isNull()) {
        return maxStack;
    }
    if (ItemInstance::isStackable(existing, item)) {
        return std::max(0, maxStack - existing->count);
    }
    return 0;
}

bool UnifiedInventoryScreen::canPlaceInSlot(SlotLocation loc, int index, const ItemInstance* item) {
    if (loc == SLOT_LOC_CATALOG || loc == SLOT_LOC_CRAFT_RESULT || loc == SLOT_LOC_NONE)
        return false;
    if (loc == SLOT_LOC_ARMOR) {
        if (!item || item->isNull()) return false;
        if (!ItemInstance::isArmorItem(item)) return false;
        ArmorItem* armor = static_cast<ArmorItem*>(item->getItem());
        return armor && armor->slot == index;
    }
    return true;
}

void UnifiedInventoryScreen::mouseMoved(int x, int y, int dx, int dy) {
    super::mouseMoved(x, y, dx, dy);
    lastMouseX = x;
    lastMouseY = y;

    if (isDragging && dragMode != DRAG_NONE) {
        SlotLocation loc;
        int index;
        if (getSlotAt(x, y, loc, index)) {
            if (canPlaceInSlot(loc, index, carriedItem) && !containsDragSlot(loc, index)) {
                dragSlots.push_back({loc, index});
            }
        }
    }
}

void UnifiedInventoryScreen::handleLeftClick(SlotLocation loc, int index) {
    if (!player || !player->inventory) return;

    if (loc == SLOT_LOC_CATALOG) {
        if (!isCreative) return;
        if (carriedItem != NULL) {
            delete carriedItem;
            carriedItem = NULL;
        } else if (index >= 0 && index < static_cast<int>(catalogItems.size())) {
            const ItemInstance* catItem = catalogItems[index];
            if (catItem) {
                int stackCount = catItem->getMaxStackSize();
                if (stackCount <= 0) stackCount = 64;
                carriedItem = new ItemInstance(catItem->id, stackCount, catItem->getAuxValue());
            }
        }
        return;
    }

    if (loc == SLOT_LOC_CRAFT_RESULT) {
        if (craftResultItem && !craftResultItem->isNull()) {
            if (carriedItem == NULL) {
                carriedItem = new ItemInstance(*craftResultItem);
                for (int i = 0; i < 4; ++i) {
                    if (craftInputSlots[i] && !craftInputSlots[i]->isNull()) {
                        craftInputSlots[i]->count--;
                        if (craftInputSlots[i]->count <= 0) {
                            delete craftInputSlots[i];
                            craftInputSlots[i] = NULL;
                        }
                    }
                }
                updateCraftingResult();
            } else if (ItemInstance::isStackable(carriedItem, craftResultItem)) {
                int maxStack = std::min(craftResultItem->getMaxStackSize(), player->inventory->getMaxStackSize());
                if (carriedItem->count + craftResultItem->count <= maxStack) {
                    carriedItem->count += craftResultItem->count;
                    for (int i = 0; i < 4; ++i) {
                        if (craftInputSlots[i] && !craftInputSlots[i]->isNull()) {
                            craftInputSlots[i]->count--;
                            if (craftInputSlots[i]->count <= 0) {
                                delete craftInputSlots[i];
                                craftInputSlots[i] = NULL;
                            }
                        }
                    }
                    updateCraftingResult();
                }
            }
        }
        return;
    }

    if (loc == SLOT_LOC_ARMOR) {
        ItemInstance* armorPiece = player->getArmor(index);
        if (carriedItem == NULL) {
            if (armorPiece && !armorPiece->isNull()) {
                carriedItem = new ItemInstance(*armorPiece);
                player->setArmor(index, NULL);
            }
        } else if (canPlaceInSlot(loc, index, carriedItem)) {
            ItemInstance* oldArmor = player->getArmor(index);
            player->setArmor(index, carriedItem);
            carriedItem = (oldArmor && !oldArmor->isNull()) ? new ItemInstance(*oldArmor) : NULL;
        }
        return;
    }

    ItemInstance* target = getSlotItem(loc, index);

    // Case 1: Cursor empty -> take all
    if (carriedItem == NULL) {
        if (target && !target->isNull()) {
            carriedItem = new ItemInstance(*target);
            clearSlot(loc, index);
        }
        return;
    }

    // Case 2: Cursor occupied -> slot empty
    if (target == NULL || target->isNull()) {
        setSlotItem(loc, index, carriedItem);
        carriedItem = NULL;
        return;
    }

    // Case 3: Cursor & Slot compatible -> merge
    if (ItemInstance::isStackable(target, carriedItem)) {
        int maxStack = std::min(target->getMaxStackSize(), player->inventory->getMaxStackSize());
        int space = maxStack - target->count;
        if (space > 0) {
            int toAdd = std::min(space, carriedItem->count);
            target->count += toAdd;
            carriedItem->count -= toAdd;
            if (carriedItem->count <= 0) {
                delete carriedItem;
                carriedItem = NULL;
            }
        }
        return;
    }

    // Case 4: Incompatible -> swap
    ItemInstance* temp = new ItemInstance(*target);
    setSlotItem(loc, index, carriedItem);
    carriedItem = temp;
}

void UnifiedInventoryScreen::handleRightClick(SlotLocation loc, int index) {
    if (!player || !player->inventory) return;

    if (loc == SLOT_LOC_CATALOG) {
        if (!isCreative) return;
        if (carriedItem == NULL && index >= 0 && index < static_cast<int>(catalogItems.size())) {
            const ItemInstance* catItem = catalogItems[index];
            if (catItem) {
                carriedItem = new ItemInstance(catItem->id, 1, catItem->getAuxValue());
            }
        }
        return;
    }

    if (loc == SLOT_LOC_CRAFT_RESULT || loc == SLOT_LOC_ARMOR) {
        handleLeftClick(loc, index);
        return;
    }

    ItemInstance* target = getSlotItem(loc, index);

    // Case 1: Cursor empty -> take half
    if (carriedItem == NULL) {
        if (!target || target->isNull()) return;
        int take = (target->count + 1) / 2;
        carriedItem = new ItemInstance(*target);
        carriedItem->count = take;
        target->count -= take;
        if (target->count <= 0) {
            clearSlot(loc, index);
        }
        return;
    }

    // Case 2: Cursor occupied -> slot empty (place 1)
    if (target == NULL || target->isNull()) {
        ItemInstance* one = new ItemInstance(*carriedItem);
        one->count = 1;
        setSlotItem(loc, index, one);
        carriedItem->count--;
        if (carriedItem->count <= 0) {
            delete carriedItem;
            carriedItem = NULL;
        }
        return;
    }

    // Case 3: Cursor occupied -> compatible slot (place 1)
    if (ItemInstance::isStackable(target, carriedItem)) {
        int maxStack = std::min(target->getMaxStackSize(), player->inventory->getMaxStackSize());
        if (target->count < maxStack) {
            target->count++;
            carriedItem->count--;
            if (carriedItem->count <= 0) {
                delete carriedItem;
                carriedItem = NULL;
            }
        }
    }
}

bool UnifiedInventoryScreen::moveStackToRange(ItemInstance*& source, int begin, int end) {
    if (!player || !player->inventory || !source || source->isNull() || source->count <= 0)
        return false;

    bool changed = false;

    // Pass 1: merge into existing compatible stacks
    for (int i = begin; i < end && source && source->count > 0; ++i) {
        ItemInstance* dst = player->inventory->getItem(i);
        if (!dst || dst->isNull()) continue;
        if (!ItemInstance::isStackable(dst, source)) continue;

        int maxStack = std::min(dst->getMaxStackSize(), player->inventory->getMaxStackSize());
        int space = maxStack - dst->count;
        if (space <= 0) continue;

        int toAdd = std::min(space, source->count);
        dst->count += toAdd;
        source->count -= toAdd;
        changed = true;
    }

    // Pass 2: fill empty slots
    for (int i = begin; i < end && source && source->count > 0; ++i) {
        ItemInstance* dst = player->inventory->getItem(i);
        if (dst && !dst->isNull()) continue;

        int maxStack = std::min(source->getMaxStackSize(), player->inventory->getMaxStackSize());
        int toMove = std::min(source->count, maxStack);

        ItemInstance* moved = new ItemInstance(*source);
        moved->count = toMove;
        player->inventory->setItem(i, moved);
        source->count -= toMove;
        changed = true;
    }

    return changed;
}

void UnifiedInventoryScreen::quickMove(SlotLocation loc, int index) {
    if (!player || !player->inventory) return;

    if (loc == SLOT_LOC_HOTBAR) {
        ItemInstance* item = player->inventory->getItem(index);
        if (!item || item->isNull()) return;

        moveStackToRange(item, Inventory::MAX_SELECTION_SIZE, player->inventory->getContainerSize());
        if (item->count <= 0) {
            player->inventory->clearSlot(index);
        }
    } else if (loc == SLOT_LOC_INVENTORY) {
        int realSlot = Inventory::MAX_SELECTION_SIZE + index;
        ItemInstance* item = player->inventory->getItem(realSlot);
        if (!item || item->isNull()) return;

        moveStackToRange(item, 0, Inventory::MAX_SELECTION_SIZE);
        if (item->count <= 0) {
            player->inventory->clearSlot(realSlot);
        }
    } else if (loc == SLOT_LOC_CRAFT_INPUT) {
        ItemInstance* item = craftInputSlots[index];
        if (!item || item->isNull()) return;

        moveStackToRange(item, Inventory::MAX_SELECTION_SIZE, player->inventory->getContainerSize());
        if (item->count > 0) {
            moveStackToRange(item, 0, Inventory::MAX_SELECTION_SIZE);
        }
        if (item->count <= 0) {
            delete craftInputSlots[index];
            craftInputSlots[index] = NULL;
            updateCraftingResult();
        }
    } else if (loc == SLOT_LOC_ARMOR) {
        ItemInstance* item = player->getArmor(index);
        if (!item || item->isNull()) return;

        ItemInstance* copy = new ItemInstance(*item);
        moveStackToRange(copy, Inventory::MAX_SELECTION_SIZE, player->inventory->getContainerSize());
        if (copy->count > 0) {
            moveStackToRange(copy, 0, Inventory::MAX_SELECTION_SIZE);
        }
        if (copy->count <= 0) {
            player->setArmor(index, NULL);
            delete copy;
        } else {
            player->setArmor(index, copy);
        }
    } else if (loc == SLOT_LOC_CATALOG && isCreative) {
        if (index >= 0 && index < static_cast<int>(catalogItems.size())) {
            const ItemInstance* catItem = catalogItems[index];
            if (catItem) {
                ItemInstance* copy = new ItemInstance(catItem->id, catItem->getMaxStackSize(), catItem->getAuxValue());
                moveStackToRange(copy, 0, Inventory::MAX_SELECTION_SIZE);
                if (copy->count > 0) {
                    moveStackToRange(copy, Inventory::MAX_SELECTION_SIZE, player->inventory->getContainerSize());
                }
                delete copy;
            }
        }
    }
}

void UnifiedInventoryScreen::swapInventorySlots(int slotA, int slotB) {
    if (!player || !player->inventory || slotA == slotB) return;

    ItemInstance* itemA = player->inventory->getItem(slotA);
    ItemInstance* itemB = player->inventory->getItem(slotB);

    ItemInstance* copyA = (itemA && !itemA->isNull()) ? new ItemInstance(*itemA) : NULL;
    ItemInstance* copyB = (itemB && !itemB->isNull()) ? new ItemInstance(*itemB) : NULL;

    player->inventory->clearSlot(slotA);
    player->inventory->clearSlot(slotB);

    if (copyB) player->inventory->setItem(slotA, copyB);
    if (copyA) player->inventory->setItem(slotB, copyA);
}

void UnifiedInventoryScreen::dropFromSlot(SlotLocation loc, int index, bool entireStack) {
    if (!player || !player->inventory) return;

    ItemInstance* target = getSlotItem(loc, index);
    if (!target || target->isNull() || target->count <= 0) return;

    int countToDrop = entireStack ? target->count : 1;
    ItemInstance* toDrop = new ItemInstance(*target);
    toDrop->count = countToDrop;

    target->count -= countToDrop;
    if (target->count <= 0) {
        clearSlot(loc, index);
    }

    player->drop(toDrop, false);
    if (player->abilities.instabuild) {
        delete toDrop;
    }
}

void UnifiedInventoryScreen::dropCarried(bool entireStack) {
    if (!player || !carriedItem || carriedItem->isNull() || carriedItem->count <= 0) return;

    int countToDrop = entireStack ? carriedItem->count : 1;
    ItemInstance* toDrop = new ItemInstance(*carriedItem);
    toDrop->count = countToDrop;

    carriedItem->count -= countToDrop;
    if (carriedItem->count <= 0) {
        delete carriedItem;
        carriedItem = NULL;
    }

    player->drop(toDrop, false);
    if (player->abilities.instabuild) {
        delete toDrop;
    }
}

bool UnifiedInventoryScreen::isOutsideInventoryPanels(int x, int y) {
    bool inRight = (x >= rightPanelRect.x && x < rightPanelRect.x + rightPanelRect.w &&
                    y >= rightPanelRect.y && y < rightPanelRect.y + rightPanelRect.h);
    bool inLeft = (isDualPane && x >= leftPanelRect.x && x < leftPanelRect.x + leftPanelRect.w &&
                   y >= leftPanelRect.y && y < leftPanelRect.y + leftPanelRect.h);
    bool inClose = (x >= btnClose.x && x < btnClose.x + btnClose.width &&
                    y >= btnClose.y && y < btnClose.y + btnClose.height);
    return !inRight && !inLeft && !inClose;
}

void UnifiedInventoryScreen::creativeClone(SlotLocation loc, int index) {
    if (!isCreative || !player) return;
    ItemInstance* source = getSlotItem(loc, index);
    if (!source || source->isNull()) return;

    delete carriedItem;
    carriedItem = new ItemInstance(source->id, source->getMaxStackSize(), source->getAuxValue());
}

void UnifiedInventoryScreen::collectMatching(SlotLocation loc, int index) {
    if (!player || !player->inventory || !carriedItem || carriedItem->isNull()) return;

    int maxStack = std::min(carriedItem->getMaxStackSize(), player->inventory->getMaxStackSize());
    if (carriedItem->count >= maxStack) return;

    // 1. Scan Main Inventory (9..35)
    for (int i = Inventory::MAX_SELECTION_SIZE; i < player->inventory->getContainerSize() && carriedItem->count < maxStack; ++i) {
        ItemInstance* item = player->inventory->getItem(i);
        if (!item || item->isNull()) continue;
        if (!ItemInstance::isStackable(item, carriedItem)) continue;

        int needed = maxStack - carriedItem->count;
        int take = std::min(needed, item->count);
        carriedItem->count += take;
        item->count -= take;
        if (item->count <= 0) {
            player->inventory->clearSlot(i);
        }
    }

    // 2. Scan Hotbar (0..8)
    for (int i = 0; i < Inventory::MAX_SELECTION_SIZE && carriedItem->count < maxStack; ++i) {
        ItemInstance* item = player->inventory->getItem(i);
        if (!item || item->isNull()) continue;
        if (!ItemInstance::isStackable(item, carriedItem)) continue;

        int needed = maxStack - carriedItem->count;
        int take = std::min(needed, item->count);
        carriedItem->count += take;
        item->count -= take;
        if (item->count <= 0) {
            player->inventory->clearSlot(i);
        }
    }
}

bool UnifiedInventoryScreen::containsDragSlot(SlotLocation loc, int index) const {
    for (size_t i = 0; i < dragSlots.size(); ++i) {
        if (dragSlots[i].location == loc && dragSlots[i].index == index)
            return true;
    }
    return false;
}

void UnifiedInventoryScreen::executeDragDistribution() {
    if (dragSlots.empty() || !player || !player->inventory) return;

    if (dragMode == DRAG_CREATIVE_MIDDLE && isCreative && carriedItem && !carriedItem->isNull()) {
        for (size_t i = 0; i < dragSlots.size(); ++i) {
            if (canPlaceInSlot(dragSlots[i], carriedItem)) {
                ItemInstance* clone = new ItemInstance(carriedItem->id, carriedItem->getMaxStackSize(), carriedItem->getAuxValue());
                setSlotItem(dragSlots[i], clone);
            }
        }
    } else if (dragMode == DRAG_RIGHT && carriedItem && carriedItem->count > 0) {
        for (size_t i = 0; i < dragSlots.size() && carriedItem->count > 0; ++i) {
            ItemInstance* target = getSlotItem(dragSlots[i]);
            if (!target || target->isNull()) {
                ItemInstance* one = new ItemInstance(*carriedItem);
                one->count = 1;
                setSlotItem(dragSlots[i], one);
                carriedItem->count--;
            } else if (ItemInstance::isStackable(target, carriedItem)) {
                int maxStack = std::min(target->getMaxStackSize(), player->inventory->getMaxStackSize());
                if (target->count < maxStack) {
                    target->count++;
                    carriedItem->count--;
                }
            }
        }
        if (carriedItem && carriedItem->count <= 0) {
            delete carriedItem;
            carriedItem = NULL;
        }
    } else if (dragMode == DRAG_LEFT && carriedItem && carriedItem->count > 0) {
        int remaining = carriedItem->count;
        while (remaining > 0) {
            int validCount = 0;
            for (size_t i = 0; i < dragSlots.size(); ++i) {
                if (getSlotCapacity(dragSlots[i], carriedItem) > 0)
                    validCount++;
            }
            if (validCount == 0) break;

            int amount = std::max(1, remaining / validCount);
            bool changed = false;

            for (size_t i = 0; i < dragSlots.size() && remaining > 0; ++i) {
                int cap = getSlotCapacity(dragSlots[i], carriedItem);
                if (cap <= 0) continue;

                int toAdd = std::min(amount, cap);
                toAdd = std::min(toAdd, remaining);

                ItemInstance* target = getSlotItem(dragSlots[i]);
                if (!target || target->isNull()) {
                    ItemInstance* placed = new ItemInstance(*carriedItem);
                    placed->count = toAdd;
                    setSlotItem(dragSlots[i], placed);
                } else {
                    target->count += toAdd;
                }

                remaining -= toAdd;
                changed = true;
            }

            if (!changed) break;
        }

        carriedItem->count = remaining;
        if (carriedItem->count <= 0) {
            delete carriedItem;
            carriedItem = NULL;
        }
    }
}

void UnifiedInventoryScreen::mouseClicked(int x, int y, int buttonNum) {
    super::mouseClicked(x, y, buttonNum);

    if (btnClose.clicked(minecraft, x, y)) {
        if (minecraft->soundEngine) {
            minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
        }
        minecraft->setScreen(NULL);
        return;
    }

    if (isCreative && btnModeDual.clicked(minecraft, x, y)) {
        if (!isDualPane) {
            if (minecraft->soundEngine) {
                minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
            }
            isDualPane = true;
            setupPositions();
        }
        return;
    }

    if (isCreative && btnModeSingle.clicked(minecraft, x, y)) {
        if (isDualPane) {
            if (minecraft->soundEngine) {
                minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
            }
            isDualPane = false;
            setupPositions();
        }
        return;
    }

    if (isCreative && isDualPane) {
        for (size_t c = 0; c < categoryButtons.size(); ++c) {
            if (categoryButtons[c]->clicked(minecraft, x, y)) {
                if (minecraft->soundEngine) {
                    minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
                }
                buttonClicked(categoryButtons[c]);
                return;
            }
        }
    }

    SlotLocation loc = SLOT_LOC_NONE;
    int index = -1;
    bool onSlot = getSlotAt(x, y, loc, index);

    bool shiftDown = Keyboard::isKeyDown(Keyboard::KEY_LSHIFT);

    if (buttonNum == MouseAction::ACTION_MIDDLE && isCreative && onSlot) {
        creativeClone(loc, index);
        dragMode = DRAG_CREATIVE_MIDDLE;
        isDragging = true;
        dragSlots.clear();
        dragSlots.push_back({loc, index});
        if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
        return;
    }

    if (buttonNum == MouseAction::ACTION_RIGHT) {
        if (onSlot) {
            handleRightClick(loc, index);
            dragMode = DRAG_RIGHT;
            isDragging = true;
            dragSlots.clear();
            dragSlots.push_back({loc, index});
            if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
        } else if (isOutsideInventoryPanels(x, y)) {
            dropCarried(false);
            if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.pop", 1.0f, 1.0f);
        }
        return;
    }

    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (onSlot) {
            if (shiftDown) {
                quickMove(loc, index);
                if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
            } else {
                uint64_t nowMs = static_cast<uint64_t>(getTimeMs());
                bool isDoubleClick = (nowMs - lastClickTimeMs < 300) && (loc == lastClickLoc) && (index == lastClickIndex) && (buttonNum == lastClickButton);

                if (isDoubleClick && carriedItem != NULL) {
                    collectMatching(loc, index);
                } else {
                    handleLeftClick(loc, index);
                    dragMode = DRAG_LEFT;
                    isDragging = true;
                    dragSlots.clear();
                    dragSlots.push_back({loc, index});
                }

                lastClickTimeMs = nowMs;
                lastClickLoc = loc;
                lastClickIndex = index;
                lastClickButton = buttonNum;
                if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
            }
        } else if (isCreative && isDualPane && carriedItem != NULL &&
                   x >= leftPanelRect.x && x < leftPanelRect.x + leftPanelRect.w &&
                   y >= leftPanelRect.y && y < leftPanelRect.y + leftPanelRect.h) {
            // Delete zone on creative catalog pane
            delete carriedItem;
            carriedItem = NULL;
            if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.pop", 1.0f, 1.0f);
        } else if (isOutsideInventoryPanels(x, y)) {
            dropCarried(true);
            if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.pop", 1.0f, 1.0f);
        }
    }
}

void UnifiedInventoryScreen::handleSlotInteraction(SlotLocation loc, int index, int buttonNum) {
    if (buttonNum == MouseAction::ACTION_RIGHT) {
        handleRightClick(loc, index);
    } else {
        handleLeftClick(loc, index);
    }
}

void UnifiedInventoryScreen::updateCraftingResult() {
    delete craftResultItem;
    craftResultItem = NULL;

    int count = 0;
    int firstSlot = -1;
    for (int i = 0; i < 4; ++i) {
        if (craftInputSlots[i] && !craftInputSlots[i]->isNull()) {
            count++;
            if (firstSlot == -1) firstSlot = i;
        }
    }

    if (count == 1 && firstSlot != -1) {
        ItemInstance* item = craftInputSlots[firstSlot];
        if (item->id == Tile::treeTrunk->id) {
            craftResultItem = new ItemInstance(Tile::wood, 4, item->getAuxValue());
        } else if (item->id == Tile::cloth->id) {
            craftResultItem = new ItemInstance(Item::string, 4);
        }
    } else if (count == 2) {
        if (craftInputSlots[0] && craftInputSlots[2] &&
            craftInputSlots[0]->id == Tile::wood->id && craftInputSlots[2]->id == Tile::wood->id) {
            craftResultItem = new ItemInstance(Item::stick, 4);
        } else if (((craftInputSlots[0] && craftInputSlots[2] && craftInputSlots[0]->id == Item::coal->id && craftInputSlots[2]->id == Item::stick->id) ||
                    (craftInputSlots[1] && craftInputSlots[3] && craftInputSlots[1]->id == Item::coal->id && craftInputSlots[3]->id == Item::stick->id))) {
            craftResultItem = new ItemInstance(Tile::torch, 4);
        }
    } else if (count == 4) {
        if (craftInputSlots[0] && craftInputSlots[1] && craftInputSlots[2] && craftInputSlots[3] &&
            craftInputSlots[0]->id == Tile::wood->id && craftInputSlots[1]->id == Tile::wood->id &&
            craftInputSlots[2]->id == Tile::wood->id && craftInputSlots[3]->id == Tile::wood->id) {
            craftResultItem = new ItemInstance(Tile::workBench, 1);
        } else if (craftInputSlots[0] && craftInputSlots[1] && craftInputSlots[2] && craftInputSlots[3] &&
                   craftInputSlots[0]->id == Tile::sand->id && craftInputSlots[1]->id == Tile::sand->id &&
                   craftInputSlots[2]->id == Tile::sand->id && craftInputSlots[3]->id == Tile::sand->id) {
            craftResultItem = new ItemInstance(Tile::sandStone, 1);
        }
    }
}

void UnifiedInventoryScreen::mouseReleased(int x, int y, int buttonNum) {
    super::mouseReleased(x, y, buttonNum);

    if (isDragging && dragSlots.size() > 1) {
        executeDragDistribution();
        if (minecraft->soundEngine) {
            minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
        }
    }

    dragMode = DRAG_NONE;
    isDragging = false;
    dragSlots.clear();
}

void UnifiedInventoryScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (isCreative && isDualPane) {
        catalogScrollY -= dy * 18.0f;
        catalogScrollY = std::max(0.0f, std::min(catalogScrollY, maxCatalogScrollY));
    }
}

void UnifiedInventoryScreen::removed() {
    super::removed();

    // Return any held items or crafting input items safely
    if (player && player->inventory) {
        if (carriedItem && !carriedItem->isNull() && carriedItem->count > 0) {
            if (!isCreative) {
                player->inventory->add(carriedItem);
                if (carriedItem->count > 0) {
                    player->drop(carriedItem, false);
                } else {
                    delete carriedItem;
                }
            } else {
                delete carriedItem;
            }
            carriedItem = NULL;
        }

        for (int i = 0; i < 4; ++i) {
            if (craftInputSlots[i] && !craftInputSlots[i]->isNull() && craftInputSlots[i]->count > 0) {
                if (!isCreative) {
                    player->inventory->add(craftInputSlots[i]);
                    if (craftInputSlots[i]->count > 0) {
                        player->drop(craftInputSlots[i], false);
                    } else {
                        delete craftInputSlots[i];
                    }
                } else {
                    delete craftInputSlots[i];
                }
                craftInputSlots[i] = NULL;
            }
        }
    }
}

void UnifiedInventoryScreen::keyPressed(int eventKey) {
    if (eventKey == 27 || eventKey == 69 || eventKey == 101 || eventKey == Keyboard::KEY_ESCAPE || eventKey == Keyboard::KEY_E) { // ESC, E, e
        minecraft->setScreen(NULL);
        return;
    }

    // 1-9 Hotbar Swapping
    if (eventKey >= Keyboard::KEY_1 && eventKey <= Keyboard::KEY_9) {
        int hotbarTarget = eventKey - Keyboard::KEY_1; // 0..8
        SlotLocation hoveredLoc;
        int hoveredIndex;
        if (getSlotAt(lastMouseX, lastMouseY, hoveredLoc, hoveredIndex)) {
            if (hoveredLoc == SLOT_LOC_INVENTORY) {
                int realSlot = Inventory::MAX_SELECTION_SIZE + hoveredIndex;
                swapInventorySlots(realSlot, hotbarTarget);
                if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
                return;
            } else if (hoveredLoc == SLOT_LOC_HOTBAR) {
                swapInventorySlots(hoveredIndex, hotbarTarget);
                if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
                return;
            } else if (hoveredLoc == SLOT_LOC_CRAFT_INPUT) {
                ItemInstance* inputItem = craftInputSlots[hoveredIndex];
                ItemInstance* hotItem = player->inventory->getItem(hotbarTarget);

                ItemInstance* copyInput = (inputItem && !inputItem->isNull()) ? new ItemInstance(*inputItem) : NULL;
                ItemInstance* copyHot = (hotItem && !hotItem->isNull()) ? new ItemInstance(*hotItem) : NULL;

                delete craftInputSlots[hoveredIndex];
                craftInputSlots[hoveredIndex] = copyHot;
                updateCraftingResult();

                player->inventory->clearSlot(hotbarTarget);
                if (copyInput) player->inventory->setItem(hotbarTarget, copyInput);

                if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
                return;
            }
        }
    }

    // Q / Ctrl+Q Dropping from hovered slot
    if (eventKey == Keyboard::KEY_Q || eventKey == 81 || eventKey == 113) {
        SlotLocation hoveredLoc;
        int hoveredIndex;
        if (getSlotAt(lastMouseX, lastMouseY, hoveredLoc, hoveredIndex)) {
            bool entireStack = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL);
            dropFromSlot(hoveredLoc, hoveredIndex, entireStack);
            if (minecraft->soundEngine) minecraft->soundEngine->playUI("random.pop", 1.0f, 1.0f);
            return;
        }
    }

    super::keyPressed(eventKey);
}
