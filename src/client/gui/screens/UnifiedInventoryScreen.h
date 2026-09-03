#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__UnifiedInventoryScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__UnifiedInventoryScreen_H__

#include <string>
#include <vector>
#include <cstddef>
#include "../Screen.h"
#include "../components/Button.h"
#include "../components/ImageButton.h"
#include "../components/TextBox.h"
#include "../../../world/item/ItemInstance.h"

class Font;
class Textures;
class NinePatchLayer;
class Tesselator;
class Player;

namespace Touch {
    class UnifiedCategoryButton;
}

enum SlotLocation {
    SLOT_LOC_NONE,
    SLOT_LOC_CATALOG,
    SLOT_LOC_INVENTORY,
    SLOT_LOC_HOTBAR,
    SLOT_LOC_ARMOR,
    SLOT_LOC_CRAFT_INPUT,
    SLOT_LOC_CRAFT_RESULT
};

struct SlotRef {
    SlotLocation location;
    int index;

    bool operator==(const SlotRef& other) const {
        return location == other.location && index == other.index;
    }
};

enum InventoryDragMode {
    DRAG_NONE,
    DRAG_LEFT,
    DRAG_RIGHT,
    DRAG_CREATIVE_MIDDLE
};

class UnifiedInventoryScreen : public Screen
{
    typedef Screen super;
public:
    static const int NUM_ARMOR = 4;
    static const int INV_ROWS = 3;
    static const int INV_COLS = 9;
    static const int HOTBAR_COLS = 9;
    static const int CAT_COLS = 7;
    static const int NUM_CATEGORIES = 5;

    UnifiedInventoryScreen();
    virtual ~UnifiedInventoryScreen();

    void init() override;
    void setupPositions() override;
    void tick() override;
    void render(int xm, int ym, float a) override;
    void renderHoverTooltip(int xm, int ym) override;
    bool renderGameBehind() override;
    void buttonClicked(Button* button) override;
    void removed() override;

protected:
    void mouseClicked(int x, int y, int buttonNum) override;
    void mouseReleased(int x, int y, int buttonNum) override;
    void mouseMoved(int x, int y, int dx, int dy) override;
    void mouseWheel(int dx, int dy, int xm, int ym) override;
    void keyPressed(int eventKey) override;

private:
    void renderPlayer(float xo, float yo, int xm, int ym);
    void renderRightPanel(Tesselator& t, int xm, int ym, float a);
    void renderLeftPanel(Tesselator& t, int xm, int ym, float a);
    void renderCarriedItem(int xm, int ym);
    void updateItems();
    void drawSlotItemAt(Tesselator& t, int slot, const ItemInstance* item, int x, int y, bool isArmorSlot = false, bool isHovered = false);

    bool getSlotAt(int x, int y, SlotLocation& outLoc, int& outIndex);
    void handleSlotInteraction(SlotLocation loc, int index, int buttonNum);
    void updateCraftingResult();

    // Slot Reference Helpers
    ItemInstance* getSlotItem(SlotLocation loc, int index);
    ItemInstance* getSlotItem(const SlotRef& slot) { return getSlotItem(slot.location, slot.index); }
    void setSlotItem(SlotLocation loc, int index, ItemInstance* item);
    void setSlotItem(const SlotRef& slot, ItemInstance* item) { setSlotItem(slot.location, slot.index, item); }
    void clearSlot(SlotLocation loc, int index);
    void clearSlot(const SlotRef& slot) { clearSlot(slot.location, slot.index); }
    int getSlotCapacity(SlotLocation loc, int index, const ItemInstance* item);
    int getSlotCapacity(const SlotRef& slot, const ItemInstance* item) { return getSlotCapacity(slot.location, slot.index, item); }
    bool canPlaceInSlot(SlotLocation loc, int index, const ItemInstance* item);
    bool canPlaceInSlot(const SlotRef& slot, const ItemInstance* item) { return canPlaceInSlot(slot.location, slot.index, item); }

    // Advanced Inventory Actions
    void handleLeftClick(SlotLocation loc, int index);
    void handleRightClick(SlotLocation loc, int index);
    void quickMove(SlotLocation loc, int index);
    bool moveStackToRange(ItemInstance*& source, int begin, int end);
    void swapInventorySlots(int slotA, int slotB);
    void dropFromSlot(SlotLocation loc, int index, bool entireStack);
    void dropCarried(bool entireStack);
    bool isOutsideInventoryPanels(int x, int y);
    void creativeClone(SlotLocation loc, int index);
    void collectMatching(SlotLocation loc, int index);
    void executeDragDistribution();
    bool containsDragSlot(SlotLocation loc, int index) const;

    bool isCreative;
    bool isDualPane;
    ImageButton btnClose;
    BlankButton btnModeDual;
    BlankButton btnModeSingle;

    BlankButton btnArmor[NUM_ARMOR];
    BlankButton btnCraftingTable;

    // Category Tabs (Creative)
    std::vector<ImageButton*> categoryButtons;
    ImageButton* selectedCategoryButton;
    int currentCategory;

    // Search bar
    TextBox searchBox;

    // Layout Rectangles
    IntRectangle rightPanelRect;
    IntRectangle leftPanelRect;
    IntRectangle guiPlayerBgRect;
    IntRectangle catalogPaneRect;
    IntRectangle headerBarRect;

    // Catalog items and scroll
    std::vector<ItemInstance*> catalogItems;
    float catalogScrollY;
    float maxCatalogScrollY;

    // 2x2 Crafting Grid slots
    ItemInstance* craftInputSlots[4];
    ItemInstance* craftResultItem;

    // Drag & Drop Cursor Carried Item
    ItemInstance* carriedItem;

    // Mouse Tracking & Drag Distribution
    int lastMouseX;
    int lastMouseY;
    InventoryDragMode dragMode;
    std::vector<SlotRef> dragSlots;
    bool isDragging;

    // Double-click Tracking
    uint64_t lastClickTimeMs;
    SlotLocation lastClickLoc;
    int lastClickIndex;
    int lastClickButton;

    NinePatchLayer* guiLeftPanelBg;
    NinePatchLayer* guiRightPanelBg;
    NinePatchLayer* guiSlot;
    NinePatchLayer* guiPaneFrame;
    NinePatchLayer* guiPlayerBg;
    NinePatchLayer* guiSlotCategory;
    NinePatchLayer* guiSlotCategorySelected;
    NinePatchLayer* guiDivider;
    NinePatchLayer* guiSrvInvBg;
    NinePatchLayer* guiTabSelected;
    NinePatchLayer* guiTabUnselected;

    Player* player;
    int selectedHotbarSlot;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__UnifiedInventoryScreen_H__*/
