#include "AccessibilitySettingsScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"

AccessibilitySettingsScreen::AccessibilitySettingsScreen() : group(nullptr) {}
AccessibilitySettingsScreen::~AccessibilitySettingsScreen() { delete group; }

void AccessibilitySettingsScreen::init() {
    btnDone = new Button(0, width / 2 - 100, height - 30, 200, 20, I18n::get("gui.done"));
    buttons.push_back(btnDone);
    group = new OptionsGroup("options.accessibility.title");
    group->x = width / 2 - 150;
    group->y = 40;
    group->width = 300;
    group->height = height - 80;
    group->addOptionItem(OPTIONS_USERNAME, minecraft);
    group->addOptionItem(OPTIONS_DIFFICULTY, minecraft);
    group->addOptionItem(OPTIONS_THIRD_PERSON_VIEW, minecraft);
    group->addOptionItem(OPTIONS_GUI_SCALE, minecraft);
    group->addOptionItem(OPTIONS_SERVER_VISIBLE, minecraft);
    group->addOptionItem(OPTIONS_HIGH_CONTRAST, minecraft);
    group->addOptionItem(OPTIONS_DAMAGE_TILT, minecraft);
    group->setupPositions();
}

void AccessibilitySettingsScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    }
}

void AccessibilitySettingsScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("options.accessibility.title"), width / 2, 15, 0xffffff);
    if (group) group->render(minecraft, xm, ym);
    Screen::render(xm, ym, a);
}

void AccessibilitySettingsScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (group) group->mouseClicked(minecraft, x, y, buttonNum);
}

void AccessibilitySettingsScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
    if (group) group->mouseReleased(minecraft, x, y, buttonNum);
}

void AccessibilitySettingsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    Screen::mouseWheel(dx, dy, xm, ym);
    if (group) group->mouseWheel(dx, dy, xm, ym);
}

void AccessibilitySettingsScreen::renderBackground() {
    Screen::renderBackground();
}

void AccessibilitySettingsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void AccessibilitySettingsScreen::tick() {
    if (group) group->tick(minecraft);
    Screen::tick();
}
