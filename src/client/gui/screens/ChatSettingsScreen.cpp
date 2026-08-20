#include "ChatSettingsScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"

ChatSettingsScreen::ChatSettingsScreen() : group(nullptr) {}
ChatSettingsScreen::~ChatSettingsScreen() { delete group; }

void ChatSettingsScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();
    if (group) { delete group; group = nullptr; }

    btnDone = new Button(0, 0, 0, 200, 20, I18n::get("gui.done"));
    buttons.push_back(btnDone);

    group = new OptionsGroup("options.chat.title");
    group->addOptionItem(OPTIONS_CHAT_VISIBLE, minecraft);
    group->addOptionItem(OPTIONS_CHAT_COLORS, minecraft);
    
    setupPositions();
}

void ChatSettingsScreen::setupPositions() {
    if (btnDone) {
        btnDone->width = std::min(200, width - 20);
        btnDone->x = width / 2 - btnDone->width / 2;
        btnDone->y = height - 28;
    }
    if (group) {
        group->width = std::min(width - 20, 360);
        group->x = width / 2 - group->width / 2;
        group->y = 35;
        group->height = height - 70;
        group->setupPositions();
    }
}

void ChatSettingsScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    }
}

void ChatSettingsScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("options.chat.title"), width / 2, 15, 0xffffff);
    if (group) group->render(minecraft, xm, ym);
    Screen::render(xm, ym, a);
}

void ChatSettingsScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (group) group->mouseClicked(minecraft, x, y, buttonNum);
}

void ChatSettingsScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
    if (group) group->mouseReleased(minecraft, x, y, buttonNum);
}

void ChatSettingsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    Screen::mouseWheel(dx, dy, xm, ym);
    if (group) group->mouseWheel(dx, dy, xm, ym);
}

void ChatSettingsScreen::renderBackground() {
    Screen::renderBackground();
}

void ChatSettingsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void ChatSettingsScreen::tick() {
    if (group) group->tick(minecraft);
    Screen::tick();
}
