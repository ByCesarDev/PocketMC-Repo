#include "SelectWorldScreen.h"
#include "StartMenuScreen.h"
#include "ProgressScreen.h"
#include "SimpleChooseLevelScreen.h"
#include "EditWorldScreen.h"
#include "DialogDefinitions.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../../AppPlatform.h"
#include "../../../util/StringUtils.h"
#include "../../../util/Mth.h"
#include "../../../platform/input/Mouse.h"
#include "../../../world/level/LevelSettings.h"
#include "../../../locale/I18n.h"

#include <algorithm>
#include <set>
#include <sstream>

// â”€â”€â”€ helpers â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

static bool strContainsCI(const std::string& haystack, const std::string& needle)
{
    if (needle.empty()) return true;
    std::string h = haystack, n = needle;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n) != std::string::npos;
}

// â”€â”€â”€ WorldListWidget â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

WorldListWidget::WorldListWidget(Minecraft* mc, int x, int y, int w, int h)
    : _mc(mc), _x(x), _y(y), _w(w), _h(h),
      _selected(-1), _scrollOffset(0), _lastClickIndex(-1),
      _lastClickTime(std::chrono::steady_clock::now())
{}

void WorldListWidget::setBounds(int x, int y, int w, int h)
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    int maxScroll = std::max(0, (int)_filtered.size() * SLOT_H - _h);
    if (_scrollOffset > maxScroll) _scrollOffset = maxScroll;
}

void WorldListWidget::loadLevels(const LevelSummaryList& levels)
{
    _all = levels;
    rebuildFiltered();
}

void WorldListWidget::setFilter(const std::string& filter)
{
    _filter = filter;
    int prevSel = _selected;
    rebuildFiltered();
    // Try to keep selection valid
    if (prevSel >= (int)_filtered.size())
        _selected = _filtered.empty() ? -1 : 0;
}

void WorldListWidget::rebuildFiltered()
{
    _filtered.clear();
    for (const LevelSummary& s : _all)
        if (strContainsCI(s.name, _filter))
            _filtered.push_back(s);
    if (_selected >= (int)_filtered.size())
        _selected = _filtered.empty() ? -1 : (int)_filtered.size() - 1;
    // clamp scroll
    int maxScroll = std::max(0, (int)_filtered.size() * SLOT_H - _h);
    if (_scrollOffset > maxScroll) _scrollOffset = maxScroll;
}

void WorldListWidget::scroll(int delta)
{
    _scrollOffset -= delta;
    int maxScroll = std::max(0, (int)_filtered.size() * SLOT_H - _h);
    if (_scrollOffset < 0) _scrollOffset = 0;
    if (_scrollOffset > maxScroll) _scrollOffset = maxScroll;
}

bool WorldListWidget::mouseClicked(int xm, int ym, int btn)
{
    if (btn != 1) return false; // Only respond to left click
    if (xm < _x || xm > _x + _w) return false;
    if (ym < _y || ym > _y + _h) return false;

    int relY = ym - _y + _scrollOffset;
    int idx  = relY / SLOT_H;
    if (idx >= 0 && idx < (int)_filtered.size()) {
        const auto now = std::chrono::steady_clock::now();
        const int elapsedMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastClickTime).count();
        bool activate = false;
#if defined(ANDROID) || defined(__APPLE__)
        activate = true;
#else
        activate = (_selected == idx && _lastClickIndex == idx && elapsedMs > 0 && elapsedMs <= 500);
#endif
        _selected = idx;
        _lastClickIndex = idx;
        _lastClickTime = now;
        return activate;
    }
    return false;
}

void WorldListWidget::render(int xm, int ym, float /*a*/)
{
    // Dark panel background
    fill(_x, _y, _x + _w, _y + _h, 0x80000000);

    // clip by only drawing slots that overlap the widget area
    int startSlot = _scrollOffset / SLOT_H;
    int endSlot   = std::min((int)_filtered.size(),
                             startSlot + _h / SLOT_H + 2);

    for (int i = startSlot; i < endSlot; ++i) {
        int slotY = _y + i * SLOT_H - _scrollOffset;
        if (slotY + SLOT_H < _y || slotY > _y + _h) continue;
        drawSlot(i, slotY, i == _selected, xm, ym);
    }

    // thin top / bottom gradient to hint at scroll
    fillGradient(_x, _y,           _x + _w, _y + 4,  0x60000000, 0x00000000);
    fillGradient(_x, _y + _h - 4,  _x + _w, _y + _h, 0x00000000, 0x60000000);
}

