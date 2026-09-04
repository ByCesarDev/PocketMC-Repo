#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ClassicChestScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ClassicChestScreen_H__

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
class ChestTileEntity;
class FillingContainer;

enum ClassicChestSlotLocation {
    CLASSIC_CHEST_SLOT_NONE,
    CLASSIC_CHEST_SLOT_CHEST,       // 0..26
    CLASSIC_CHEST_SLOT_INVENTORY,   // 9..35
    CLASSIC_CHEST_SLOT_HOTBAR       // 0..8
};

struct ClassicChestSlotRef {
    ClassicChestSlotLocation location;
    int index;

    bool operator==(const ClassicChestSlotRef& other) const {
        return location == other.location && index == other.index;
    }
};

enum ClassicChestDragMode {
    CLASSIC_CHEST_DRAG_NONE,
    CLASSIC_CHEST_DRAG_LEFT,
    CLASSIC_CHEST_DRAG_RIGHT
};

class ClassicChestScreen : public Screen
{
    typedef Screen super;
public:
    static const int CHEST_COLS = 9;
    static const int CHEST_ROWS = 3;
    static const int CHEST_SIZE = 27;
    static const int INV_ROWS = 3;
    static const int INV_COLS = 9;
    static const int HOTBAR_COLS = 9;
    static const int SLOT_SIZE = 18;

    ClassicChestScreen(Player* player, ChestTileEntity* chest);
    virtual ~ClassicChestScreen();

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
    bool getSlotAt(int x, int y, ClassicChestSlotLocation& outLoc, int& outIndex);

    // Slot manipulation helpers
    ItemInstance* getSlotItem(ClassicChestSlotLocation loc, int index);
    ItemInstance* getSlotItem(const ClassicChestSlotRef& slot) { return getSlotItem(slot.location, slot.index); }
    void setSlotItem(ClassicChestSlotLocation loc, int index, ItemInstance* item);
    void setSlotItem(const ClassicChestSlotRef& slot, ItemInstance* item) { setSlotItem(slot.location, slot.index, item); }
    void clearSlot(ClassicChestSlotLocation loc, int index);
    void clearSlot(const ClassicChestSlotRef& slot) { clearSlot(slot.location, slot.index); }
    int getSlotCapacity(ClassicChestSlotLocation loc, int index, const ItemInstance* item);
    int getSlotCapacity(const ClassicChestSlotRef& slot, const ItemInstance* item) { return getSlotCapacity(slot.location, slot.index, item); }
    bool canPlaceInSlot(ClassicChestSlotLocation loc, int index, const ItemInstance* item);

    // Advanced Java mouse actions
    void handleLeftClick(ClassicChestSlotLocation loc, int index);
    void handleRightClick(ClassicChestSlotLocation loc, int index);
    void quickMove(ClassicChestSlotLocation loc, int index);
    bool moveStackToChest(ItemInstance*& source);
    bool moveStackToPlayerInv(ItemInstance*& source);
    bool moveStackToRange(FillingContainer* container, ItemInstance*& source, int begin, int end);
    void swapInventorySlots(int slotA, int slotB);
    void dropFromSlot(ClassicChestSlotLocation loc, int index, bool entireStack);
    void dropCarried(bool entireStack);
    bool isOutsidePanel(int x, int y);
    void collectMatching(ClassicChestSlotLocation loc, int index);
    void executeDragDistribution();
    bool containsDragSlot(ClassicChestSlotLocation loc, int index) const;

    Player* player;
    ChestTileEntity* chest;
    BlankButton btnClose;

    NinePatchLayer* guiPanelBg;
    NinePatchLayer* guiSlot;

    int panelX, panelY, panelWidth, panelHeight;
    int chestGridX, chestGridY;
    int invX, invY;
    int hotbarX, hotbarY;

    ItemInstance* carriedItem;

    // Mouse drag distribution
    ClassicChestDragMode dragMode;
    std::vector<ClassicChestSlotRef> draggedSlots;
    int dragItemOrigCount;

    // Double-click tracking
    int lastClickedButton;
    int lastClickTime;
    ClassicChestSlotRef lastClickedSlot;
    int lastMouseX;
    int lastMouseY;
    bool chestWasOpened;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ClassicChestScreen_H__*/
