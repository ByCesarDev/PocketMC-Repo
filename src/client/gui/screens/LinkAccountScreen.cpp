#include "LinkAccountScreen.h"
#include "ProfileScreen.h"
#include "../../Minecraft.h"
#include "../Font.h"
#include "../components/Button.h"
#include "../../../AppPlatform.h"
#include "../../player/AccountManager.h"
#include "locale/I18n.h"

LinkAccountScreen::LinkAccountScreen()
:   _btnSignIn(1, "Sign In"),
    _btnCreateAccount(2, "Create Account (Web)"),
    _btnBack(3, I18n::get("gui.cancel")),
    tUsername(10, "Username"),
    tPassword(11, "Password"),
    _isError(false)
{
}

LinkAccountScreen::~LinkAccountScreen()
{
}

void LinkAccountScreen::init()
{
    buttons.clear();
    tabButtons.clear();
    textBoxes.clear();

    buttons.push_back(&_btnSignIn);
    buttons.push_back(&_btnCreateAccount);
    buttons.push_back(&_btnBack);

    tabButtons.push_back(&_btnSignIn);
    tabButtons.push_back(&_btnCreateAccount);
    tabButtons.push_back(&_btnBack);

    tPassword.isPassword = true;

    textBoxes.push_back(&tUsername);
    textBoxes.push_back(&tPassword);

    setupPositions();
}

void LinkAccountScreen::setupPositions()
{
    int fieldW = 160;
    int btnH = 22;

    tUsername.width = fieldW;
    tUsername.height = 20;
    tUsername.x = (width - fieldW) / 2;
    tUsername.y = height / 2 - 40;

    tPassword.width = fieldW;
    tPassword.height = 20;
    tPassword.x = (width - fieldW) / 2;
    tPassword.y = tUsername.y + 32;

    _btnSignIn.width = fieldW;
    _btnSignIn.height = btnH;
    _btnSignIn.x = (width - fieldW) / 2;
    _btnSignIn.y = tPassword.y + 36;

    _btnCreateAccount.width = fieldW;
    _btnCreateAccount.height = btnH;
    _btnCreateAccount.x = (width - fieldW) / 2;
    _btnCreateAccount.y = _btnSignIn.y + 26;

    _btnBack.width = 90;
    _btnBack.height = btnH;
    _btnBack.x = (width - 90) / 2;
    _btnBack.y = _btnCreateAccount.y + 30;
}

void LinkAccountScreen::render(int xm, int ym, float a)
{
    renderBackground();

    int centerX = width / 2;
    drawCenteredString(font, "LINK POCKETMC ACCOUNT", centerX, height / 2 - 85, 0xffffffff);
    drawCenteredString(font, "Sign in with your PocketMC Username", centerX, height / 2 - 70, 0xffaaaaaa);

    drawString(font, "Username:", tUsername.x, tUsername.y - 10, 0xffdddddd);
    drawString(font, "Password:", tPassword.x, tPassword.y - 10, 0xffdddddd);

    if (!_statusMsg.empty()) {
        int color = _isError ? 0xffff5555 : 0xff55ff55;
        drawCenteredString(font, _statusMsg, centerX, _btnBack.y + 26, color);
    }

    Screen::render(xm, ym, a);
}

void LinkAccountScreen::buttonClicked(Button* button)
{
    if (button == &_btnBack) {
        minecraft->setScreen(new ProfileScreen());
        return;
    }

    if (button == &_btnCreateAccount) {
        minecraft->platform()->openURL("http://localhost:5173/pages/register.html");
        return;
    }

    if (button == &_btnSignIn) {
        if (tUsername.text.empty() || tPassword.text.empty()) {
            _statusMsg = "Please enter username and password.";
            _isError = true;
            return;
        }

        _statusMsg = "Authenticating...";
        _isError = false;

        std::string err;
        bool ok = AccountManager::loginWithSupabase(tUsername.text, tPassword.text, err);

        if (ok) {
            minecraft->setScreen(new ProfileScreen());
        } else {
            _statusMsg = err.empty() ? "Login failed." : err;
            _isError = true;
        }
    }
}

void LinkAccountScreen::keyPressed(int eventKey)
{
    if (eventKey == 1 || eventKey == 27) { // Escape
        minecraft->setScreen(new ProfileScreen());
        return;
    }
    Screen::keyPressed(eventKey);
}