void WorldListWidget::drawSlot(int idx, int slotY, bool isSelected, int /*xm*/, int /*ym*/)
{
    const LevelSummary& lvl = _filtered[idx];

    // Selection highlight
    if (isSelected) {
        fill(_x,     slotY,          _x + _w, slotY + SLOT_H, 0x60FFFFFF); // soft white border
        fill(_x + 1, slotY + 1,      _x + _w - 1, slotY + SLOT_H - 1, 0xC0222222);
    } else {
        // Subtle alternating row shading
        int shade = (idx % 2 == 0) ? 0x55000000 : 0x33000000;
        fill(_x, slotY, _x + _w, slotY + SLOT_H, shade);
    }

    // World thumbnail (32Ã—32 inside a 34Ã—34 border)
    const int THUMB = 32;
    const int imgX = _x + 2;
    const int imgY = slotY + (SLOT_H - THUMB) / 2;

    _mc->textures->loadAndBindTexture("gui/default_world.png");
    Tesselator& t = Tesselator::instance;
    t.begin();
    t.color(0xFFFFFFFF);
    t.vertexUV((float)imgX,        (float)(imgY + THUMB), blitOffset, 0, 1);
    t.vertexUV((float)(imgX+THUMB),(float)(imgY + THUMB), blitOffset, 1, 1);
    t.vertexUV((float)(imgX+THUMB),(float)imgY,           blitOffset, 1, 0);
    t.vertexUV((float)imgX,        (float)imgY,           blitOffset, 0, 0);
    t.draw();

    // Text
    int tx = _x + THUMB + 6;
    int ty = slotY + 3;
    drawString(_mc->font, lvl.name.c_str(), tx, ty, 0xFFFFFFFF);
    drawString(_mc->font, lvl.id.c_str(),   tx, ty + 10, 0xFFAAAAAA);
    drawString(_mc->font, LevelSettings::gameTypeToString(lvl.gameType).c_str(),
               tx, ty + 20, 0xFFAAAAAA);
}

// â”€â”€â”€ SelectWorldScreen â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

SelectWorldScreen::SelectWorldScreen()
    : bPlay    (1, I18n::get("selectWorld.select")),
      bCreate  (2, I18n::get("selectWorld.create")),
      bEdit    (3, I18n::get("selectWorld.edit")),
      bDelete  (4, I18n::get("selectWorld.delete")),
      bRecreate(5, I18n::get("selectWorld.recreate")),
      bCancel  (6, I18n::get("gui.cancel")),
      tSearch  (0, ""),
      worldList(nullptr),
      _hasStartedLevel(false)
{
    tSearch.hint = I18n::get("selectWorld.search");
}

SelectWorldScreen::~SelectWorldScreen()
{
    delete worldList;
}

void SelectWorldScreen::init()
{
    int listW = width - 60;  // leave 30px margin on each side
    if (listW > 400) listW = 400; // cap max width
    int listX = width / 2 - listW / 2;

    worldList = new WorldListWidget(minecraft,
        listX,                      // x
        45,                         // y (started at 45 to leave room for search box)
        listW,                      // width
        height - 45 - 64 - 4        // height: leave room for buttons
    );

    loadLevels();

    // Search textbox
    tSearch.x     = listX;
    tSearch.y     = 22;
    tSearch.width = listW;
    tSearch.height= 18;
    tSearch.active= true;
    tSearch.visible = true;
    textBoxes.push_back(&tSearch);

    buttons.push_back(&bPlay);
    buttons.push_back(&bCreate);
    buttons.push_back(&bEdit);
    buttons.push_back(&bDelete);
    buttons.push_back(&bRecreate);
    buttons.push_back(&bCancel);

    updateButtonStates();
}
 
void SelectWorldScreen::setupPositions()
{
    int listW = width - 60;
    if (listW > 400) listW = 400;
    if (listW < 220) listW = width - 12;
    int listX = width / 2 - listW / 2;

    tSearch.x      = listX;
    tSearch.y      = 22;
    tSearch.width  = listW;
    tSearch.height = 18;

    if (worldList)
        worldList->setBounds(listX, 45, listW, std::max(36, height - 45 - 64 - 4));

    // Row 1 (top button row): Play + Create
    const int BW = 150;
    const int BH = 20;
    const int CX = width / 2;
    const int row1Y = height - 52;
    const int row2Y = height - 28;
    const int GAP   = 4;

    bPlay.x    = CX - BW - GAP/2;   bPlay.y    = row1Y;
    bPlay.width = BW; bPlay.height = BH;

    bCreate.x  = CX + GAP/2;         bCreate.y  = row1Y;
    bCreate.width = BW; bCreate.height = BH;

    // Row 2 (bottom button row): Edit + Delete + Re-Create + Cancel
    const int BW2 = 70;
    int x2 = CX - BW2 * 2 - GAP * 3 / 2;
    bEdit.x      = x2;            bEdit.y      = row2Y; bEdit.width = BW2; bEdit.height = BH;
    bDelete.x    = x2 + BW2 + GAP; bDelete.y    = row2Y; bDelete.width = BW2; bDelete.height = BH;
    bRecreate.x  = x2 + (BW2 + GAP)*2; bRecreate.y = row2Y; bRecreate.width = BW2; bRecreate.height = BH;
    bCancel.x    = x2 + (BW2 + GAP)*3; bCancel.y   = row2Y; bCancel.width = BW2; bCancel.height = BH;
}

void SelectWorldScreen::tick()
{
    updateButtonStates();
}

