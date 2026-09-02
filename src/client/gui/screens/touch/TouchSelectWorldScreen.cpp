#include "TouchSelectWorldScreen.h"
#include "../StartMenuScreen.h"
#include "../ProgressScreen.h"
#include "../DialogDefinitions.h"
#include "../SimpleChooseLevelScreen.h"
#include "../EditWorldScreen.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/Textures.h"
#include "../../../../world/level/LevelSettings.h"
#include "../../../../AppPlatform.h"
#include "../../../../util/StringUtils.h"
#include "../../../../util/Mth.h"
#include "../../../../platform/input/Mouse.h"
#include "../../../../locale/I18n.h"

#include <algorithm>
#include <set>

namespace Touch {

// ─── helpers ────────────────────────────────────────────────────────────────

static bool strContainsCITouch(const std::string& haystack, const std::string& needle)
{
    if (needle.empty()) return true;
    std::string h = haystack, n = needle;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n) != std::string::npos;
}

// ─── SelectWorldScreen ──────────────────────────────────────────────────────

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
        listX,              // x
        45,                 // y (started at 45 to leave room for search box)
        listW,              // width
        height - 45 - 64 - 4  // height: leave room for buttons
    );

    loadLevels();

    // Search textbox
    tSearch.x      = listX;
    tSearch.y      = 22;
    tSearch.width  = listW;
    tSearch.height = 18;
    tSearch.active = true;
    tSearch.visible= true;
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

    const int BW  = 150;
    const int BH  = 20;
    const int CX  = width / 2;
    const int row1Y = height - 52;
    const int row2Y = height - 28;
    const int GAP   = 4;

    // Row 1: Play + Create
    bPlay.x    = CX - BW - GAP/2;  bPlay.y    = row1Y;
    bPlay.width = BW; bPlay.height = BH;

    bCreate.x  = CX + GAP/2;       bCreate.y  = row1Y;
    bCreate.width = BW; bCreate.height = BH;

    // Row 2: Edit + Delete + Re-Create + Cancel
    const int BW2 = 70;
    int x2 = CX - BW2 * 2 - GAP * 3 / 2;
    bEdit.x      = x2;                  bEdit.y      = row2Y; bEdit.width = BW2; bEdit.height = BH;
    bDelete.x    = x2 + BW2 + GAP;      bDelete.y    = row2Y; bDelete.width = BW2; bDelete.height = BH;
    bRecreate.x  = x2 + (BW2+GAP)*2;   bRecreate.y  = row2Y; bRecreate.width = BW2; bRecreate.height = BH;
    bCancel.x    = x2 + (BW2+GAP)*3;   bCancel.y    = row2Y; bCancel.width = BW2; bCancel.height = BH;
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
        minecraft->setScreen(new TouchDeleteWorldScreen(lvl));
        return;
    }
    if (button->id == bRecreate.id && worldList->hasSelection()) {
        const LevelSummary& lvl = worldList->selectedLevel();
        std::string name = getUniqueLevelName(lvl.name);
        minecraft->setScreen(new SimpleChooseLevelScreen(name));
        return;
    }
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
    for (TextBox* tb : textBoxes)
        if (tb->focused)
            tb->charPressed(minecraft, c);

    worldList->setFilter(tSearch.text);
}

void SelectWorldScreen::render(int xm, int ym, float a)
{
    renderBackground();   // panorama

    // Title
    drawCenteredString(minecraft->font, I18n::get("selectWorld.title").c_str(),
                       width / 2, 8, 0xFFFFFFFF);

    // Search label
    drawString(minecraft->font, (I18n::get("selectWorld.search") + ":").c_str(),
               tSearch.x, tSearch.y - 10, 0xFFFFFFFF);

    // World list
    worldList->render(xm, ym, a);

    // Buttons + search textbox
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

// ─── TouchDeleteWorldScreen ─────────────────────────────────────────────────

TouchDeleteWorldScreen::TouchDeleteWorldScreen(const LevelSummary& level)
    : ConfirmScreen(nullptr,
                    "Are you sure you want to delete this world?",
                    "'" + level.name + "' will be lost forever!",
                    "Delete", "Cancel", 0),
      _level(level)
{
    tabButtonIndex = 1;
}

void TouchDeleteWorldScreen::postResult(bool isOk)
{
    if (isOk) {
        LevelStorageSource* src = minecraft->getLevelSource();
        src->deleteLevel(_level.id);
    }
    minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
}

} // namespace Touch
