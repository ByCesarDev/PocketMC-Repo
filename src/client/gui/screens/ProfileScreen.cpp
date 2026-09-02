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
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../renderer/TextureData.h"
#include "../../renderer/GuiShader.h"
#include "locale/I18n.h"
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

ProfileScreen::ProfileScreen()
:   _btnBack(1, I18n::get("profile.back")),
    _selectedTab(0),
    _copiedFeedbackTicks(0),
    _leftPanelX(0), _leftPanelY(0), _leftPanelW(0), _leftPanelH(0),
    _rightPanelX(0), _rightPanelY(0), _rightPanelW(0), _rightPanelH(0)
{
}

ProfileScreen::~ProfileScreen()
{
}

void ProfileScreen::init()
{
    buttons.clear();
    tabButtons.clear();

    _btnBack.msg = I18n::get("profile.back");

    buttons.push_back(&_btnBack);
    tabButtons.push_back(&_btnBack);

    setupPositions();
}

void ProfileScreen::tick()
{
    if (_copiedFeedbackTicks > 0) {
        _copiedFeedbackTicks--;
    }
}

void ProfileScreen::setupPositions()
{
    int topBarH = 28;
    int margin  = 6;

    // Top bar buttons
    _btnBack.x = margin;
    _btnBack.y = 4;
    _btnBack.width = 46;
    _btnBack.height = 20;

    // Panels layout
    int contentY = topBarH + 2;
    int contentH = height - contentY - margin;

    _leftPanelX = margin;
    _leftPanelY = contentY;
    _leftPanelW = std::max(105, (int)(width * 0.23f));
    _leftPanelH = contentH;

    _rightPanelX = _leftPanelX + _leftPanelW + margin;
    _rightPanelY = contentY;
    _rightPanelW = width - _rightPanelX - margin;
    _rightPanelH = contentH;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

void ProfileScreen::drawScaledString(const std::string& str, float x, float y, int color, float scale)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    font->drawShadow(str, 0, 0, color);
    glPopMatrix();
}

void ProfileScreen::drawGoldButton(int x, int y, int w, int h, const std::string& text, bool hover, float scale)
{
    // Background fill
    fill(x, y, x + w, y + h, hover ? 0xfff0b020 : 0xffd99b16);
    // Highlight top/left border
    fill(x, y, x + w, y + 1, 0xfff5c542);
    fill(x, y, x + 1, y + h, 0xfff5c542);
    // Shadow bottom/right border
    fill(x, y + h - 1, x + w, y + h, 0xff9e6c07);
    fill(x + w - 1, y, x + w, y + h, 0xff9e6c07);

    // Text with soft grayish-gold shadow (no harsh black shadow)
    glPushMatrix();
    float textX = (float)(x + 6);
    float textY = (float)(y + (h - 8.0f * scale) / 2.0f);
    glTranslatef(textX, textY, 0.0f);
    glScalef(scale, scale, 1.0f);

    font->draw(text, 1.0f, 1.0f, 0xff8c7040);
    font->draw(text, 0.0f, 0.0f, 0xff1c1202);
    glPopMatrix();

    // Right chevron >
    glPushMatrix();
    float arrX = (float)(x + w - 9);
    float arrY = (float)(y + (h - 8.0f * scale) / 2.0f);
    glTranslatef(arrX, arrY, 0.0f);
    glScalef(scale, scale, 1.0f);
    font->draw(">", 1.0f, 1.0f, 0xff8c7040);
    font->draw(">", 0.0f, 0.0f, 0xff1c1202);
    glPopMatrix();
}

void ProfileScreen::copyToClipboard(const std::string& text)
{
#ifdef _WIN32
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hMem) {
            char* p = static_cast<char*>(GlobalLock(hMem));
            if (p) {
                memcpy(p, text.c_str(), text.size() + 1);
                GlobalUnlock(hMem);
                SetClipboardData(CF_TEXT, hMem);
            }
        }
        CloseClipboard();
    }
#endif
    _copiedFeedbackTicks = 40; // ~2 seconds of visual feedback
}

std::string ProfileScreen::maskEmail(const std::string& email)
{
    if (email.empty()) return "No disponible";
    size_t atPos = email.find('@');
    if (atPos == std::string::npos || atPos <= 2) return email;
    return email.substr(0, 2) + "***" + email.substr(atPos);
}

void ProfileScreen::drawPanel(int x, int y, int w, int h, unsigned int bg, unsigned int border)
{
    fill(x, y, x + w, y + h, bg);
    fill(x, y, x + w, y + 1, border);
    fill(x, y + h - 1, x + w, y + h, border);
    fill(x, y, x + 1, y + h, border);
    fill(x + w - 1, y, x + w, y + h, border);
}