void SelectWorldScreen::updateButtonStates()
{
    bool hasSel = worldList && worldList->hasSelection();
    bPlay.active     = hasSel;
    bEdit.active     = hasSel;
    bDelete.active   = hasSel;
    bRecreate.active = hasSel;
    bCreate.active   = !_hasStartedLevel;
}

bool SelectWorldScreen::handleBackEvent(bool isDown)
{
    if (!isDown) {
        minecraft->cancelLocateMultiplayer();
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
    }
    return true;
}

void SelectWorldScreen::buttonClicked(Button* button)
{
    if (button->id == bCancel.id) {
        minecraft->cancelLocateMultiplayer();
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        return;
    }
    if (button->id == bCreate.id && !_hasStartedLevel) {
        std::string name = getUniqueLevelName("world");
        minecraft->setScreen(new SimpleChooseLevelScreen(name));
        return;
    }
    if (button->id == bPlay.id && worldList->hasSelection()) {
        const LevelSummary& lvl = worldList->selectedLevel();
        minecraft->selectLevel(lvl.id, lvl.name, LevelSettings::None());
        minecraft->hostMultiplayer();
        minecraft->setScreen(new ProgressScreen());
        _hasStartedLevel = true;
        return;
    }
    if (button->id == bDelete.id && worldList->hasSelection()) {
        const LevelSummary& lvl = worldList->selectedLevel();
        minecraft->setScreen(new DeleteWorldScreen(lvl));
        return;
    }
    if (button->id == bRecreate.id && worldList->hasSelection()) {
        const LevelSummary& lvl = worldList->selectedLevel();
        std::string name = getUniqueLevelName(lvl.name);
        minecraft->setScreen(new SimpleChooseLevelScreen(name));
        return;
    }
    // Edit: open the new EditWorldScreen
    if (button->id == bEdit.id && worldList->hasSelection()) {
        const LevelSummary& lvl = worldList->selectedLevel();
        minecraft->setScreen(new EditWorldScreen(lvl));
        return;
    }
}

void SelectWorldScreen::keyPressed(int eventKey)
{
    Screen::keyPressed(eventKey);
}

void SelectWorldScreen::charPressed(char c)
{
    // forward to textbox, then update filter
    for (TextBox* tb : textBoxes)
        if (tb->focused)
            tb->charPressed(minecraft, c);

    worldList->setFilter(tSearch.text);
}

void SelectWorldScreen::render(int xm, int ym, float a)
{
    renderBackground();                         // panorama

    // Title
    drawCenteredString(minecraft->font, I18n::get("selectWorld.title").c_str(),
                       width / 2, 8, 0xFFFFFFFF);

    // Search box label
    drawString(minecraft->font, (I18n::get("selectWorld.search") + ":").c_str(),
               tSearch.x, tSearch.y - 10, 0xFFFFFFFF);

    // World list
    worldList->render(xm, ym, a);

    // Buttons + text boxes
    Screen::render(xm, ym, a);
}

void SelectWorldScreen::mouseWheel(int /*dx*/, int dy, int /*xm*/, int /*ym*/)
{
    if (worldList)
        worldList->scroll(dy * 10);
}

void SelectWorldScreen::loadLevels()
{
    LevelStorageSource* src = minecraft->getLevelSource();
    src->getLevelList(levels);
    std::sort(levels.begin(), levels.end());

    LevelSummaryList filtered;
    for (const LevelSummary& l : levels)
        if (l.id != LevelStorageSource::TempLevelId)
            filtered.push_back(l);

    worldList->loadLevels(filtered);
}

std::string SelectWorldScreen::getUniqueLevelName(const std::string& base)
{
    std::set<std::string> existing;
    for (const LevelSummary& l : levels)
        existing.insert(l.id);

    if (existing.find(base) == existing.end())
        return base;

    int count = 1;
    while (true) {
        std::string candidate = base + " (" + std::to_string(count) + ")";
        if (existing.find(candidate) == existing.end())
            return candidate;
        count++;
    }
}

bool SelectWorldScreen::isInGameScreen() { return true; }

void SelectWorldScreen::mouseClicked(int x, int y, int buttonNum)
{
    Screen::mouseClicked(x, y, buttonNum);
    if (worldList) {
        if (worldList->mouseClicked(x, y, buttonNum) && worldList->hasSelection())
            buttonClicked(&bPlay);
    }
}

// â”€â”€â”€ DeleteWorldScreen â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

DeleteWorldScreen::DeleteWorldScreen(const LevelSummary& level)
    : ConfirmScreen(nullptr,
                    "Are you sure you want to delete this world?",
                    "'" + level.name + "' will be lost forever!",
                    "Delete", "Cancel", 0),
      _level(level)
{
    tabButtonIndex = 1;
}

void DeleteWorldScreen::postResult(bool isOk)
{
    if (isOk) {
        LevelStorageSource* src = minecraft->getLevelSource();
        src->deleteLevel(_level.id);
    }
    minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
}
