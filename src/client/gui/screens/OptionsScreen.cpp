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
    int w = 150;
    int col1 = width / 2 - w - 5;
    int col2 = width / 2 + 5;
    
    // Main columns
    btnSkin = new Button(1, col1, height / 6 + 24, w, 20, I18n::get("options.skinCustomisation"));
    btnAudio = new Button(2, col2, height / 6 + 24, w, 20, I18n::get("options.musicAndSounds"));
    
    btnGraphics = new Button(3, col1, height / 6 + 48, w, 20, I18n::get("options.video"));
    btnControls = new Button(4, col2, height / 6 + 48, w, 20, I18n::get("options.controls"));
    
    btnLanguage = new Button(5, col1, height / 6 + 72, w, 20, I18n::get("options.language"));
    btnChat = new Button(6, col2, height / 6 + 72, w, 20, I18n::get("options.chat.title"));
    
    btnResourcePacks = new Button(7, col1, height / 6 + 96, w, 20, I18n::get("options.resourcepack"));
    btnAccessibility = new Button(8, col2, height / 6 + 96, w, 20, I18n::get("options.accessibility.title"));
    
    btnDone = new Button(0, width / 2 - 100, height / 6 + 168, 200, 20, I18n::get("gui.done"));
    
    buttons.push_back(btnSkin);
    buttons.push_back(btnAudio);
    buttons.push_back(btnGraphics);
    buttons.push_back(btnControls);
    buttons.push_back(btnLanguage);
    buttons.push_back(btnChat);
    buttons.push_back(btnResourcePacks);
    buttons.push_back(btnAccessibility);
    buttons.push_back(btnDone);
}

void OptionsScreen::setupPositions() {
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
