#include "UsernameScreen.h"
#include "ProfileScreen.h"
#include "StartMenuScreen.h"
#include "../../Minecraft.h"
#include "../Font.h"
#include "../components/Button.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../AppPlatform.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../renderer/GuiShader.h"
#include "locale/I18n.h"
#include <cctype>
#include <algorithm>

UsernameScreen::UsernameScreen()
:   _btnBack(1, I18n::get("profile.back")),
    _btnContinue(2, I18n::get("username.continue")),
    tUsername(10, ""),
    _hasExistingName(false),
    _panelX(0), _panelY(0), _panelW(0), _panelH(0)
{
}

UsernameScreen::~UsernameScreen()
{
}

void UsernameScreen::init()
{
    buttons.clear();
    tabButtons.clear();
    textBoxes.clear();

    _btnBack.msg = I18n::get("profile.back");
    _btnContinue.msg = I18n::get("username.continue");

    std::string current = minecraft->options.getStringValue(OPTIONS_USERNAME);
    _hasExistingName = !current.empty();

    if (tUsername.text.empty() && !current.empty()) {
        tUsername.text = current;
    }

    _btnBack.active = _hasExistingName;
    _btnContinue.active = isAllValid();

    buttons.push_back(&_btnBack);
    buttons.push_back(&_btnContinue);

    tabButtons.push_back(&_btnBack);
    tabButtons.push_back(&_btnContinue);

    tUsername.hint = I18n::get("username.placeholder");
    tUsername.isPassword = false;

    textBoxes.push_back(&tUsername);

    setupPositions();
}

void UsernameScreen::setupPositions()
{
    int topBarH = 28;
    int margin = 6;

    // Top bar < Back button
    _btnBack.x = margin;
    _btnBack.y = 4;
    _btnBack.width = 46;
    _btnBack.height = 20;

    // Central card panel
    _panelW = std::min(224, width - 24);
    _panelH = 166;
    _panelX = (width - _panelW) / 2;
    _panelY = std::max(topBarH + 4, (height - _panelH) / 2 + 10);

    // Inputs inside panel
    int innerX = _panelX + 8;
    int innerW = _panelW - 16;

    tUsername.x = innerX;
    tUsername.y = _panelY + 44;
    tUsername.width = innerW;
    tUsername.height = 18;

    // Continue button
    _btnContinue.x = innerX;
    _btnContinue.y = _panelY + 100;
    _btnContinue.width = innerW;
    _btnContinue.height = 19;
}

void UsernameScreen::tick()
{
    tUsername.tick(minecraft);
    _btnContinue.active = isAllValid();
}

// ─── Validation ─────────────────────────────────────────────────────────────

bool UsernameScreen::isLengthValid() const
{
    size_t len = tUsername.text.length();
    return (len >= 3 && len <= 16);
}

bool UsernameScreen::areCharsValid() const
{
    if (tUsername.text.empty()) return false;
    for (char c : tUsername.text) {
        if (!isalnum((unsigned char)c) && c != '_') return false;
    }
    return true;
}

bool UsernameScreen::isStartLetterValid() const
{
    if (tUsername.text.empty()) return false;
    return isalpha((unsigned char)tUsername.text[0]) != 0;
}

bool UsernameScreen::isAllValid() const
{
    return isLengthValid() && areCharsValid() && isStartLetterValid();
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

void UsernameScreen::drawScaledString(const std::string& str, float x, float y, int color, float scale)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    font->drawShadow(str, 0, 0, color);
    glPopMatrix();
}

void UsernameScreen::drawPanel(int x, int y, int w, int h, unsigned int bg, unsigned int border)
{
    fill(x, y, x + w, y + h, bg);
    fill(x, y, x + w, y + 1, border);
    fill(x, y + h - 1, x + w, y + h, border);
    fill(x, y, x + 1, y + h, border);
    fill(x + w - 1, y, x + w, y + h, border);
}

void UsernameScreen::drawIconTexture(const std::string& path, int x, int y, int w, int h, unsigned int color)
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

void UsernameScreen::drawRuleItem(const std::string& text, int x, int y, bool valid)
{
    unsigned int iconColor = valid ? 0xff55ff55 : 0xff777777;
    unsigned int textColor = valid ? 0xffa0c8a0 : 0xff888888;

    if (valid) {
        // Pixel checkmark
        fill(x,     y + 3, x + 1, y + 5, 0xff55ff55);
        fill(x + 1, y + 4, x + 2, y + 6, 0xff55ff55);
        fill(x + 2, y + 3, x + 3, y + 5, 0xff55ff55);
        fill(x + 3, y + 2, x + 4, y + 4, 0xff55ff55);
        fill(x + 4, y + 1, x + 5, y + 3, 0xff55ff55);
        fill(x + 5, y,     x + 6, y + 2, 0xff55ff55);
    } else {
        // Subtle dot
        fill(x + 1, y + 3, x + 4, y + 4, 0xff555555);
    }

    drawScaledString(text, (float)(x + 9), (float)y, textColor, 0.70f);
}

