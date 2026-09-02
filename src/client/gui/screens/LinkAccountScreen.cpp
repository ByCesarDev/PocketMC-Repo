#include "LinkAccountScreen.h"
#include "ProfileScreen.h"
#include "../../Minecraft.h"
#include "../Font.h"
#include "../components/Button.h"
#include "../../../AppPlatform.h"
#include "../../player/AccountManager.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../renderer/GuiShader.h"
#include "locale/I18n.h"
#include <algorithm>

LinkAccountScreen::LinkAccountScreen()
:   _btnBack(1, I18n::get("profile.back")),
    _btnSignIn(2, I18n::get("linkAccount.login")),
    tUsername(10, ""),
    tPassword(11, ""),
    _rememberMe(true),
    _isAuthenticating(false),
    _isError(false),
    _panelX(0), _panelY(0), _panelW(0), _panelH(0),
    _eyeX(0), _eyeY(0), _eyeW(0), _eyeH(0),
    _cbX(0), _cbY(0), _cbSize(0),
    _fpX(0), _fpY(0), _fpW(0), _fpH(0),
    _createBtnX(0), _createBtnY(0), _createBtnW(0), _createBtnH(0)
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

    _btnBack.msg = I18n::get("profile.back");
    _btnSignIn.msg = I18n::get("linkAccount.login");

    buttons.push_back(&_btnBack);
    buttons.push_back(&_btnSignIn);
    tabButtons.push_back(&_btnBack);
    tabButtons.push_back(&_btnSignIn);

    tUsername.hint = I18n::get("linkAccount.usernamePlaceholder");
    tPassword.hint = I18n::get("linkAccount.passwordPlaceholder");
    tPassword.isPassword = true;

    textBoxes.push_back(&tUsername);
    textBoxes.push_back(&tPassword);

    setupPositions();
}

void LinkAccountScreen::tick()
{
    tUsername.tick(minecraft);
    tPassword.tick(minecraft);
}

void LinkAccountScreen::setupPositions()
{
    int topBarH = 28;
    int margin = 6;

    // Top bar < Back button
    _btnBack.x = margin;
    _btnBack.y = 4;
    _btnBack.width = 46;
    _btnBack.height = 20;

    // Central card panel layout
    _panelW = std::min(224, width - 24);
    _panelH = 184;
    _panelX = (width - _panelW) / 2;
    _panelY = std::max(topBarH + 4, (height - _panelH) / 2 + 10);

    // Inputs inside panel
    int innerX = _panelX + 8;
    int innerW = _panelW - 16;

    tUsername.x = innerX;
    tUsername.y = _panelY + 26;
    tUsername.width = innerW;
    tUsername.height = 18;

    _eyeW = 18;
    _eyeH = 18;
    _eyeX = _panelX + _panelW - 8 - _eyeW;
    _eyeY = _panelY + 56;

    tPassword.x = innerX;
    tPassword.y = _panelY + 56;
    tPassword.width = innerW - _eyeW - 2;
    tPassword.height = 18;

    // Remember me & Forgot Password row
    _cbX = innerX;
    _cbY = _panelY + 78;
    _cbSize = 8;

    std::string fpText = I18n::get("linkAccount.forgotPassword");
    _fpW = (int)(font->width(fpText) * 0.72f);
    _fpX = _panelX + _panelW - 8 - _fpW;
    _fpY = _panelY + 78;
    _fpH = 10;

    // Sign In button
    _btnSignIn.x = innerX;
    _btnSignIn.y = _panelY + 91;
    _btnSignIn.width = innerW;
    _btnSignIn.height = 19;

    // Create Account button
    _createBtnX = innerX;
    _createBtnY = _panelY + 122;
    _createBtnW = innerW;
    _createBtnH = 18;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

void LinkAccountScreen::drawScaledString(const std::string& str, float x, float y, int color, float scale)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    font->drawShadow(str, 0, 0, color);
    glPopMatrix();
}

void LinkAccountScreen::drawPanel(int x, int y, int w, int h, unsigned int bg, unsigned int border)
{
    fill(x, y, x + w, y + h, bg);
    fill(x, y, x + w, y + 1, border);
    fill(x, y + h - 1, x + w, y + h, border);
    fill(x, y, x + 1, y + h, border);
    fill(x + w - 1, y, x + w, y + h, border);
}

void LinkAccountScreen::drawIconTexture(const std::string& path, int x, int y, int w, int h, unsigned int color)
{
    TextureId tid = minecraft->textures->loadTexture(path, true);
    if (tid <= 0) return;
    minecraft->textures->bind(tid);

    glEnable2(GL_TEXTURE_2D);
    glEnable2(GL_BLEND);
    glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Tesselator& t = Tesselator::instance;
    t.begin();
    const int abgr = (color & 0xff00ff00) | ((color & 0xff0000) >> 16) | ((color & 0xff) << 16);
    t.colorABGR(abgr);
    t.vertexUV((float)x,       (float)(y + h), 0.0f, 0.0f, 1.0f);
    t.vertexUV((float)(x + w), (float)(y + h), 0.0f, 1.0f, 1.0f);
    t.vertexUV((float)(x + w), (float)y,       0.0f, 1.0f, 0.0f);
    t.vertexUV((float)x,       (float)y,       0.0f, 0.0f, 0.0f);
    t.draw();
}

