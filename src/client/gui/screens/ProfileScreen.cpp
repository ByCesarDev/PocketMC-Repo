#include "ProfileScreen.h"
#include "StartMenuScreen.h"
#include "UsernameScreen.h"
#include "LinkAccountScreen.h"
#include "../../Minecraft.h"
#include "../Font.h"
#include "../components/Button.h"
#include "../../../AppPlatform.h"
#include "../../player/AccountManager.h"
#include "../../player/PlayerIdentity.h"
#include "locale/I18n.h"

ProfileScreen::ProfileScreen()
:   _btnAction(1, "Link PocketMC Account"),
    _btnSecondary(2, "Edit Local Username"),
    _btnBack(3, I18n::get("gui.back"))
{
}

ProfileScreen::~ProfileScreen()
{
}

void ProfileScreen::init()
{
    buttons.clear();
    tabButtons.clear();

    if (AccountManager::isOnline()) {
        _btnAction.msg    = "Manage Account";
        _btnSecondary.msg = "Sign Out";
    } else {
        _btnAction.msg    = "Link PocketMC Account";
        _btnSecondary.msg = "Edit Local Username";
    }

    buttons.push_back(&_btnAction);
    buttons.push_back(&_btnSecondary);
    buttons.push_back(&_btnBack);

    tabButtons.push_back(&_btnAction);
    tabButtons.push_back(&_btnSecondary);
    tabButtons.push_back(&_btnBack);

    setupPositions();
}

void ProfileScreen::setupPositions()
{
    int cardW = 220;
    int btnH  = 22;

    _btnAction.width  = cardW;
    _btnAction.height = btnH;
    _btnAction.x = (width - cardW) / 2;
    _btnAction.y = height / 2 + 10;

    _btnSecondary.width  = cardW;
    _btnSecondary.height = btnH;
    _btnSecondary.x = (width - cardW) / 2;
    _btnSecondary.y = _btnAction.y + 26;

    _btnBack.width  = 100;
    _btnBack.height = btnH;
    _btnBack.x = (width - 100) / 2;
    _btnBack.y = _btnSecondary.y + 32;
}

void ProfileScreen::render(int xm, int ym, float a)
{
    renderBackground();

    int cx     = width / 2;
    int startY = height / 6;

    drawCenteredString(font, "PROFILE", cx, startY, 0xffffffff);

    const PlayerIdentity& id = AccountManager::getIdentity();
    std::string localUname = minecraft->options.getStringValue(OPTIONS_USERNAME);

    if (!AccountManager::isOnline()) {
        // ── Offline ─────────────────────────────────────────────────────────
        drawCenteredString(font, "Account Type",  cx, startY + 18, 0xffaaaaaa);
        drawCenteredString(font, "Offline",        cx, startY + 30, 0xffcccccc);

        drawCenteredString(font, "Username",       cx, startY + 48, 0xffaaaaaa);
        drawCenteredString(font, localUname,        cx, startY + 60, 0xffffffff);

        drawCenteredString(font, "PocketMC Account", cx, startY + 78, 0xffaaaaaa);
        drawCenteredString(font, "Not linked",       cx, startY + 90, 0xffffaaaa);
    } else {
        // ── Online ──────────────────────────────────────────────────────────
        std::string uname = AccountManager::getUsername(localUname);
        std::string dname = AccountManager::getDisplayName(localUname);
        std::string pid   = AccountManager::getPlayerId();
        if (pid.size() > 18) pid = pid.substr(0, 16) + "...";

        bool liveConn = id.authenticated;

        drawCenteredString(font, "Account Type",   cx, startY + 14, 0xffaaaaaa);
        drawCenteredString(font, liveConn ? "Online" : "Online (Offline Mode)",
                           cx, startY + 26, liveConn ? 0xff55ff55 : 0xffffcc55);

        drawCenteredString(font, "Username",        cx, startY + 46, 0xffaaaaaa);
        drawCenteredString(font, "@" + uname,       cx, startY + 58, 0xffffffff);

        drawCenteredString(font, "Display Name",    cx, startY + 74, 0xffaaaaaa);
        drawCenteredString(font, dname,             cx, startY + 86, 0xffdddddd);

        drawCenteredString(font, "Player ID",       cx, startY + 102, 0xffaaaaaa);
        drawCenteredString(font, pid,               cx, startY + 114, 0xff888888);
    }

    Screen::render(xm, ym, a);
}

void ProfileScreen::buttonClicked(Button* button)
{
    if (button == &_btnBack) {
        minecraft->setScreen(new StartMenuScreen());
        return;
    }

    if (AccountManager::isOnline()) {
        if (button == &_btnAction) {
            // Manage Account — open web portal
            minecraft->platform()->openURL("http://localhost:5173/pages/profile.html");
        } else if (button == &_btnSecondary) {
            // Sign Out (Fase 19)
            AccountManager::logout();
            init();  // refresh UI labels
        }
    } else {
        if (button == &_btnAction) {
            minecraft->setScreen(new LinkAccountScreen());
        } else if (button == &_btnSecondary) {
            minecraft->setScreen(new UsernameScreen());
        }
    }
}

void ProfileScreen::keyPressed(int eventKey)
{
    if (eventKey == 1 || eventKey == 27) {
        minecraft->setScreen(new StartMenuScreen());
        return;
    }
    Screen::keyPressed(eventKey);
}