void ProfileScreen::drawIconTexture(const std::string& path, int x, int y, int w, int h, unsigned int color)
{
    TextureId tid = minecraft->textures->loadTexture(path, true);
    if (tid <= 0) return;
    minecraft->textures->bind(tid);

    glEnable2(GL_TEXTURE_2D);
    glEnable2(GL_BLEND);
    glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float r = ((color >> 16) & 0xff) / 255.0f;
    float g = ((color >> 8) & 0xff) / 255.0f;
    float b = (color & 0xff) / 255.0f;
    float a = ((color >> 24) & 0xff) / 255.0f;
    if (a == 0.0f) a = 1.0f;
    glColor4f2(r, g, b, a);

    Tesselator& t = Tesselator::instance;
    t.begin();
    t.vertexUV((float)x,       (float)(y + h), 0.0f, 0.0f, 1.0f);
    t.vertexUV((float)(x + w), (float)(y + h), 0.0f, 1.0f, 1.0f);
    t.vertexUV((float)(x + w), (float)y,       0.0f, 1.0f, 0.0f);
    t.vertexUV((float)x,       (float)y,       0.0f, 0.0f, 0.0f);
    t.draw();
}

void ProfileScreen::drawSkinFace(int x, int y, int size)
{
    std::string chosenSkin = minecraft->options.getStringValue(OPTIONS_SKIN);
    std::string skinPath = (chosenSkin.empty() || chosenSkin == "Default") ? "mob/char.png" : chosenSkin;
    TextureId tid = minecraft->textures->loadTexture(skinPath, true);
    if (tid <= 0) return;

    minecraft->textures->bind(tid);
    const TextureData* tdata = minecraft->textures->getTemporaryTextureData(tid);
    float texW = (tdata && tdata->w > 0) ? (float)tdata->w : 64.0f;
    float texH = (tdata && tdata->h > 0) ? (float)tdata->h : 64.0f;
    float us = 1.0f / texW;
    float vs = 1.0f / texH;

    glEnable2(GL_TEXTURE_2D);
    glEnable2(GL_BLEND);
    glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);

    Tesselator& t = Tesselator::instance;
    auto drawQuad = [&](float px, float py, float pw, float ph, float u, float v, float uw, float vh) {
        t.begin();
        t.vertexUV(px,      py + ph, 0.0f, u * us,         (v + vh) * vs);
        t.vertexUV(px + pw, py + ph, 0.0f, (u + uw) * us, (v + vh) * vs);
        t.vertexUV(px + pw, py,      0.0f, (u + uw) * us, v * vs);
        t.vertexUV(px,      py,      0.0f, u * us,         v * vs);
        t.draw();
    };

    drawQuad((float)x, (float)y, (float)size, (float)size, 8.0f, 8.0f, 8.0f, 8.0f);
    drawQuad((float)x, (float)y, (float)size, (float)size, 40.0f, 8.0f, 8.0f, 8.0f);
}

void ProfileScreen::drawSkinBust(int x, int y, int size)
{
    std::string chosenSkin = minecraft->options.getStringValue(OPTIONS_SKIN);
    std::string skinPath = (chosenSkin.empty() || chosenSkin == "Default") ? "mob/char.png" : chosenSkin;
    TextureId tid = minecraft->textures->loadTexture(skinPath, true);
    if (tid <= 0) return;

    minecraft->textures->bind(tid);
    const TextureData* tdata = minecraft->textures->getTemporaryTextureData(tid);
    float texW = (tdata && tdata->w > 0) ? (float)tdata->w : 64.0f;
    float texH = (tdata && tdata->h > 0) ? (float)tdata->h : 64.0f;
    float us = 1.0f / texW;
    float vs = 1.0f / texH;

    glEnable2(GL_TEXTURE_2D);
    glEnable2(GL_BLEND);
    glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);

    Tesselator& t = Tesselator::instance;
    auto drawQuad = [&](float px, float py, float pw, float ph, float u, float v, float uw, float vh) {
        t.begin();
        t.vertexUV(px,      py + ph, 0.0f, u * us,         (v + vh) * vs);
        t.vertexUV(px + pw, py + ph, 0.0f, (u + uw) * us, (v + vh) * vs);
        t.vertexUV(px + pw, py,      0.0f, (u + uw) * us, v * vs);
        t.vertexUV(px,      py,      0.0f, u * us,         v * vs);
        t.draw();
    };

    float unit = (float)size / 16.0f;
    float headW = 8.0f * unit;
    float headH = 8.0f * unit;
    float headX = (float)x + ((float)size - headW) / 2.0f;
    float headY = (float)y + 1.0f * unit;

    float bodyW = 16.0f * unit;
    float bodyH = 7.0f * unit;
    float bodyX = (float)x + ((float)size - bodyW) / 2.0f;
    float bodyY = headY + headH;

    drawQuad(headX, headY, headW, headH, 8.0f, 8.0f, 8.0f, 8.0f);
    drawQuad(headX, headY, headW, headH, 40.0f, 8.0f, 8.0f, 8.0f);

    drawQuad(bodyX, bodyY, 4.0f * unit, bodyH, 44.0f, 20.0f, 4.0f, 6.0f);
    if (texH >= 64) drawQuad(bodyX, bodyY, 4.0f * unit, bodyH, 44.0f, 36.0f, 4.0f, 6.0f);

    drawQuad(bodyX + 4.0f * unit, bodyY, 8.0f * unit, bodyH, 20.0f, 20.0f, 8.0f, 6.0f);
    if (texH >= 64) drawQuad(bodyX + 4.0f * unit, bodyY, 8.0f * unit, bodyH, 20.0f, 36.0f, 8.0f, 6.0f);

    if (texH >= 64) {
        drawQuad(bodyX + 12.0f * unit, bodyY, 4.0f * unit, bodyH, 36.0f, 52.0f, 4.0f, 6.0f);
        drawQuad(bodyX + 12.0f * unit, bodyY, 4.0f * unit, bodyH, 52.0f, 52.0f, 4.0f, 6.0f);
    } else {
        drawQuad(bodyX + 12.0f * unit, bodyY, 4.0f * unit, bodyH, 44.0f, 20.0f, 4.0f, 6.0f);
    }
}

