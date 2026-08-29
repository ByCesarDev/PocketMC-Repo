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

    // Persistent Creative 27-slot buffer across inventory openings
    static ItemInstance* s_creativeExtraSlots[27];

    // 2x2 Crafting Grid slots
    ItemInstance* craftInputSlots[4];
    ItemInstance* craftResultItem;

    // Drag & Drop Cursor Carried Item
    ItemInstance* carriedItem;

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
