#include "SimpleChooseLevelScreen.h"
#include "ProgressScreen.h"
#include "ScreenChooser.h"
#include "../components/Button.h"
#include "../components/ImageButton.h"
#include "../../Minecraft.h"
#include "../../../world/level/LevelSettings.h"
#include "../../../platform/time.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/log.h"
#include "../../../locale/I18n.h"

static std::string getWorldTypeName(int type) {
    std::string prefix = I18n::get("selectWorld.mapType") + ": ";
    switch (type) {
    case WorldType::INFINITE_SIZE: return prefix + I18n::get("selectWorld.mapType.infinite");
    case WorldType::FLAT:          return prefix + I18n::get("selectWorld.mapType.flat");
    case WorldType::POCKET:        return prefix + I18n::get("selectWorld.mapType.pocket");
    case WorldType::SMALL:         return prefix + I18n::get("selectWorld.mapType.small");
    case WorldType::LARGE:         return prefix + I18n::get("selectWorld.mapType.large");
    default:                       return prefix + I18n::get("selectWorld.mapType.infinite");
    }
}

SimpleChooseLevelScreen::SimpleChooseLevelScreen(const std::string& levelName)
:   bHeader(0),
    bGamemode(0),
    bCheats(0),
    bWorldType(0),
    bExperimental(0),
    bBack(0),
    bCreate(0),
    levelName(levelName),
    hasChosen(false),
    gamemode(GameType::Survival),
    cheatsEnabled(false),
    worldType(WorldType::INFINITE_SIZE),
    experimental(false),
    tLevelName(0, I18n::get("selectWorld.enterName")),
    tSeed(1, I18n::get("selectWorld.enterSeed"))
{
}

SimpleChooseLevelScreen::~SimpleChooseLevelScreen()
{
    if (bHeader) delete bHeader;
    delete bGamemode;
    delete bCheats;
    delete bWorldType;
    delete bExperimental;
    delete bBack;
    delete bCreate;
}

void SimpleChooseLevelScreen::init()
{
    ChooseLevelScreen::init();

    tLevelName.text = I18n::get("selectWorld.newWorld");

    bHeader = new Touch::THeader(0, I18n::get("selectWorld.createWorld"));
    bBack = new ImageButton(2, "");
    {
        ImageDef def;
        def.name = "gui/touchgui.png";
        def.width = 34;
        def.height = 26;
        def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
        bBack->setImageDef(def, true);
    }
    bGamemode = new Button(1, I18n::get("selectWorld.survivalMode"));
    bCheats  = new Button(4, I18n::get("selectWorld.cheatsOff"));
    bWorldType = new Button(5, getWorldTypeName(worldType));
    bExperimental = new Button(6, experimental ? I18n::get("selectWorld.experimental.on") : I18n::get("selectWorld.experimental.off"));
    bExperimental->active = (worldType == WorldType::INFINITE_SIZE);
    bCreate  = new Button(3, I18n::get("gui.createButton"));

    buttons.push_back(bHeader);
    buttons.push_back(bBack);
    buttons.push_back(bGamemode);
    buttons.push_back(bCheats);
    buttons.push_back(bWorldType);
    buttons.push_back(bExperimental);
    buttons.push_back(bCreate);

    tabButtons.push_back(bGamemode);
    tabButtons.push_back(bCheats);
    tabButtons.push_back(bWorldType);
    tabButtons.push_back(bExperimental);
    tabButtons.push_back(bBack);
    tabButtons.push_back(bCreate);

    textBoxes.push_back(&tLevelName);
    textBoxes.push_back(&tSeed);
}

void SimpleChooseLevelScreen::setupPositions()
{
    int buttonHeight = bBack->height;

    bBack->x = width - bBack->width;
    bBack->y = 0;

    if (bHeader) {
        bHeader->x = 0;
        bHeader->y = 0;
        bHeader->width = width - bBack->width;
        bHeader->height = buttonHeight;
    }

    int centerX = width / 2;
    const int padding = 5;

    tLevelName.width = tSeed.width = 200;
    tLevelName.x = centerX - tLevelName.width / 2;
    tLevelName.y = buttonHeight + 15;

    tSeed.x = tLevelName.x;
    tSeed.y = tLevelName.y + 26;

    const int buttonWidth = 140;
    const int buttonSpacing = 10;
    const int totalButtonWidth = buttonWidth * 2 + buttonSpacing;

    bGamemode->width = buttonWidth;
    bCheats->width = buttonWidth;
    bWorldType->width = buttonWidth;
    bExperimental->width = buttonWidth;

    int row1Y = tSeed.y + 22;
    int row2Y = row1Y + bGamemode->height + 6;

    bGamemode->x = centerX - totalButtonWidth / 2;
    bGamemode->y = row1Y;

    bCheats->x = bGamemode->x + buttonWidth + buttonSpacing;
    bCheats->y = row1Y;

    bWorldType->x = bGamemode->x;
    bWorldType->y = row2Y;

    bExperimental->x = bCheats->x;
    bExperimental->y = row2Y;

    bCreate->width = 110;
    bCreate->x = centerX - bCreate->width / 2;
    int bottomPadding = 12;
    bCreate->y = height - bottomPadding - bCreate->height;
}