void ProfileScreen::drawNavTab(int x, int y, int w, int h, int index, const std::string& iconPath, const std::string& title, const std::string& subtitle, bool isSelected, int xm, int ym)
{
    bool hover = (xm >= x && xm <= x + w && ym >= y && ym <= y + h);

    if (isSelected) {
        fill(x, y, x + w, y + h, 0x85242216);
        fill(x, y, x + w, y + 1, 0xffffcc00);
        fill(x, y + h - 1, x + w, y + h, 0xffffcc00);
        fill(x, y, x + 1, y + h, 0xffffcc00);
        fill(x + w - 1, y, x + w, y + h, 0xffffcc00);
        drawString(font, ">", x + w - 8, y + (h - 8) / 2, 0xffffcc00);
    } else if (hover) {
        fill(x, y, x + w, y + h, 0x40ffffff);
    }

    int textX = x + 24;
    int blockY = y + (h - 17) / 2;

    if (index == 0) {
        drawSkinFace(x + 5, y + (h - 14) / 2, 14);
    } else if (!iconPath.empty()) {
        drawIconTexture(iconPath, x + 6, y + (h - 12) / 2, 12, 12, isSelected ? 0xffffcc00 : 0xffcccccc);
    }

    drawString(font, title, textX, blockY, isSelected ? 0xffffcc00 : 0xffffffff);
    drawScaledString(subtitle, (float)textX, (float)(blockY + 10), isSelected ? 0xffd4c890 : 0xff707478, 0.75f);
}

void ProfileScreen::drawInfoCard(int x, int y, int w, int h, const std::string& iconPath, const std::string& label, const std::string& val, int valColor)
{
    drawPanel(x, y, w, h, 0xd016181b, 0xff2e3136);

    int iconSize = 14;
    if (!iconPath.empty()) {
        int iconY = y + (h - iconSize) / 2;
        drawIconTexture(iconPath, x + 6, iconY, iconSize, iconSize, 0xffffffff);
    }

    int tx = x + (iconPath.empty() ? 8 : 24);
    int blockY = y + (h - 17) / 2;
    drawScaledString(label, (float)tx, (float)blockY, 0xff888888, 0.75f);
    drawString(font, val, tx, blockY + 8, valColor);
}

void ProfileScreen::drawActionCard(int x, int y, int w, int h, const std::string& iconPath, const std::string& title, const std::string& subtitle, int xm, int ym)
{
    bool hover = (xm >= x && xm <= x + w && ym >= y && ym <= y + h);
    unsigned int bg = hover ? 0xd022262a : 0xd016181b;
    unsigned int border = hover ? 0xffffcc00 : 0xff2e3136;

    drawPanel(x, y, w, h, bg, border);

    int iconSize = 12;
    if (!iconPath.empty()) {
        int iconY = y + (h - iconSize) / 2;
        drawIconTexture(iconPath, x + 6, iconY, iconSize, iconSize, hover ? 0xffffcc00 : 0xffffffff);
    }

    int tx = x + (iconPath.empty() ? 6 : 22);
    int blockY = y + (h - 16) / 2;
    drawString(font, title, tx, blockY, hover ? 0xffffcc00 : 0xffffffff);
    drawScaledString(subtitle, (float)tx, (float)(blockY + 9), 0xff888888, 0.68f);

    drawString(font, ">", x + w - 8, y + (h - 8) / 2, hover ? 0xffffcc00 : 0xff7e8288);
}

// ─── Main Render ─────────────────────────────────────────────────────────────

