#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ClassicCraftingScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ClassicCraftingScreen_H__

#include <string>
#include <vector>
#include <cstddef>
#include "../Screen.h"
#include "../components/ImageButton.h"
#include "../../../world/item/ItemInstance.h"

class Font;
class Textures;
class NinePatchLayer;
class Tesselator;
class Player;

enum ClassicSlotLocation {
    CLASSIC_SLOT_NONE,
    CLASSIC_SLOT_INVENTORY,
    CLASSIC_SLOT_HOTBAR,
    CLASSIC_SLOT_CRAFT_INPUT,
    CLASSIC_SLOT_CRAFT_RESULT
};

struct ClassicSlotRef {
    ClassicSlotLocation location;
    int index;

    bool operator==(const ClassicSlotRef& other) const {
        return location == other.location && index == other.index;
    }
};

enum ClassicDragMode {
    CLASSIC_DRAG_NONE,
    CLASSIC_DRAG_LEFT,
    CLASSIC_DRAG_RIGHT
};

class ClassicCraftingScreen : public Screen
{
    typedef Screen super;
public:
    static const int CRAFT_GRID_COLS = 3;
    static const int CRAFT_GRID_ROWS = 3;
    static const int CRAFT_GRID_SIZE = 9;
    static const int INV_ROWS = 3;
    static const int INV_COLS = 9;
    static const int HOTBAR_COLS = 9;
    static const int SLOT_SIZE = 18;

    ClassicCraftingScreen();
    virtual ~ClassicCraftingScreen();

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
    void renderCarriedItem(int xm, int ym);
    void drawInventorySlot(Tesselator& t, int x, int y, const ItemInstance* item, bool isHovered = false, bool isDragTarget = false);
    void drawResultSlot(Tesselator& t, int x, int y, const ItemInstance* item, bool isHovered = false);
    bool getSlotAt(int x, int y, ClassicSlotLocation& outLoc, int& outIndex);
    void updateCraftingResult();
    void consumeCraftingIngredients();

    // Slot manipulation helpers
    ItemInstance* getSlotItem(ClassicSlotLocation loc, int index);
    ItemInstance* getSlotItem(const ClassicSlotRef& slot) { return getSlotItem(slot.location, slot.index); }
    void setSlotItem(ClassicSlotLocation loc, int index, ItemInstance* item);
    void setSlotItem(const ClassicSlotRef& slot, ItemInstance* item) { setSlotItem(slot.location, slot.index, item); }
    void clearSlot(ClassicSlotLocation loc, int index);
    void clearSlot(const ClassicSlotRef& slot) { clearSlot(slot.location, slot.index); }
    int getSlotCapacity(ClassicSlotLocation loc, int index, const ItemInstance* item);
    int getSlotCapacity(const ClassicSlotRef& slot, const ItemInstance* item) { return getSlotCapacity(slot.location, slot.index, item); }
    bool canPlaceInSlot(ClassicSlotLocation loc, int index, const ItemInstance* item);

    // Advanced Java mouse actions
    void handleLeftClick(ClassicSlotLocation loc, int index);
    void handleRightClick(ClassicSlotLocation loc, int index);
    void quickMove(ClassicSlotLocation loc, int index);
    bool moveStackToRange(ItemInstance*& source, int begin, int end);
    void swapInventorySlots(int slotA, int slotB);
    void dropFromSlot(ClassicSlotLocation loc, int index, bool entireStack);
    void dropCarried(bool entireStack);
    bool isOutsidePanel(int x, int y);
    void collectMatching(ClassicSlotLocation loc, int index);
    void executeDragDistribution();
    bool containsDragSlot(ClassicSlotLocation loc, int index) const;

    Player* player;
    BlankButton btnClose;

    NinePatchLayer* guiPanelBg;
    NinePatchLayer* guiSlot;
    NinePatchLayer* guiSlotResult;

    int panelX, panelY, panelWidth, panelHeight;
    int craftGridX, craftGridY;
    int arrowX, arrowY;
    int outputX, outputY;
    int invX, invY;
    int hotbarX, hotbarY;

    ItemInstance* craftSlots[CRAFT_GRID_SIZE];
    ItemInstance* craftResult;
    ItemInstance* carriedItem;

    // Mouse drag distribution
    ClassicDragMode dragMode;
    std::vector<ClassicSlotRef> draggedSlots;
    int dragItemOrigCount;

    // Double-click tracking
    int lastClickedButton;
    int lastClickTime;
    ClassicSlotRef lastClickedSlot;
    int lastMouseX;
    int lastMouseY;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ClassicCraftingScreen_H__*/
