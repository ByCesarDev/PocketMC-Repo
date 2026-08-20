#include "OptionsScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "../../renderer/LevelRenderer.h"
#include "locale/I18n.h"
#include "SkinCustomizationScreen.h"
#include "GraphicsScreen.h"
#include "LanguageScreen.h"
#include "ResourcePacksScreen.h"
#include "MusicSoundScreen.h"
#include "ControlsScreen.h"
#include "ChatSettingsScreen.h"
#include "AccessibilitySettingsScreen.h"

OptionsScreen::OptionsScreen() {
}

OptionsScreen::~OptionsScreen() {
}

void OptionsScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();

    btnSkin = new Button(1, 0, 0, 150, 20, I18n::get("options.skinCustomisation"));
    btnAudio = new Button(2, 0, 0, 150, 20, I18n::get("options.musicAndSounds"));
    btnGraphics = new Button(3, 0, 0, 150, 20, I18n::get("options.video"));
    btnControls = new Button(4, 0, 0, 150, 20, I18n::get("options.controls"));
    btnLanguage = new Button(5, 0, 0, 150, 20, I18n::get("options.language"));
    btnChat = new Button(6, 0, 0, 150, 20, I18n::get("options.chat.title"));
    btnResourcePacks = new Button(7, 0, 0, 150, 20, I18n::get("options.resourcepack"));
    btnAccessibility = new Button(8, 0, 0, 150, 20, I18n::get("options.accessibility.title"));
    btnDone = new Button(0, 0, 0, 200, 20, I18n::get("gui.done"));
    
    buttons.push_back(btnSkin);
    buttons.push_back(btnAudio);
    buttons.push_back(btnGraphics);
    buttons.push_back(btnControls);
    buttons.push_back(btnLanguage);
    buttons.push_back(btnChat);
    buttons.push_back(btnResourcePacks);
    buttons.push_back(btnAccessibility);
    buttons.push_back(btnDone);

    setupPositions();
}

void OptionsScreen::setupPositions() {
    int w = std::min((width - 30) / 2, 150);
    if (w < 100) w = 100;
    int col1 = width / 2 - w - 5;
    int col2 = width / 2 + 5;
    int startY = height / 6 + 10;
    int spacing = 24;

    if (btnSkin) { btnSkin->width = w; btnSkin->x = col1; btnSkin->y = startY; }
    if (btnAudio) { btnAudio->width = w; btnAudio->x = col2; btnAudio->y = startY; }

    if (btnGraphics) { btnGraphics->width = w; btnGraphics->x = col1; btnGraphics->y = startY + spacing; }
    if (btnControls) { btnControls->width = w; btnControls->x = col2; btnControls->y = startY + spacing; }

    if (btnLanguage) { btnLanguage->width = w; btnLanguage->x = col1; btnLanguage->y = startY + spacing * 2; }
    if (btnChat) { btnChat->width = w; btnChat->x = col2; btnChat->y = startY + spacing * 2; }

    if (btnResourcePacks) { btnResourcePacks->width = w; btnResourcePacks->x = col1; btnResourcePacks->y = startY + spacing * 3; }
    if (btnAccessibility) { btnAccessibility->width = w; btnAccessibility->x = col2; btnAccessibility->y = startY + spacing * 3; }

    if (btnDone) { btnDone->width = std::min(200, width - 20); btnDone->x = width / 2 - btnDone->width / 2; btnDone->y = std::max(startY + spacing * 4 + 10, height - 30); }
}

void OptionsScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    } else if (button->id == 1) {
        minecraft->pushScreen(new SkinCustomizationScreen());
    } else if (button->id == 2) {
        minecraft->pushScreen(new MusicSoundScreen());
    } else if (button->id == 3) {
        minecraft->pushScreen(new GraphicsScreen());
    } else if (button->id == 4) {
        minecraft->pushScreen(new ControlsScreen());
    } else if (button->id == 5) {
        minecraft->pushScreen(new LanguageScreen());
    } else if (button->id == 6) {
        minecraft->pushScreen(new ChatSettingsScreen());
    } else if (button->id == 7) {
        minecraft->pushScreen(new ResourcePacksScreen());
    } else if (button->id == 8) {
        minecraft->pushScreen(new AccessibilitySettingsScreen());
    }
}

void OptionsScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("menu.options"), width / 2, 15, 0xffffff);
    Screen::render(xm, ym, a);
}

void OptionsScreen::renderBackground() {
    Screen::renderBackground();
}

void OptionsScreen::tick() {
    
}

void OptionsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void OptionsScreen::removed() {
    minecraft->options.save();
    if (minecraft->level && minecraft->levelRenderer) {
        minecraft->levelRenderer->allChanged();
    }
}