void ProfileScreen::render(int xm, int ym, float a)
{
    renderBackground();
    fill(0, 0, width, height, 0x60000000);

    // Top Bar
    int headerTextX = _btnBack.x + _btnBack.width + 10;
    drawString(font, I18n::get("profile.title"), headerTextX, 5, 0xffffffff);
    drawScaledString(I18n::get("profile.subtitle"), (float)headerTextX, 16.0f, 0xff888888, 0.8f);

    // Left Navigation Panel (4 Tabs)
    drawPanel(_leftPanelX, _leftPanelY, _leftPanelW, _leftPanelH, 0xd8141618, 0xff282a2e);

    int tabH = (_leftPanelH - 12) / 4;
    tabH = std::min(32, std::max(24, tabH));
    int tabY = _leftPanelY + 6;

    drawNavTab(_leftPanelX + 4, tabY, _leftPanelW - 8, tabH, 0, "", I18n::get("profile.tab.summary"), I18n::get("profile.tab.summary.desc"), _selectedTab == 0, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + tabH + 2, _leftPanelW - 8, tabH, 1, "gui/user/user_icon.png", I18n::get("profile.tab.account"), I18n::get("profile.tab.account.desc"), _selectedTab == 1, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + (tabH + 2) * 2, _leftPanelW - 8, tabH, 2, "gui/user/user_icon.png", I18n::get("profile.tab.identity"), I18n::get("profile.tab.identity.desc"), _selectedTab == 2, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + (tabH + 2) * 3, _leftPanelW - 8, tabH, 3, "gui/user/lock.png", I18n::get("profile.tab.security"), I18n::get("profile.tab.security.desc"), _selectedTab == 3, xm, ym);

    // Right Content Area — Switch based on _selectedTab
    switch (_selectedTab) {
        case 0: renderTabResumen(xm, ym); break;
        case 1: renderTabCuenta(xm, ym); break;
        case 2: renderTabIdentidad(xm, ym); break;
        case 3: renderTabSeguridad(xm, ym); break;
        default: renderTabResumen(xm, ym); break;
    }

    _btnBack.render(minecraft, xm, ym);
}

// ─── TAB 0: Resumen ──────────────────────────────────────────────────────────