// ─── Main Render ─────────────────────────────────────────────────────────────

void UsernameScreen::render(int xm, int ym, float a)
{
    renderBackground();
    fill(0, 0, width, height, 0x60000000);

    // Top Bar Header
    _btnBack.render(minecraft, xm, ym);
    int headerTextX = _btnBack.x + _btnBack.width + 10;
    drawString(font, I18n::get("username.title"), headerTextX, 5, 0xffffffff);
    drawScaledString(I18n::get("username.subtitle"), (float)headerTextX, 16.0f, 0xff888888, 0.8f);

    // Central Card
    drawPanel(_panelX, _panelY, _panelW, _panelH, 0xd815171a, 0xff2d3035);

    // Card Title
    drawString(font, I18n::get("username.cardTitle"), _panelX + 8, _panelY + 6, 0xffffcc00);

    // User Info Header inside card
    int userIconSize = 10;
    drawIconTexture("gui/user/user_icon.png", _panelX + 8, _panelY + 18, userIconSize, userIconSize, 0xffffffff);
    drawString(font, I18n::get("username.label"), _panelX + 22, _panelY + 16, 0xffffffff);
    drawScaledString(I18n::get("username.desc1"), (float)(_panelX + 22), (float)(_panelY + 25), 0xff888888, 0.68f);
    drawScaledString(I18n::get("username.desc2"), (float)(_panelX + 22), (float)(_panelY + 33), 0xff888888, 0.68f);

    // Username Input Box
    tUsername.render(minecraft, xm, ym);

    // Dynamic Validation Rules
    int rulesX = _panelX + 8;
    drawRuleItem(I18n::get("username.ruleLength"), rulesX, _panelY + 66, isLengthValid());
    drawRuleItem(I18n::get("username.ruleChars"),  rulesX, _panelY + 76, areCharsValid());
    drawRuleItem(I18n::get("username.ruleStart"),  rulesX, _panelY + 86, isStartLetterValid());

    // Continue Button
    _btnContinue.active = isAllValid();
    _btnContinue.render(minecraft, xm, ym);

    // Bottom Info Box
    int noteX = _panelX + 8;
    int noteY = _panelY + 125;
    int noteW = _panelW - 16;
    int noteH = 26;

    fill(noteX, noteY, noteX + noteW, noteY + noteH, 0x500f1c29);
    fill(noteX, noteY, noteX + noteW, noteY + 1, 0xff1e3a5a);
    fill(noteX, noteY + noteH - 1, noteX + noteW, noteY + noteH, 0xff1e3a5a);
    fill(noteX, noteY, noteX + 1, noteY + noteH, 0xff1e3a5a);
    fill(noteX + noteW - 1, noteY, noteX + noteW, noteY + noteH, 0xff1e3a5a);

    int noteIconSize = 10;
    drawIconTexture("gui/user/notice.png", noteX + 5, noteY + (noteH - noteIconSize) / 2, noteIconSize, noteIconSize, 0xff38bdf8);

    drawScaledString(I18n::get("username.notice1"), (float)(noteX + 18), (float)(noteY + 4), 0xff88a0b8, 0.68f);
    drawScaledString(I18n::get("username.notice2"), (float)(noteX + 18), (float)(noteY + 13), 0xff88a0b8, 0.68f);
}

// ─── Interaction Handlers ───────────────────────────────────────────────────

void UsernameScreen::buttonClicked(Button* button)
{
    if (button == &_btnBack && _hasExistingName) {
        minecraft->setScreen(new ProfileScreen());
        return;
    }

    if (button == &_btnContinue && isAllValid()) {
        minecraft->options.set(OPTIONS_USERNAME, tUsername.text);
        minecraft->options.save();

        if (_hasExistingName) {
            minecraft->setScreen(new ProfileScreen());
        } else {
            minecraft->setScreen(NULL); // Proceed to StartMenuScreen
        }
    }
}

void UsernameScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == 1) {
        tUsername.mouseClicked(minecraft, x, y, buttonNum);
    }
    Screen::mouseClicked(x, y, buttonNum);
}

void UsernameScreen::keyPressed(int eventKey)
{
    if ((eventKey == 1 || eventKey == 27) && _hasExistingName) { // Escape
        minecraft->setScreen(new ProfileScreen());
        return;
    }

    if (eventKey == Keyboard::KEY_RETURN || eventKey == 28 || eventKey == 13) {
        if (isAllValid()) {
            buttonClicked(&_btnContinue);
            return;
        }
    }

    Screen::keyPressed(eventKey);
}

bool UsernameScreen::handleBackEvent(bool isDown)
{
    if (!isDown && _hasExistingName) {
        minecraft->setScreen(new ProfileScreen());
    }
    return true;
}

void UsernameScreen::removed()
{
    minecraft->platform()->hideKeyboard();
}
