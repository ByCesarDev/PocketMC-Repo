#include "GraphicsScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "../../renderer/LevelRenderer.h"
#include "locale/I18n.h"

GraphicsScreen::GraphicsScreen() : group(nullptr) {}
GraphicsScreen::~GraphicsScreen() { delete group; }

void GraphicsScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();
    if (group) { delete group; group = nullptr; }

    btnDone = new Button(0, 0, 0, 200, 20, I18n::get("gui.done"));
    buttons.push_back(btnDone);
    group = new OptionsGroup("options.videoTitle");
    group->addOptionItem(OPTIONS_FOV, minecraft);
    group->addOptionItem(OPTIONS_FANCY_GRAPHICS, minecraft);
    group->addOptionItem(OPTIONS_VIEW_DISTANCE, minecraft);
    group->addOptionItem(OPTIONS_VIEW_BOBBING, minecraft);
    group->addOptionItem(OPTIONS_AMBIENT_OCCLUSION, minecraft);
    group->addOptionItem(OPTIONS_LIMIT_FRAMERATE, minecraft);
    group->addOptionItem(OPTIONS_VSYNC, minecraft);
    group->addOptionItem(OPTIONS_CLOUDS, minecraft);
    group->addOptionItem(OPTIONS_PARTICLES, minecraft);
    group->addOptionItem(OPTIONS_ANAGLYPH_3D, minecraft);
    group->addOptionItem(OPTIONS_RENDER_DEBUG, minecraft);
    group->addOptionItem(OPTIONS_SHOW_VIGNETTE, minecraft);
    
    setupPositions();
}

void GraphicsScreen::setupPositions() {
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

void GraphicsScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    }
}

void GraphicsScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("options.videoTitle"), width / 2, 15, 0xffffff);
    if (group) group->render(minecraft, xm, ym);
    Screen::render(xm, ym, a);
}

void GraphicsScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (group) group->mouseClicked(minecraft, x, y, buttonNum);
}

void GraphicsScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
    if (group) group->mouseReleased(minecraft, x, y, buttonNum);
}

void GraphicsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    Screen::mouseWheel(dx, dy, xm, ym);
    if (group) group->mouseWheel(dx, dy, xm, ym);
}

void GraphicsScreen::renderBackground() {
    Screen::renderBackground();
}

void GraphicsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void GraphicsScreen::tick() {
    if (group) group->tick(minecraft);
    Screen::tick();
}

void GraphicsScreen::removed() {
    minecraft->options.save();
    if (minecraft->level && minecraft->levelRenderer) {
        minecraft->levelRenderer->allChanged();
    }
}