void ProfileScreen::renderTabResumen(int xm, int ym)
{
    const PlayerIdentity& id = AccountManager::getIdentity();
    bool isOnline = AccountManager::isOnline();
    std::string localUname = minecraft->options.getStringValue(OPTIONS_USERNAME);
    std::string activeUname = AccountManager::getUsername(localUname);
    std::string activeDispName = AccountManager::getDisplayName(localUname);

    int contentW = _rightPanelW;
    int gap = 6;

    int secTopH = 64;
    int secActionH = 46;
    int noteH = 18;

    int sec1Y = _rightPanelY;
    int sec2Y = sec1Y + secTopH + gap;
    int noteY = sec2Y + secActionH + gap;
    int noteX = _rightPanelX + 8;
    int noteW = contentW - 16;

    // ── SECTION 1: TU PERFIL & CUENTA ──
    drawPanel(_rightPanelX, sec1Y, contentW, secTopH, 0xd815171a, 0xff2d3035);
    drawString(font, I18n::get("profile.yourProfile"), _rightPanelX + 8, sec1Y + 5, 0xffffcc00);

    int avatarBoxX = _rightPanelX + 8;
    int avatarBoxY = sec1Y + 14;
    int avatarSize = 44;
    drawPanel(avatarBoxX, avatarBoxY, avatarSize, avatarSize, 0xff101214, 0xff3a3c40);
    drawSkinBust(avatarBoxX + 2, avatarBoxY + 2, avatarSize - 4);

    int profileInfoX = avatarBoxX + avatarSize + 8;
    int profileInfoY = sec1Y + 13;

    drawString(font, activeDispName, profileInfoX, profileInfoY, 0xffffffff);

    int badgeTagY = profileInfoY + 11;
    if (!isOnline) {
        int tagW = 46;
        int tagH = 10;
        fill(profileInfoX, badgeTagY, profileInfoX + tagW, badgeTagY + tagH, 0x25301010);
        fill(profileInfoX, badgeTagY, profileInfoX + tagW, badgeTagY + 1, 0xffaa3333);
        fill(profileInfoX, badgeTagY + tagH - 1, profileInfoX + tagW, badgeTagY + tagH, 0xffaa3333);
        fill(profileInfoX, badgeTagY, profileInfoX + 1, badgeTagY + tagH, 0xffaa3333);
        fill(profileInfoX + tagW - 1, badgeTagY, profileInfoX + tagW, badgeTagY + tagH, 0xffaa3333);
        drawScaledString(I18n::get("profile.offline"), (float)(profileInfoX + 4), (float)(badgeTagY + 2), 0xffff6666, 0.75f);

        drawString(font, I18n::get("profile.localProfile"), profileInfoX, badgeTagY + 12, 0xffdddddd);
        drawScaledString(I18n::get("profile.playingWithoutAccount"), (float)profileInfoX, (float)(badgeTagY + 22), 0xff888888, 0.72f);
    } else {
        int tagW = 42;
        int tagH = 10;
        fill(profileInfoX, badgeTagY, profileInfoX + tagW, badgeTagY + tagH, 0x25103010);
        fill(profileInfoX, badgeTagY, profileInfoX + tagW, badgeTagY + 1, 0xff33aa33);
        fill(profileInfoX, badgeTagY + tagH - 1, profileInfoX + tagW, badgeTagY + tagH, 0xff33aa33);
        fill(profileInfoX, badgeTagY, profileInfoX + 1, badgeTagY + tagH, 0xff33aa33);
        fill(profileInfoX + tagW - 1, badgeTagY, profileInfoX + tagW, badgeTagY + tagH, 0xff33aa33);
        drawScaledString(I18n::get("profile.online"), (float)(profileInfoX + 4), (float)(badgeTagY + 2), 0xff55ff55, 0.75f);

        drawString(font, "@" + activeUname, profileInfoX, badgeTagY + 12, 0xffffcc00);
        drawScaledString(I18n::get("profile.officialAccountLinked"), (float)profileInfoX, (float)(badgeTagY + 22), 0xff888888, 0.72f);
    }

    // Right Sub-Card: CUENTA
    int accountCardW = std::min(148, (int)(contentW * 0.44f));
    int accountCardX = _rightPanelX + contentW - accountCardW - 6;
    int accountCardY = sec1Y + 4;
    int accountCardH = secTopH - 8;
    drawPanel(accountCardX, accountCardY, accountCardW, accountCardH, 0xe0101214, 0xff383b40);

    drawString(font, I18n::get("profile.account"), accountCardX + 8, accountCardY + 4, 0xffffcc00);

    drawScaledString(I18n::get("profile.accountType"), (float)(accountCardX + 8), (float)(accountCardY + 14), 0xff888888, 0.72f);
    drawString(font, isOnline ? I18n::get("profile.online") : I18n::get("profile.offline"), accountCardX + 8, accountCardY + 21, isOnline ? 0xff55ff55 : 0xffff5555);

    drawScaledString(I18n::get("profile.status"), (float)(accountCardX + 8), (float)(accountCardY + 31), 0xff888888, 0.72f);
    drawString(font, isOnline ? I18n::get("profile.linkedActive") : I18n::get("profile.unlinked"), accountCardX + 8, accountCardY + 38, isOnline ? 0xff55ff55 : 0xffffcc00);

    // Golden CTA Button only when Offline
    if (!isOnline) {
        int bx = accountCardX + 5;
        int by = accountCardY + 40;
        int bw = accountCardW - 10;
        int bh = 13;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        drawGoldButton(bx, by, bw, bh, I18n::get("profile.linkAccount"), bHover, 0.68f);
    }

    // ── SECTION 2: ACCIONES RAPIDAS ──
    drawPanel(_rightPanelX, sec2Y, contentW, secActionH, 0xd815171a, 0xff2d3035);
    drawString(font, I18n::get("profile.quickActions"), _rightPanelX + 8, sec2Y + 4, 0xffffcc00);

    int actionCardGap = 4;
    int actionCardY = sec2Y + 14;
    int actionCardH = secActionH - 18;
    int actionCardW = (contentW - 16 - actionCardGap * 2) / 3;

    drawActionCard(_rightPanelX + 8, actionCardY, actionCardW, actionCardH, "gui/user/pencil_edit_icon.png", I18n::get("profile.editLocalName"), I18n::get("profile.editLocalNameDesc"), xm, ym);
    drawActionCard(_rightPanelX + 8 + (actionCardW + actionCardGap), actionCardY, actionCardW, actionCardH, "", I18n::get("profile.exportProfile"), I18n::get("profile.exportProfileDesc"), xm, ym);
    drawActionCard(_rightPanelX + 8 + (actionCardW + actionCardGap) * 2, actionCardY, actionCardW, actionCardH, "", I18n::get("profile.deleteLocalProfile"), I18n::get("profile.deleteLocalProfileDesc"), xm, ym);

    // ── Bottom Note Box ──
    fill(noteX, noteY, noteX + noteW, noteY + noteH, 0x500f1c29);
    fill(noteX, noteY, noteX + noteW, noteY + 1, 0xff1e3a5a);
    fill(noteX, noteY + noteH - 1, noteX + noteW, noteY + noteH, 0xff1e3a5a);
    fill(noteX, noteY, noteX + 1, noteY + noteH, 0xff1e3a5a);
    fill(noteX + noteW - 1, noteY, noteX + noteW, noteY + noteH, 0xff1e3a5a);

    drawString(font, "(i)", noteX + 5, noteY + (noteH - 8) / 2, 0xff38bdf8);
    drawScaledString(I18n::get("profile.bottomNotice1"), (float)(noteX + 18), (float)(noteY + 2), 0xff88a0b8, 0.70f);
    drawScaledString(I18n::get("profile.bottomNotice2"), (float)(noteX + 18), (float)(noteY + 9), 0xff88a0b8, 0.70f);
}

// ─── TAB 1: Cuenta ───────────────────────────────────────────────────────────

