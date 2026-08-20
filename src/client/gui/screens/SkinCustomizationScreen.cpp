#include "SkinCustomizationScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"

SkinCustomizationScreen::SkinCustomizationScreen() : group(nullptr) {}
SkinCustomizationScreen::~SkinCustomizationScreen() { delete group; }

void SkinCustomizationScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();
    if (group) { delete group; group = nullptr; }

    btnDone = new Button(0, 0, 0, 200, 20, I18n::get("gui.done"));
    buttons.push_back(btnDone);

    group = new OptionsGroup("options.skinCustomisation");
    group->addOptionItem(OPTIONS_SKIN_CAPE, minecraft);
    group->addOptionItem(OPTIONS_SKIN_JACKET, minecraft);
    group->addOptionItem(OPTIONS_SKIN_LEFT_SLEEVE, minecraft);
    group->addOptionItem(OPTIONS_SKIN_RIGHT_SLEEVE, minecraft);
    group->addOptionItem(OPTIONS_SKIN_LEFT_PANTS, minecraft);
    group->addOptionItem(OPTIONS_SKIN_RIGHT_PANTS, minecraft);
    group->addOptionItem(OPTIONS_SKIN_HAT, minecraft);
    group->addOptionItem(OPTIONS_SKIN_MAIN_HAND, minecraft);
    
    setupPositions();
}

void SkinCustomizationScreen::setupPositions() {
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

void SkinCustomizationScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    }
}

void SkinCustomizationScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("options.skinCustomisation"), width / 2, 15, 0xffffff);
    if (group) group->render(minecraft, xm, ym);
    Screen::render(xm, ym, a);
}

void SkinCustomizationScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (group) group->mouseClicked(minecraft, x, y, buttonNum);
}

void SkinCustomizationScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
    if (group) group->mouseReleased(minecraft, x, y, buttonNum);
}

void SkinCustomizationScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    Screen::mouseWheel(dx, dy, xm, ym);
    if (group) group->mouseWheel(dx, dy, xm, ym);
}

void SkinCustomizationScreen::renderBackground() {
    Screen::renderBackground();
}

void SkinCustomizationScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void SkinCustomizationScreen::tick() {
    if (group) group->tick(minecraft);
    Screen::tick();
}