void SimpleChooseLevelScreen::tick()
{
    for (auto* tb : textBoxes)
        tb->tick(minecraft);
}

void SimpleChooseLevelScreen::render( int xm, int ym, float a )
{
    renderDirtBackground(0);
    glEnable2(GL_BLEND);

    const char* modeDesc = NULL;
    if (gamemode == GameType::Survival) {
        modeDesc = "Mobs, health and gather resources";
    } else if (gamemode == GameType::Creative) {
        modeDesc = "Unlimited resources and flying";
    }
    if (modeDesc) {
        drawCenteredString(minecraft->font, modeDesc, width / 2, bExperimental->y + bExperimental->height + 4, 0xffcccccc);
    }

    drawString(minecraft->font, I18n::get("selectWorld.enterName") + ":", tLevelName.x, tLevelName.y - Font::DefaultLineHeight - 2, 0xffcccccc);
    drawString(minecraft->font, I18n::get("selectWorld.enterSeed") + ":", tSeed.x, tSeed.y - Font::DefaultLineHeight - 2, 0xffcccccc);

    Screen::render(xm, ym, a);
    glDisable2(GL_BLEND);
}

void SimpleChooseLevelScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == MouseAction::ACTION_LEFT) {
        int lvlTop = tLevelName.y - (Font::DefaultLineHeight + 4);
        int lvlBottom = tLevelName.y + tLevelName.height;
        int lvlLeft = tLevelName.x;
        int lvlRight = tLevelName.x + tLevelName.width;
        bool clickedLevel = x >= lvlLeft && x < lvlRight && y >= lvlTop && y < lvlBottom;

        int seedTop = tSeed.y - (Font::DefaultLineHeight + 4);
        int seedBottom = tSeed.y + tSeed.height;
        int seedLeft = tSeed.x;
        int seedRight = tSeed.x + tSeed.width;
        bool clickedSeed  = x >= seedLeft && x < seedRight && y >= seedTop && y < seedBottom;

        if (clickedLevel) {
            tLevelName.setFocus(minecraft);
            tSeed.loseFocus(minecraft);
        } else if (clickedSeed) {
            tSeed.setFocus(minecraft);
            tLevelName.loseFocus(minecraft);
        } else {
            tLevelName.loseFocus(minecraft);
            tSeed.loseFocus(minecraft);
        }
    }

    Screen::mouseClicked(x, y, buttonNum);
}

void SimpleChooseLevelScreen::buttonClicked( Button* button )
{
    if (hasChosen)
        return;

    if (button == bGamemode) {
        gamemode ^= 1;
        bGamemode->msg = (gamemode == GameType::Survival) ? I18n::get("selectWorld.survivalMode") : I18n::get("selectWorld.creativeMode");
        return;
    }

    if (button == bCheats) {
        cheatsEnabled = !cheatsEnabled;
        bCheats->msg = cheatsEnabled ? I18n::get("selectWorld.cheatsOn") : I18n::get("selectWorld.cheatsOff");
        return;
    }

    if (button == bWorldType) {
        worldType = (worldType + 1) % WorldType::WORLD_TYPE_COUNT;
        bWorldType->msg = getWorldTypeName(worldType);
        if (worldType == WorldType::INFINITE_SIZE) {
            bExperimental->active = true;
            bExperimental->msg = experimental ? I18n::get("selectWorld.experimental.on") : I18n::get("selectWorld.experimental.off");
        } else {
            experimental = false;
            bExperimental->active = false;
            bExperimental->msg = I18n::get("selectWorld.experimental.off");
        }
        return;
    }

    if (button == bExperimental && worldType == WorldType::INFINITE_SIZE) {
        experimental = !experimental;
        bExperimental->msg = experimental ? I18n::get("selectWorld.experimental.on") : I18n::get("selectWorld.experimental.off");
        return;
    }

    if (button == bCreate && !tLevelName.text.empty()) {
        int seed = getEpochTimeS();
        if (!tSeed.text.empty()) {
            std::string seedString = Util::stringTrim(tSeed.text);
            int tmpSeed;
            if (sscanf(seedString.c_str(), "%d", &tmpSeed) > 0) {
                seed = tmpSeed;
            } else {
                seed = Util::hashCode(seedString);
            }
        }
        std::string levelId = getUniqueLevelName(tLevelName.text);
        bool isFlat = (worldType == WorldType::FLAT);
        LevelSettings settings(seed, gamemode, cheatsEnabled, worldType, isFlat, experimental);
        minecraft->selectLevel(levelId, levelId, settings);
        minecraft->hostMultiplayer();
        minecraft->setScreen(new ProgressScreen());
        hasChosen = true;
        return;
    }

    if (button == bBack) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
    }
}

void SimpleChooseLevelScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        return;
    }
    Screen::keyPressed(eventKey);
}

bool SimpleChooseLevelScreen::handleBackEvent(bool isDown) {
	if (!isDown)
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	return true; 
}