void ProfileScreen::renderTabCuenta(int xm, int ym)
{
    const PlayerIdentity& id = AccountManager::getIdentity();
    bool isOnline = AccountManager::isOnline();

    drawPanel(_rightPanelX, _rightPanelY, _rightPanelW, _rightPanelH, 0xd815171a, 0xff2d3035);

    drawString(font, I18n::get("profile.account"), _rightPanelX + 10, _rightPanelY + 8, 0xffffcc00);
    drawScaledString(I18n::get("profile.account.desc"), (float)(_rightPanelX + 10), (float)(_rightPanelY + 20), 0xff888888, 0.8f);

    int cardX = _rightPanelX + 10;
    int cardY = _rightPanelY + 34;
    int cardW = _rightPanelW - 20;
    int cardH = isOnline ? 120 : 92;

    drawPanel(cardX, cardY, cardW, cardH, 0xe0101214, 0xff383b40);

    int lineY = cardY + 10;

    drawScaledString(I18n::get("profile.accountType"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
    drawString(font, isOnline ? I18n::get("profile.online") : I18n::get("profile.offline"), cardX + 12, lineY + 8, isOnline ? 0xff55ff55 : 0xffff5555);
    lineY += 22;

    drawScaledString(I18n::get("profile.status"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
    drawString(font, isOnline ? I18n::get("profile.linkedActive") : I18n::get("profile.unlinked"), cardX + 12, lineY + 8, isOnline ? 0xff55ff55 : 0xffffcc00);
    lineY += 22;

    if (isOnline) {
        drawScaledString(I18n::get("profile.account.email"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, maskEmail("cuentas.oficial.cesardev@gmail.com"), cardX + 12, lineY + 8, 0xffffffff);
        lineY += 22;

        drawScaledString(I18n::get("profile.account.memberSince"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, I18n::get("profile.account.memberSinceVal"), cardX + 12, lineY + 8, 0xffcccccc);
        lineY += 24;

        // Button Administrar cuenta (Web)
        int bw = 150, bh = 18;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        drawGoldButton(bx, by, bw, bh, I18n::get("profile.manageAccountWeb"), bHover, 0.75f);

        // Button Cerrar sesion
        int sbx = bx + bw + 10;
        bool sHover = (xm >= sbx && xm <= sbx + 90 && ym >= by && ym <= by + bh);
        fill(sbx, by, sbx + 90, by + bh, sHover ? 0x80ff4444 : 0x5033363a);
        fill(sbx, by, sbx + 90, by + 1, 0xff555555);
        fill(sbx, by + bh - 1, sbx + 90, by + bh, 0xff555555);
        fill(sbx, by, sbx + 1, by + bh, 0xff555555);
        fill(sbx + 90 - 1, by, sbx + 90, by + bh, 0xff555555);
        drawString(font, I18n::get("profile.signOut"), sbx + 10, by + 5, 0xffffffff);
    } else {
        drawScaledString(I18n::get("profile.account.offlineDesc1"), (float)(cardX + 12), (float)lineY, 0xffaaaaaa, 0.75f);
        drawScaledString(I18n::get("profile.account.offlineDesc2"), (float)(cardX + 12), (float)(lineY + 9), 0xff888888, 0.75f);
        lineY += 24;

        int bw = 170, bh = 20;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        drawGoldButton(bx, by, bw, bh, I18n::get("profile.linkAccount"), bHover, 0.75f);
    }
}

// ─── TAB 2: Identidad ────────────────────────────────────────────────────────

void ProfileScreen::renderTabIdentidad(int xm, int ym)
{
    const PlayerIdentity& id = AccountManager::getIdentity();
    bool isOnline = AccountManager::isOnline();
    std::string localUname = minecraft->options.getStringValue(OPTIONS_USERNAME);
    std::string activeUname = AccountManager::getUsername(localUname);
    std::string activeDispName = AccountManager::getDisplayName(localUname);

    drawPanel(_rightPanelX, _rightPanelY, _rightPanelW, _rightPanelH, 0xd815171a, 0xff2d3035);

    drawString(font, I18n::get("profile.identity.title"), _rightPanelX + 10, _rightPanelY + 8, 0xffffcc00);
    drawScaledString(I18n::get("profile.identity.subtitle"), (float)(_rightPanelX + 10), (float)(_rightPanelY + 20), 0xff888888, 0.8f);

    int cardX = _rightPanelX + 10;
    int cardY = _rightPanelY + 34;
    int cardW = _rightPanelW - 20;
    int cardH = isOnline ? 104 : 88;

    drawPanel(cardX, cardY, cardW, cardH, 0xe0101214, 0xff383b40);

    int lineY = cardY + 10;

    if (!isOnline) {
        drawScaledString(I18n::get("profile.identity.localName"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, localUname, cardX + 12, lineY + 8, 0xffffffff);
        lineY += 22;

        drawScaledString(I18n::get("profile.identity.localDesc"), (float)(cardX + 12), (float)lineY, 0xffaaaaaa, 0.75f);
        lineY += 22;

        int bw = 150, bh = 18;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, bHover ? 0xff404448 : 0xff282a2e);
        fill(bx, by, bx + bw, by + 1, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx, by + bh - 1, bx + bw, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx, by, bx + 1, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx + bw - 1, by, bx + bw, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        drawString(font, I18n::get("profile.editLocalName") + "  >", bx + 8, by + 5, bHover ? 0xffffcc00 : 0xffffffff);
    } else {
        drawScaledString(I18n::get("profile.identity.displayName"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, activeDispName, cardX + 12, lineY + 8, 0xffffffff);
        lineY += 22;

        drawScaledString(I18n::get("profile.identity.uniqueUser"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, "@" + activeUname, cardX + 12, lineY + 8, 0xffffcc00);
        lineY += 22;

        drawScaledString(I18n::get("profile.identity.uuid"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, id.playerId.empty() ? "No disponible" : id.playerId, cardX + 12, lineY + 8, 0xff55ff55);
        lineY += 22;

        // Button Copiar ID
        int bw = 85, bh = 18;
        int bx = cardX + 12, by = lineY;
        bool cHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, _copiedFeedbackTicks > 0 ? 0xff10b981 : (cHover ? 0xff404448 : 0xff282a2e));
        fill(bx, by, bx + bw, by + 1, 0xff555555);
        fill(bx, by + bh - 1, bx + bw, by + bh, 0xff555555);
        fill(bx, by, bx + 1, by + bh, 0xff555555);
        fill(bx + bw - 1, by, bx + bw, by + bh, 0xff555555);
        drawString(font, _copiedFeedbackTicks > 0 ? I18n::get("profile.identity.copied") : I18n::get("profile.identity.copyId"), bx + 10, by + 5, _copiedFeedbackTicks > 0 ? 0xff022315 : 0xffffffff);

        // Button Editar perfil (Web)
        int ebx = bx + bw + 10;
        int ebw = 135;
        bool eHover = (xm >= ebx && xm <= ebx + ebw && ym >= by && ym <= by + bh);
        drawGoldButton(ebx, by, ebw, bh, I18n::get("profile.identity.editWeb"), eHover, 0.75f);
    }
}

// ─── TAB 3: Seguridad ────────────────────────────────────────────────────────

void ProfileScreen::renderTabSeguridad(int xm, int ym)
{
    bool isOnline = AccountManager::isOnline();

    drawPanel(_rightPanelX, _rightPanelY, _rightPanelW, _rightPanelH, 0xd815171a, 0xff2d3035);

    drawString(font, I18n::get("profile.security.title"), _rightPanelX + 10, _rightPanelY + 8, 0xffffcc00);
    drawScaledString(I18n::get("profile.security.subtitle"), (float)(_rightPanelX + 10), (float)(_rightPanelY + 20), 0xff888888, 0.8f);

    int cardX = _rightPanelX + 10;
    int cardY = _rightPanelY + 34;
    int cardW = _rightPanelW - 20;
    int cardH = isOnline ? 104 : 76;

    drawPanel(cardX, cardY, cardW, cardH, 0xe0101214, 0xff383b40);

    int lineY = cardY + 12;

    if (!isOnline) {
        drawIconTexture("gui/user/lock.png", cardX + 12, lineY, 14, 14, 0xffffcc00);
        drawString(font, I18n::get("profile.security.offlineNotice1"), cardX + 30, lineY + 3, 0xffffffff);
        lineY += 20;

        drawScaledString(I18n::get("profile.security.offlineNotice2"), (float)(cardX + 12), (float)lineY, 0xffaaaaaa, 0.72f);
        lineY += 20;

        int bw = 170, bh = 20;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        drawGoldButton(bx, by, bw, bh, I18n::get("profile.linkAccount"), bHover, 0.75f);
    } else {
        drawScaledString(I18n::get("profile.security.activeSession"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, I18n::get("profile.security.activeDevice"), cardX + 12, lineY + 8, 0xff55ff55);
        lineY += 22;

        drawScaledString(I18n::get("profile.security.lastAuth"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, I18n::get("profile.security.today"), cardX + 12, lineY + 8, 0xffffffff);
        lineY += 22;

        drawScaledString(I18n::get("profile.security.password"), (float)(cardX + 12), (float)lineY, 0xff888888, 0.75f);
        drawString(font, "************", cardX + 12, lineY + 8, 0xffaaaaaa);
        lineY += 24;

        // Button Cambiar contraseña (Web)
        int bw = 150, bh = 18;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, bHover ? 0xff404448 : 0xff282a2e);
        fill(bx, by, bx + bw, by + 1, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx, by + bh - 1, bx + bw, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx, by, bx + 1, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx + bw - 1, by, bx + bw, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        drawString(font, I18n::get("profile.security.changePasswordWeb") + "  >", bx + 8, by + 5, bHover ? 0xffffcc00 : 0xffffffff);

        // Button Cerrar sesion
        int sbx = bx + bw + 10;
        bool sHover = (xm >= sbx && xm <= sbx + 90 && ym >= by && ym <= by + bh);
        fill(sbx, by, sbx + 90, by + bh, sHover ? 0x80ff4444 : 0x5033363a);
        fill(sbx, by, sbx + 90, by + 1, 0xff555555);
        fill(sbx, by + bh - 1, sbx + 90, by + bh, 0xff555555);
        fill(sbx, by, sbx + 1, by + bh, 0xff555555);
        fill(sbx + 90 - 1, by, sbx + 90, by + bh, 0xff555555);
        drawString(font, I18n::get("profile.signOut"), sbx + 10, by + 5, 0xffffffff);
    }
}

// ─── Interaction Handlers ───────────────────────────────────────────────────

void ProfileScreen::buttonClicked(Button* button)
{
    if (button == &_btnBack) {
        minecraft->setScreen(new StartMenuScreen());
        return;
    }
}

void ProfileScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == 1) { // Left click
        // 1. Check tab clicks (4 tabs)
        int tabH = (_leftPanelH - 12) / 4;
        tabH = std::min(32, std::max(24, tabH));
        int tabY0 = _leftPanelY + 6;

        for (int i = 0; i < 4; ++i) {
            int ty = tabY0 + i * (tabH + 2);
            if (x >= _leftPanelX + 4 && x <= _leftPanelX + _leftPanelW - 4 &&
                y >= ty && y <= ty + tabH) {
                _selectedTab = i;
                return;
            }
        }

        bool isOnline = AccountManager::isOnline();

        // 2. Tab 0 (Resumen) Interactions
        if (_selectedTab == 0) {
            int contentW = _rightPanelW;
            int gap = 6;
            int secTopH = 64;
            int secActionH = 46;
            int sec1Y = _rightPanelY;
            int sec2Y = sec1Y + secTopH + gap;

            int actionCardGap = 4;
            int actionCardY = sec2Y + 14;
            int actionCardH = secActionH - 18;
            int actionCardW = (contentW - 16 - actionCardGap * 2) / 3;

            // Card 1: Editar nombre local
            if (x >= _rightPanelX + 8 && x <= _rightPanelX + 8 + actionCardW &&
                y >= actionCardY && y <= actionCardY + actionCardH) {
                minecraft->setScreen(new UsernameScreen());
                return;
            }

            // Gold Button click when Offline
            if (!isOnline) {
                int accountCardW = std::min(148, (int)(contentW * 0.44f));
                int accountCardX = _rightPanelX + contentW - accountCardW - 6;
                int accountCardY = sec1Y + 4;
                int bx = accountCardX + 5;
                int by = accountCardY + 40;
                int bw = accountCardW - 10;
                int bh = 13;
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->setScreen(new LinkAccountScreen());
                    return;
                }
            }
        }

        // 3. Tab 1 (Cuenta) Interactions
        else if (_selectedTab == 1) {
            int cardX = _rightPanelX + 10;
            int cardY = _rightPanelY + 34;
            if (isOnline) {
                int lineY = cardY + 10 + 22 * 3 + 24;
                int bx = cardX + 12, by = lineY, bw = 150, bh = 18;
                // Administrar cuenta (Web)
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->platform()->openURL("http://localhost:5173/pages/profile.html");
                    return;
                }
                // Cerrar sesion
                int sbx = bx + bw + 10;
                if (x >= sbx && x <= sbx + 90 && y >= by && y <= by + bh) {
                    AccountManager::logout();
                    init();
                    return;
                }
            } else {
                int lineY = cardY + 10 + 22 * 2 + 24;
                int bx = cardX + 12, by = lineY, bw = 170, bh = 20;
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->setScreen(new LinkAccountScreen());
                    return;
                }
            }
        }

        // 4. Tab 2 (Identidad) Interactions
        else if (_selectedTab == 2) {
            int cardX = _rightPanelX + 10;
            int cardY = _rightPanelY + 34;
            if (!isOnline) {
                int lineY = cardY + 10 + 22 * 2;
                int bx = cardX + 12, by = lineY, bw = 150, bh = 18;
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->setScreen(new UsernameScreen());
                    return;
                }
            } else {
                int lineY = cardY + 10 + 22 * 3;
                int bx = cardX + 12, by = lineY, bw = 85, bh = 18;
                // Copiar ID
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    copyToClipboard(AccountManager::getIdentity().playerId);
                    return;
                }
                // Editar perfil (Web)
                int ebx = bx + bw + 10, ebw = 135;
                if (x >= ebx && x <= ebx + ebw && y >= by && y <= by + bh) {
                    minecraft->platform()->openURL("http://localhost:5173/pages/edit-profile.html");
                    return;
                }
            }
        }

        // 5. Tab 3 (Seguridad) Interactions
        else if (_selectedTab == 3) {
            int cardX = _rightPanelX + 10;
            int cardY = _rightPanelY + 34;
            if (!isOnline) {
                int lineY = cardY + 12 + 20 * 2;
                int bx = cardX + 12, by = lineY, bw = 170, bh = 20;
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->setScreen(new LinkAccountScreen());
                    return;
                }
            } else {
                int lineY = cardY + 12 + 22 * 2 + 24;
                int bx = cardX + 12, by = lineY, bw = 150, bh = 18;
                // Cambiar contrasena (Web)
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->platform()->openURL("http://localhost:5173/pages/forgot-password.html");
                    return;
                }
                // Cerrar sesion
                int sbx = bx + bw + 10;
                if (x >= sbx && x <= sbx + 90 && y >= by && y <= by + bh) {
                    AccountManager::logout();
                    init();
                    return;
                }
            }
        }
    }

    Screen::mouseClicked(x, y, buttonNum);
}

void ProfileScreen::keyPressed(int eventKey)
{
    if (eventKey == 1 || eventKey == 27) { // Escape
        minecraft->setScreen(new StartMenuScreen());
        return;
    }
    Screen::keyPressed(eventKey);
}

bool ProfileScreen::handleBackEvent(bool isDown)
{
    if (!isDown) {
        minecraft->setScreen(new StartMenuScreen());
    }
    return true;
}