// ─── Main Render ─────────────────────────────────────────────────────────────

void LinkAccountScreen::render(int xm, int ym, float a)
{
    renderBackground();
    fill(0, 0, width, height, 0x60000000);

    // Top Bar Header (matches ProfileScreen)
    _btnBack.render(minecraft, xm, ym);
    int headerTextX = _btnBack.x + _btnBack.width + 10;
    drawString(font, I18n::get("linkAccount.title"), headerTextX, 5, 0xffffffff);
    drawScaledString(I18n::get("linkAccount.subtitle"), (float)headerTextX, 16.0f, 0xff888888, 0.8f);

    // Central Card
    drawPanel(_panelX, _panelY, _panelW, _panelH, 0xd815171a, 0xff2d3035);

    // Card Title
    drawString(font, I18n::get("linkAccount.cardTitle"), _panelX + 8, _panelY + 6, 0xffffcc00);

    // Username Label & Box
    drawString(font, I18n::get("linkAccount.username"), _panelX + 8, _panelY + 17, 0xffdddddd);
    tUsername.render(minecraft, xm, ym);

    // Password Label & Box
    drawString(font, I18n::get("linkAccount.password"), _panelX + 8, _panelY + 47, 0xffdddddd);
    tPassword.render(minecraft, xm, ym);

    // Eye Button
    bool eyeHover = (xm >= _eyeX && xm <= _eyeX + _eyeW && ym >= _eyeY && ym <= _eyeY + _eyeH);
    drawPanel(_eyeX, _eyeY, _eyeW, _eyeH, eyeHover ? 0x90282a2e : 0x60101214, eyeHover ? 0xffffcc00 : 0xff484c52);
    drawIconTexture(tPassword.isPassword ? "gui/user/eye.png" : "gui/user/eye_slash.png", _eyeX + 3, _eyeY + 3, 12, 12, 0xffffffff);

    // Remember Me Checkbox
    fill(_cbX, _cbY, _cbX + _cbSize, _cbY + _cbSize, 0x60101214);
    fill(_cbX, _cbY, _cbX + _cbSize, _cbY + 1, 0xff484c52);
    fill(_cbX, _cbY + _cbSize - 1, _cbX + _cbSize, _cbY + _cbSize, 0xff484c52);
    fill(_cbX, _cbY, _cbX + 1, _cbY + _cbSize, 0xff484c52);
    fill(_cbX + _cbSize - 1, _cbY, _cbX + _cbSize, _cbY + _cbSize, 0xff484c52);
    if (_rememberMe) {
        fill(_cbX + 2, _cbY + 2, _cbX + _cbSize - 2, _cbY + _cbSize - 2, 0xff55ff55);
    }
    drawScaledString(I18n::get("linkAccount.rememberMe"), (float)(_cbX + 12), (float)(_cbY + 1), 0xffaaaaaa, 0.70f);

    // Forgot Password Link
    bool fpHover = (xm >= _fpX && xm <= _fpX + _fpW && ym >= _fpY && ym <= _fpY + _fpH);
    drawScaledString(I18n::get("linkAccount.forgotPassword"), (float)_fpX, (float)_fpY, fpHover ? 0xffffffff : 0xffffcc00, 0.70f);

    // Status / Error message
    if (!_statusMsg.empty()) {
        int sc = _isError ? 0xffff5555 : 0xff55ff55;
        drawScaledString(_statusMsg, (float)(_panelX + 8), (float)(_btnSignIn.y - 7), sc, 0.68f);
    }

    // Sign In Button
    _btnSignIn.msg = _isAuthenticating ? I18n::get("linkAccount.signingIn") : I18n::get("linkAccount.login");
    _btnSignIn.render(minecraft, xm, ym);

    // Separator ── OR ──
    int sepY = _panelY + 113;
    fill(_panelX + 8, sepY + 3, _panelX + (_panelW / 2) - 10, sepY + 4, 0xff404448);
    drawScaledString(I18n::get("linkAccount.or"), (float)(_panelX + (_panelW / 2) - 4), (float)sepY, 0xff707478, 0.70f);
    fill(_panelX + (_panelW / 2) + 10, sepY + 3, _panelX + _panelW - 8, sepY + 4, 0xff404448);

    // Create Account Button (Dark background + Yellow border & text)
    bool cHover = (xm >= _createBtnX && xm <= _createBtnX + _createBtnW && ym >= _createBtnY && ym <= _createBtnY + _createBtnH);
    drawPanel(_createBtnX, _createBtnY, _createBtnW, _createBtnH, cHover ? 0xd824262a : 0xd8141618, cHover ? 0xffffe066 : 0xffffcc00);
    std::string cText = I18n::get("linkAccount.createAccount");
    int ctw = (int)(font->width(cText) * 0.75f);
    drawScaledString(cText, (float)(_createBtnX + (_createBtnW - ctw) / 2), (float)(_createBtnY + 5), 0xffffcc00, 0.75f);

    // Bottom Info Box
    int noteX = _panelX + 8;
    int noteY = _panelY + 146;
    int noteW = _panelW - 16;
    int noteH = 26;

    fill(noteX, noteY, noteX + noteW, noteY + noteH, 0x500f1c29);
    fill(noteX, noteY, noteX + noteW, noteY + 1, 0xff1e3a5a);
    fill(noteX, noteY + noteH - 1, noteX + noteW, noteY + noteH, 0xff1e3a5a);
    fill(noteX, noteY, noteX + 1, noteY + noteH, 0xff1e3a5a);
    fill(noteX + noteW - 1, noteY, noteX + noteW, noteY + noteH, 0xff1e3a5a);

    int noteIconSize = 10;
    drawIconTexture("gui/user/notice.png", noteX + 5, noteY + (noteH - noteIconSize) / 2, noteIconSize, noteIconSize, 0xff38bdf8);

    drawScaledString(I18n::get("linkAccount.termsNotice1"), (float)(noteX + 18), (float)(noteY + 3), 0xff88a0b8, 0.68f);

    float spaceW = (float)font->width(" ") * 0.68f;
    float l2X = (float)(noteX + 18);
    float l2Y = (float)(noteY + 12);
    drawScaledString(I18n::get("linkAccount.termsNoticeTerms"), l2X, l2Y, 0xffffcc00, 0.68f);
    l2X += font->width(I18n::get("linkAccount.termsNoticeTerms")) * 0.68f + spaceW;

    drawScaledString(I18n::get("linkAccount.termsNoticeAnd"), l2X, l2Y, 0xff88a0b8, 0.68f);
    l2X += font->width(I18n::get("linkAccount.termsNoticeAnd")) * 0.68f + spaceW;

    drawScaledString(I18n::get("linkAccount.termsNoticePrivacy"), l2X, l2Y, 0xffffcc00, 0.68f);
}

