#include "MusicSoundScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"

MusicSoundScreen::MusicSoundScreen() : group(nullptr) {}
MusicSoundScreen::~MusicSoundScreen() { delete group; }

void MusicSoundScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();
    if (group) { delete group; group = nullptr; }

    btnDone = new Button(0, 0, 0, 200, 20, I18n::get("gui.done"));
    buttons.push_back(btnDone);

    group = new OptionsGroup("options.musicAndSounds");
    group->addOptionItem(OPTIONS_MUSIC_VOLUME, minecraft);
    group->addOptionItem(OPTIONS_SOUND_VOLUME, minecraft);
    
    setupPositions();
}

void MusicSoundScreen::setupPositions() {
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

void MusicSoundScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    }
}

void MusicSoundScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("options.musicAndSounds"), width / 2, 15, 0xffffff);
    if (group) group->render(minecraft, xm, ym);
    Screen::render(xm, ym, a);
}

void MusicSoundScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (group) group->mouseClicked(minecraft, x, y, buttonNum);
}

void MusicSoundScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
    if (group) group->mouseReleased(minecraft, x, y, buttonNum);
}

void MusicSoundScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    Screen::mouseWheel(dx, dy, xm, ym);
    if (group) group->mouseWheel(dx, dy, xm, ym);
}

void MusicSoundScreen::renderBackground() {
    Screen::renderBackground();
}

void MusicSoundScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void MusicSoundScreen::tick() {
    if (group) group->tick(minecraft);
    Screen::tick();
}
