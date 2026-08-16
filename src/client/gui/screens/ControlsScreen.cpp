#include "ControlsScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"
#include "MouseSettingsScreen.h"
#include "KeyboardSettingsScreen.h"

ControlsScreen::ControlsScreen() : group(nullptr) {}
ControlsScreen::~ControlsScreen() { delete group; }

void ControlsScreen::init() {
    btnDone = new Button(0, width / 2 - 100, height - 30, 200, 20, I18n::get("gui.done"));
    buttons.push_back(btnDone);
    group = new OptionsGroup("options.controls");
    group->x = width / 2 - 150;
    group->y = 40;
    group->width = 300;
    group->height = height - 100;
    group->addOptionItem(OPTIONS_AUTOJUMP, minecraft);
    group->addOptionItem(OPTIONS_ALLOW_SPRINT, minecraft);
    group->addOptionItem(OPTIONS_IS_LEFT_HANDED, minecraft);
    group->addOptionItem(OPTIONS_IS_JOY_TOUCH_AREA, minecraft);
    group->setupPositions();
    
    buttons.push_back(new Button(1, width / 2 - 155, height - 55, 150, 20, I18n::get("options.mouse_settings")));
    buttons.push_back(new Button(2, width / 2 + 5, height - 55, 150, 20, I18n::get("options.keyboard_settings")));
}

void ControlsScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    } else if (button->id == 1) {
        minecraft->pushScreen(new MouseSettingsScreen());
    } else if (button->id == 2) {
        minecraft->pushScreen(new KeyboardSettingsScreen());
    }
}

void ControlsScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("options.controls"), width / 2, 15, 0xffffff);
    if (group) group->render(minecraft, xm, ym);
    Screen::render(xm, ym, a);
}

void ControlsScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (group) group->mouseClicked(minecraft, x, y, buttonNum);
}

void ControlsScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
    if (group) group->mouseReleased(minecraft, x, y, buttonNum);
}

void ControlsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    Screen::mouseWheel(dx, dy, xm, ym);
    if (group) group->mouseWheel(dx, dy, xm, ym);
}

void ControlsScreen::renderBackground() {
    Screen::renderBackground();
}

void ControlsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void ControlsScreen::tick() {
    if (group) group->tick(minecraft);
    Screen::tick();
}
