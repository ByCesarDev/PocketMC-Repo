#include "KeyboardSettingsScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"

KeyboardSettingsScreen::KeyboardSettingsScreen() : group(nullptr) {}
KeyboardSettingsScreen::~KeyboardSettingsScreen() { delete group; }

void KeyboardSettingsScreen::init() {
    btnDone = new Button(0, width / 2 - 100, height - 30, 200, 20, I18n::get("gui.done"));
    buttons.push_back(btnDone);
    group = new OptionsGroup("options.keyboard_settings.title");
    group->x = width / 2 - 150;
    group->y = 40;
    group->width = 300;
    group->height = height - 80;
    for (int i = OPTIONS_KEY_FORWARD; i <= OPTIONS_KEY_USE; i++) {
        group->addOptionItem((OptionId)i, minecraft);
    }
    group->setupPositions();
}

void KeyboardSettingsScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    }
}

void KeyboardSettingsScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("options.keyboard_settings.title"), width / 2, 15, 0xffffff);
    if (group) group->render(minecraft, xm, ym);
    Screen::render(xm, ym, a);
}

void KeyboardSettingsScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (group) group->mouseClicked(minecraft, x, y, buttonNum);
}

void KeyboardSettingsScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
    if (group) group->mouseReleased(minecraft, x, y, buttonNum);
}

void KeyboardSettingsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    Screen::mouseWheel(dx, dy, xm, ym);
    if (group) group->mouseWheel(dx, dy, xm, ym);
}

void KeyboardSettingsScreen::renderBackground() {
    Screen::renderBackground();
}

void KeyboardSettingsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void KeyboardSettingsScreen::tick() {
    if (group) group->tick(minecraft);
    Screen::tick();
}