// ─── Interaction Handlers ───────────────────────────────────────────────────

void LinkAccountScreen::buttonClicked(Button* button)
{
    if (button == &_btnBack) {
        minecraft->setScreen(new ProfileScreen());
        return;
    }

    if (button == &_btnSignIn && !_isAuthenticating) {
        if (tUsername.text.empty() || tPassword.text.empty()) {
            _statusMsg = I18n::get("linkAccount.errorEmpty");
            _isError = true;
            return;
        }

        _statusMsg = I18n::get("linkAccount.signingIn");
        _isError = false;
        _isAuthenticating = true;

        std::string err;
        bool ok = AccountManager::loginWithSupabase(tUsername.text, tPassword.text, err);

        _isAuthenticating = false;

        if (ok) {
            minecraft->setScreen(new ProfileScreen());
        } else {
            _statusMsg = err.empty() ? I18n::get("linkAccount.error") : err;
            _isError = true;
        }
    }
}

void LinkAccountScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == 1) { // Left click
        // Eye button toggle
        if (x >= _eyeX && x <= _eyeX + _eyeW && y >= _eyeY && y <= _eyeY + _eyeH) {
            tPassword.isPassword = !tPassword.isPassword;
            return;
        }

        // Remember me toggle
        if (x >= _cbX && x <= _cbX + 110 && y >= _cbY && y <= _cbY + _cbSize + 4) {
            _rememberMe = !_rememberMe;
            return;
        }

        // Forgot Password link
        if (x >= _fpX && x <= _fpX + _fpW && y >= _fpY && y <= _fpY + _fpH) {
            minecraft->platform()->openURL("http://localhost:5173/pages/forgot-password.html");
            return;
        }

        // Create Account button
        if (x >= _createBtnX && x <= _createBtnX + _createBtnW && y >= _createBtnY && y <= _createBtnY + _createBtnH) {
            minecraft->platform()->openURL("http://localhost:5173/pages/register.html");
            return;
        }

        // TextBoxes focus
        tUsername.mouseClicked(minecraft, x, y, buttonNum);
        tPassword.mouseClicked(minecraft, x, y, buttonNum);
    }

    Screen::mouseClicked(x, y, buttonNum);
}

void LinkAccountScreen::keyPressed(int eventKey)
{
    if (eventKey == 1 || eventKey == 27) { // Escape
        minecraft->setScreen(new ProfileScreen());
        return;
    }

    // Enter submits the form
    if (eventKey == 28 || eventKey == 13) {
        buttonClicked(&_btnSignIn);
        return;
    }

    // Tab key switches between username and password fields
    if (eventKey == 15) {
        if (tUsername.focused) {
            tUsername.loseFocus(minecraft);
            tPassword.setFocus(minecraft);
        } else if (tPassword.focused) {
            tPassword.loseFocus(minecraft);
            tUsername.setFocus(minecraft);
        }
        return;
    }

    Screen::keyPressed(eventKey);
}

bool LinkAccountScreen::handleBackEvent(bool isDown)
{
    if (!isDown) {
        minecraft->setScreen(new ProfileScreen());
    }
    return true;
}
