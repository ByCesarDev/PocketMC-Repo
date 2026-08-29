#include "UISettingsScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"
#include "SafeAreaScreen.h"

UISettingsScreen::UISettingsScreen()
    : btnUIProfile(nullptr), btnSafeArea(nullptr), btnDone(nullptr) {}

UISettingsScreen::~UISettingsScreen() {}

void UISettingsScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();

    btnUIProfile = new Button(1, 0, 0, 200, 20, "");
    btnSafeArea = new Button(2, 0, 0, 200, 20, I18n::get("options.safeArea.button"));
    btnDone = new Button(0, 0, 0, 200, 20, I18n::get("gui.done"));

    buttons.push_back(btnUIProfile);
    buttons.push_back(btnSafeArea);
    buttons.push_back(btnDone);

    updateProfileButtonText();
    setupPositions();
}

void UISettingsScreen::setupPositions() {
    int btnW = std::min(width - 40, 260);
    int startY = height / 4 + 24;

    if (btnUIProfile) {
        btnUIProfile->width = btnW;
        btnUIProfile->x = width / 2 - btnW / 2;
        btnUIProfile->y = startY;
    }
    if (btnSafeArea) {
        btnSafeArea->width = btnW;
        btnSafeArea->x = width / 2 - btnW / 2;
        btnSafeArea->y = startY + 28;
    }
    if (btnDone) {
        btnDone->width = btnW;
        btnDone->x = width / 2 - btnW / 2;
        btnDone->y = height - 36;
    }
}

void UISettingsScreen::updateProfileButtonText() {
    if (!btnUIProfile || !minecraft) return;
    int profile = minecraft->options.getIntValue(OPTIONS_UI_PROFILE);
    std::string profileName = (profile == 0)
        ? I18n::get("options.uiProfile.classic")
        : I18n::get("options.uiProfile.pocket");
    btnUIProfile->msg = I18n::get("options.uiProfile") + profileName;
}

void UISettingsScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    } else if (button->id == 1) {
        int current = minecraft->options.getIntValue(OPTIONS_UI_PROFILE);
        int next = (current == 0) ? 1 : 0;
        minecraft->options.set(OPTIONS_UI_PROFILE, next);
        updateProfileButtonText();
    } else if (button->id == 2) {
        minecraft->pushScreen(new SafeAreaScreen());
    }
}

void UISettingsScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("options.uiSettingsTitle"), width / 2, 20, 0xffffff);
    Screen::render(xm, ym, a);
}

void UISettingsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void UISettingsScreen::removed() {
    if (minecraft) {
        minecraft->options.save();
    }
}
