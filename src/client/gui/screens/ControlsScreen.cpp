#include "ControlsScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"
#include "MouseSettingsScreen.h"
#include "KeyboardSettingsScreen.h"

ControlsScreen::ControlsScreen() : btnDone(nullptr), btnMouse(nullptr), btnKeyboard(nullptr), group(nullptr) {}
ControlsScreen::~ControlsScreen() { delete group; }

void ControlsScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();
    if (group) { delete group; group = nullptr; }

    btnDone = new Button(0, 0, 0, 200, 20, I18n::get("gui.done"));
    btnMouse = new Button(1, 0, 0, 150, 20, I18n::get("options.mouse_settings"));
    btnKeyboard = new Button(2, 0, 0, 150, 20, I18n::get("options.keyboard_settings"));

    buttons.push_back(btnDone);
    buttons.push_back(btnMouse);
    buttons.push_back(btnKeyboard);

    group = new OptionsGroup("options.controls");
    group->addOptionItem(OPTIONS_USE_TOUCHSCREEN, minecraft);
    group->addOptionItem(OPTIONS_AUTOJUMP, minecraft);
    group->addOptionItem(OPTIONS_ALLOW_SPRINT, minecraft);
    group->addOptionItem(OPTIONS_IS_LEFT_HANDED, minecraft);
    group->addOptionItem(OPTIONS_IS_JOY_TOUCH_AREA, minecraft);

    setupPositions();
}

void ControlsScreen::setupPositions() {
    int subW = std::min((width - 30) / 2, 150);
    if (subW < 100) subW = 100;
    
    if (btnMouse) {
        btnMouse->width = subW;
        btnMouse->x = width / 2 - subW - 5;
        btnMouse->y = height - 55;
    }
    if (btnKeyboard) {
        btnKeyboard->width = subW;
        btnKeyboard->x = width / 2 + 5;
        btnKeyboard->y = height - 55;
    }
    if (btnDone) {
        btnDone->width = std::min(200, width - 20);
        btnDone->x = width / 2 - btnDone->width / 2;
        btnDone->y = height - 28;
    }
    if (group) {
        group->width = std::min(width - 20, 360);
        group->x = width / 2 - group->width / 2;
        group->y = 35;
        group->height = height - 95;
        group->setupPositions();
    }
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
