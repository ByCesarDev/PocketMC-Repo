#include "UsernameScreen.h"
#include "StartMenuScreen.h"
#include "../../Minecraft.h"
#include "../Font.h"
#include "../components/Button.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../AppPlatform.h"
#include "locale/I18n.h"

UsernameScreen::UsernameScreen()
:   _btnDone(0, I18n::get("gui.done")),
    tUsername(0, I18n::get("options.username")),
    _cursorBlink(0)
{
}

UsernameScreen::~UsernameScreen()
{
}

void UsernameScreen::init()
{
    _input = "";
    _btnDone.active = false; // disabled until name typed
    buttons.push_back(&_btnDone);
    tabButtons.push_back(&_btnDone);
    textBoxes.push_back(&tUsername);
    setupPositions();
}

void UsernameScreen::setupPositions()
{
    int cx = width / 2;
    int cy = height / 2;

    _btnDone.width  = (std::max)(100, font->width(_btnDone.msg) + 16);
    _btnDone.height = 24;
    _btnDone.x = (width - _btnDone.width) / 2;
    _btnDone.y = height / 2 + 52;

    tUsername.width = 120;
    tUsername.height = 20;
    tUsername.x = (width - tUsername.width) / 2;
    tUsername.y = _btnDone.y - 60;
}

void UsernameScreen::tick()
{
    for (auto* tb : textBoxes)
        tb->tick(minecraft);
}

void UsernameScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_RETURN) {
        if (!tUsername.text.empty())
            buttonClicked(&_btnDone);
    }

    // deliberately do NOT call super::keyPressed — that would close the screen on Escape
    Screen::keyPressed(eventKey);

    // enable the Done button only when there is some text (and ensure it updates after backspace)
    _btnDone.active = !tUsername.text.empty();
}

void UsernameScreen::removed()
{
    minecraft->platform()->hideKeyboard();
}

void UsernameScreen::buttonClicked(Button* button)
{
    if (button == &_btnDone && !tUsername.text.empty()) {
        minecraft->options.set(OPTIONS_USERNAME, tUsername.text);
        minecraft->options.save();
        minecraft->setScreen(NULL); // goes to StartMenuScreen
    }
}

void UsernameScreen::render(int xm, int ym, float a)
{
    // Dark dirt background
    renderBackground();

    int cx = width / 2;
    int cy = height / 2;

    // Title
    drawCenteredString(font, I18n::get("username.title"), cx, cy - 70, 0xffffffff);

    // Subtitle
    drawCenteredString(font, I18n::get("username.desc1"), cx, cy - 52, 0xffaaaaaa);
    drawCenteredString(font, I18n::get("username.desc2"), cx, cy - 40, 0xffaaaaaa);
    drawCenteredString(font, I18n::get("username.desc3"), cx, cy - 28, 0xffaaaaaa);

    // // Hint below box
    // drawCenteredString(font, "Max 16 characters", cx, cy + 20, 0xff808080);

    // Buttons (Done)
    super::render(xm, ym, a);
}
