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
:   _btnBack(1, "< Atras"),
    _btnClose(2, "X"),
    _btnAction(3, "Vincular Cuenta PocketMC  >"),
    _btnSignOut(4, "Cerrar sesion"),
    _btnCopyId(5, "Copiar ID"),
    _btnEditProfileWeb(6, "Editar perfil (Web)  >"),
    _btnChangePasswordWeb(7, "Cambiar contrasena (Web)  >"),
    _btnEditLocalName(8, "Editar nombre local  >"),
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

    _btnBack.msg = "< Atras";
    _btnClose.msg = "X";

    buttons.push_back(&_btnBack);
    buttons.push_back(&_btnClose);
    tabButtons.push_back(&_btnBack);
    tabButtons.push_back(&_btnClose);

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

    _btnClose.x = width - margin - 20;
    _btnClose.y = 4;
    _btnClose.width = 20;
    _btnClose.height = 20;

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

    // Right panel sub-cards sizing for Tab 0 (Resumen)
    int secTopH = std::max(75, (int)(_rightPanelH * 0.38f));
    int accountCardW = std::min(150, (int)(_rightPanelW * 0.44f));
    int accountCardX = _rightPanelX + _rightPanelW - accountCardW - 8;
    int accountCardY = _rightPanelY + 6;

    _btnAction.width = accountCardW - 12;
    _btnAction.height = 18;
    _btnAction.x = accountCardX + 6;
    _btnAction.y = accountCardY + secTopH - 26;

    if (AccountManager::isOnline()) {
        _btnAction.y = accountCardY + secTopH - 42;
        _btnSignOut.width = accountCardW - 12;
        _btnSignOut.height = 16;
        _btnSignOut.x = accountCardX + 6;
        _btnSignOut.y = accountCardY + secTopH - 22;
    }
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

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
    int textY = y + 4;

    if (index == 0) {
        drawSkinFace(x + 5, y + (h - 14) / 2, 14);
    } else if (!iconPath.empty()) {
        drawIconTexture(iconPath, x + 6, y + (h - 12) / 2, 12, 12, isSelected ? 0xffffcc00 : 0xffcccccc);
    }

    drawString(font, title, textX, textY, isSelected ? 0xffffcc00 : 0xffffffff);
    drawString(font, subtitle, textX, textY + 11, isSelected ? 0xffb0a880 : 0xff707478);
}

void ProfileScreen::drawInfoCard(int x, int y, int w, int h, const std::string& iconPath, const std::string& label, const std::string& val, int valColor)
{
    drawPanel(x, y, w, h, 0xd016181b, 0xff2e3136);

    int iconSize = 14;
    if (!iconPath.empty()) {
        drawIconTexture(iconPath, x + 6, y + (h - iconSize) / 2, iconSize, iconSize, 0xffffffff);
    }

    int tx = x + (iconPath.empty() ? 8 : 24);
    drawString(font, label, tx, y + 4, 0xff888888);
    drawString(font, val, tx, y + 15, valColor);
}

void ProfileScreen::drawActionCard(int x, int y, int w, int h, const std::string& iconPath, const std::string& title, const std::string& subtitle, int xm, int ym)
{
    bool hover = (xm >= x && xm <= x + w && ym >= y && ym <= y + h);
    unsigned int bg = hover ? 0xd022262a : 0xd016181b;
    unsigned int border = hover ? 0xffffcc00 : 0xff2e3136;

    drawPanel(x, y, w, h, bg, border);

    int iconSize = 14;
    if (!iconPath.empty()) {
        drawIconTexture(iconPath, x + 8, y + (h - iconSize) / 2, iconSize, iconSize, hover ? 0xffffcc00 : 0xffffffff);
    }

    int tx = x + (iconPath.empty() ? 10 : 26);
    drawString(font, title, tx, y + 5, hover ? 0xffffcc00 : 0xffffffff);
    drawString(font, subtitle, tx, y + 15, 0xff888888);

    drawString(font, ">", x + w - 10, y + (h - 8) / 2, hover ? 0xffffcc00 : 0xff7e8288);
}

// ─── Main Render ─────────────────────────────────────────────────────────────

void ProfileScreen::render(int xm, int ym, float a)
{
    renderBackground();
    fill(0, 0, width, height, 0x60000000);

    const PlayerIdentity& id = AccountManager::getIdentity();
    bool isOnline = AccountManager::isOnline();
    std::string localUname = minecraft->options.getStringValue(OPTIONS_USERNAME);
    std::string activeDispName = AccountManager::getDisplayName(localUname);

    // Top Bar
    int headerTextX = _btnBack.x + _btnBack.width + 10;
    drawString(font, "Perfil", headerTextX, 5, 0xffffffff);
    drawString(font, "Administra tu cuenta y tu identidad en PocketMC.", headerTextX, 16, 0xff888888);

    // Right User Badge in Top Bar
    int badgeW = 92;
    int badgeH = 20;
    int badgeX = _btnClose.x - badgeW - 6;
    int badgeY = 4;
    drawPanel(badgeX, badgeY, badgeW, badgeH, 0xa0141618, 0xff2e3034);

    drawSkinFace(badgeX + 3, badgeY + 3, 14);

    std::string shortDisp = activeDispName.size() > 8 ? activeDispName.substr(0, 7) + "." : activeDispName;
    drawString(font, shortDisp, badgeX + 20, badgeY + 2, 0xffffffff);
    if (isOnline) {
        drawString(font, "Online", badgeX + 20, badgeY + 11, 0xff55ff55);
    } else {
        drawString(font, "Offline", badgeX + 20, badgeY + 11, 0xffff5555);
    }

    // Left Navigation Panel (4 Tabs)
    drawPanel(_leftPanelX, _leftPanelY, _leftPanelW, _leftPanelH, 0xd8141618, 0xff282a2e);

    int tabH = (_leftPanelH - 12) / 4;
    tabH = std::min(32, std::max(24, tabH));
    int tabY = _leftPanelY + 6;

    drawNavTab(_leftPanelX + 4, tabY, _leftPanelW - 8, tabH, 0, "", "Resumen", "Informacion general", _selectedTab == 0, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + tabH + 2, _leftPanelW - 8, tabH, 1, "gui/user/user_icon.png", "Cuenta", "Estado y tipo de cuenta", _selectedTab == 1, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + (tabH + 2) * 2, _leftPanelW - 8, tabH, 2, "gui/user/user_icon.png", "Identidad", "Nombre y display name", _selectedTab == 2, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + (tabH + 2) * 3, _leftPanelW - 8, tabH, 3, "gui/user/lock.png", "Seguridad", "Contrasena y sesion", _selectedTab == 3, xm, ym);

    // Right Content Area — Switch based on _selectedTab
    switch (_selectedTab) {
        case 0: renderTabResumen(xm, ym); break;
        case 1: renderTabCuenta(xm, ym); break;
        case 2: renderTabIdentidad(xm, ym); break;
        case 3: renderTabSeguridad(xm, ym); break;
        default: renderTabResumen(xm, ym); break;
    }

    _btnBack.render(minecraft, xm, ym);
    _btnClose.render(minecraft, xm, ym);
}

// ─── TAB 0: Resumen ──────────────────────────────────────────────────────────

void ProfileScreen::renderTabResumen(int xm, int ym)
{
    const PlayerIdentity& id = AccountManager::getIdentity();
    bool isOnline = AccountManager::isOnline();
    std::string localUname = minecraft->options.getStringValue(OPTIONS_USERNAME);
    std::string activeUname = AccountManager::getUsername(localUname);
    std::string activeDispName = AccountManager::getDisplayName(localUname);

    int pad = 4;
    int contentW = _rightPanelW;
    int contentH = _rightPanelH;

    int secTopH = std::max(75, (int)(contentH * 0.38f));
    int secMidH = std::max(46, (int)(contentH * 0.24f));
    int secBotH = contentH - secTopH - secMidH - pad * 2;

    int sec1Y = _rightPanelY;
    int sec2Y = sec1Y + secTopH + pad;
    int sec3Y = sec2Y + secMidH + pad;

    // SECTION 1: TU PERFIL & CUENTA
    drawPanel(_rightPanelX, sec1Y, contentW, secTopH, 0xd815171a, 0xff2d3035);
    drawString(font, "TU PERFIL", _rightPanelX + 8, sec1Y + 6, 0xffffcc00);

    int avatarBoxX = _rightPanelX + 8;
    int avatarBoxY = sec1Y + 18;
    int avatarSize = std::min(46, secTopH - 24);
    drawPanel(avatarBoxX, avatarBoxY, avatarSize, avatarSize, 0xff101214, 0xff3a3c40);
    drawSkinBust(avatarBoxX + 2, avatarBoxY + 2, avatarSize - 4);

    int profileInfoX = avatarBoxX + avatarSize + 8;
    int profileInfoY = avatarBoxY + 1;

    drawString(font, activeDispName, profileInfoX, profileInfoY, 0xffffffff);

    int badgeTagY = profileInfoY + 11;
    if (!isOnline) {
        int tagW = 46;
        int tagH = 10;
        fill(profileInfoX, badgeTagY, profileInfoX + tagW, badgeTagY + tagH, 0x40ff0000);
        fill(profileInfoX, badgeTagY, profileInfoX + tagW, badgeTagY + 1, 0xffff4444);
        fill(profileInfoX, badgeTagY + tagH - 1, profileInfoX + tagW, badgeTagY + tagH, 0xffff4444);
        fill(profileInfoX, badgeTagY, profileInfoX + 1, badgeTagY + tagH, 0xffff4444);
        fill(profileInfoX + tagW - 1, badgeTagY, profileInfoX + tagW, badgeTagY + tagH, 0xffff4444);
        drawString(font, "OFFLINE", profileInfoX + 5, badgeTagY + 1, 0xffff5555);

        drawString(font, "Perfil local", profileInfoX, badgeTagY + 13, 0xffdddddd);
        drawString(font, "Estas jugando sin una cuenta", profileInfoX, badgeTagY + 23, 0xff888888);
        drawString(font, "de PocketMC vinculada.", profileInfoX, badgeTagY + 32, 0xff888888);
    } else {
        int tagW = 42;
        int tagH = 10;
        fill(profileInfoX, badgeTagY, profileInfoX + tagW, badgeTagY + tagH, 0x4000ff00);
        fill(profileInfoX, badgeTagY, profileInfoX + tagW, badgeTagY + 1, 0xff55ff55);
        fill(profileInfoX, badgeTagY + tagH - 1, profileInfoX + tagW, badgeTagY + tagH, 0xff55ff55);
        fill(profileInfoX, badgeTagY, profileInfoX + 1, badgeTagY + tagH, 0xff55ff55);
        fill(profileInfoX + tagW - 1, badgeTagY, profileInfoX + tagW, badgeTagY + tagH, 0xff55ff55);
        drawString(font, "ONLINE", profileInfoX + 5, badgeTagY + 1, 0xff55ff55);

        drawString(font, "@" + activeUname, profileInfoX, badgeTagY + 13, 0xffffcc00);
        drawString(font, "Cuenta oficial PocketMC vinculada.", profileInfoX, badgeTagY + 23, 0xff888888);
    }

    // Right Sub-Card: CUENTA
    int accountCardW = std::min(150, (int)(contentW * 0.44f));
    int accountCardX = _rightPanelX + contentW - accountCardW - 8;
    int accountCardY = sec1Y + 6;
    int accountCardH = secTopH - 12;
    drawPanel(accountCardX, accountCardY, accountCardW, accountCardH, 0xe0101214, 0xff383b40);

    drawString(font, "CUENTA", accountCardX + 8, accountCardY + 5, 0xffffcc00);
    drawString(font, "Tipo de cuenta", accountCardX + 8, accountCardY + 16, 0xff888888);
    drawString(font, isOnline ? ". Online" : ". Offline", accountCardX + 8, accountCardY + 25, isOnline ? 0xff55ff55 : 0xffff5555);

    drawString(font, "Estado", accountCardX + 8, accountCardY + 36, 0xff888888);
    drawString(font, isOnline ? ". Vinculada / Activa" : ". Sin vincular", accountCardX + 8, accountCardY + 45, isOnline ? 0xff55ff55 : 0xffffcc00);

    // Golden CTA Button
    if (!isOnline) {
        int bx = _btnAction.x, by = _btnAction.y, bw = _btnAction.width, bh = _btnAction.height;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, bHover ? 0xfff0b020 : 0xffd99b16);
        fill(bx, by, bx + bw, by + 1, 0xfff5c542);
        fill(bx, by, bx + 1, by + bh, 0xfff5c542);
        fill(bx, by + bh - 1, bx + bw, by + bh, 0xff9e6c07);
        fill(bx + bw - 1, by, bx + bw, by + bh, 0xff9e6c07);
        drawString(font, "Vincular Cuenta PocketMC", bx + 6, by + (bh - 8) / 2, 0xff1c1202);
        drawString(font, ">", bx + bw - 10, by + (bh - 8) / 2, 0xff1c1202);
    } else {
        _btnAction.render(minecraft, xm, ym);
        _btnSignOut.render(minecraft, xm, ym);
    }

    // SECTION 2: INFORMACION
    drawPanel(_rightPanelX, sec2Y, contentW, secMidH, 0xd815171a, 0xff2d3035);
    drawString(font, "INFORMACION", _rightPanelX + 8, sec2Y + 4, 0xffffcc00);

    int infoCardGap = 4;
    int infoCardY = sec2Y + 15;
    int infoCardH = secMidH - 20;
    int infoCardW = (contentW - 16 - infoCardGap * 3) / 4;

    drawInfoCard(_rightPanelX + 8, infoCardY, infoCardW, infoCardH, "gui/user/user_icon.png", "Nombre de usuario", isOnline ? "@" + activeUname : localUname, 0xffffcc00);
    std::string pidVal = isOnline ? (id.playerId.size() > 10 ? id.playerId.substr(0, 9) + ".." : id.playerId) : "Modo local";
    drawInfoCard(_rightPanelX + 8 + (infoCardW + infoCardGap), infoCardY, infoCardW, infoCardH, "gui/user/calendar.png", isOnline ? "PocketMC ID" : "Creado el", isOnline ? pidVal : "Modo local", 0xffffcc00);
    drawInfoCard(_rightPanelX + 8 + (infoCardW + infoCardGap) * 2, infoCardY, infoCardW, infoCardH, "gui/user/smartphone.png", "Dispositivo", "Este dispositivo", 0xffffcc00);
    drawInfoCard(_rightPanelX + 8 + (infoCardW + infoCardGap) * 3, infoCardY, infoCardW, infoCardH, "gui/user/control.png", "Modo de juego", isOnline ? "Online" : "Offline", 0xffffcc00);

    // SECTION 3: ACCIONES RAPIDAS & NOTA
    drawPanel(_rightPanelX, sec3Y, contentW, secBotH, 0xd815171a, 0xff2d3035);
    drawString(font, "ACCIONES RAPIDAS", _rightPanelX + 8, sec3Y + 4, 0xffffcc00);

    int actionCardGap = 4;
    int actionCardY = sec3Y + 15;
    int actionCardH = std::min(26, secBotH / 2);
    int actionCardW = (contentW - 16 - actionCardGap * 2) / 3;

    drawActionCard(_rightPanelX + 8, actionCardY, actionCardW, actionCardH, "gui/user/pencil_edit_icon.png", "Editar nombre local", "Cambia tu nombre de usuario offline.", xm, ym);
    drawActionCard(_rightPanelX + 8 + (actionCardW + actionCardGap), actionCardY, actionCardW, actionCardH, "", "Exportar perfil", "Guarda una copia de tu perfil local.", xm, ym);
    drawActionCard(_rightPanelX + 8 + (actionCardW + actionCardGap) * 2, actionCardY, actionCardW, actionCardH, "", "Borrar perfil local", "Eliminar este perfil de tu dispositivo.", xm, ym);

    // Bottom note
    int noteY = actionCardY + actionCardH + 4;
    int noteH = secBotH - (noteY - sec3Y) - 4;
    if (noteH >= 16) {
        int noteX = _rightPanelX + 8;
        int noteW = contentW - 16;
        fill(noteX, noteY, noteX + noteW, noteY + noteH, 0x500f1c29);
        fill(noteX, noteY, noteX + noteW, noteY + 1, 0xff1e3a5a);
        fill(noteX, noteY + noteH - 1, noteX + noteW, noteY + noteH, 0xff1e3a5a);
        fill(noteX, noteY, noteX + 1, noteY + noteH, 0xff1e3a5a);
        fill(noteX + noteW - 1, noteY, noteX + noteW, noteY + noteH, 0xff1e3a5a);

        drawString(font, "(i)", noteX + 6, noteY + (noteH - 8) / 2, 0xff38bdf8);
        drawString(font, "Al vincular una cuenta PocketMC, obtendras tu ID unico y podras acceder a servicios online en el futuro.", noteX + 22, noteY + 3, 0xff88a0b8);
        if (noteH >= 22) {
            drawString(font, "Puedes seguir jugando sin una cuenta.", noteX + 22, noteY + 12, 0xff88a0b8);
        }
    }
}

// ─── TAB 1: Cuenta ───────────────────────────────────────────────────────────

void ProfileScreen::renderTabCuenta(int xm, int ym)
{
    const PlayerIdentity& id = AccountManager::getIdentity();
    bool isOnline = AccountManager::isOnline();

    drawPanel(_rightPanelX, _rightPanelY, _rightPanelW, _rightPanelH, 0xd815171a, 0xff2d3035);

    drawString(font, "CUENTA", _rightPanelX + 10, _rightPanelY + 8, 0xffffcc00);
    drawString(font, "Estado y administracion de tu cuenta PocketMC.", _rightPanelX + 10, _rightPanelY + 20, 0xff888888);

    int cardX = _rightPanelX + 10;
    int cardY = _rightPanelY + 36;
    int cardW = _rightPanelW - 20;
    int cardH = _rightPanelH - 46;

    drawPanel(cardX, cardY, cardW, cardH, 0xe0101214, 0xff383b40);

    int lineY = cardY + 12;

    drawString(font, "Tipo de cuenta", cardX + 12, lineY, 0xff888888);
    drawString(font, isOnline ? ". Online" : ". Offline", cardX + 12, lineY + 11, isOnline ? 0xff55ff55 : 0xffff5555);
    lineY += 28;

    drawString(font, "Estado de vinculacion", cardX + 12, lineY, 0xff888888);
    drawString(font, isOnline ? ". Cuenta vinculada y activa" : ". Sin vincular (Perfil local)", cardX + 12, lineY + 11, isOnline ? 0xff55ff55 : 0xffffcc00);
    lineY += 28;

    if (isOnline) {
        drawString(font, "Correo electronico de la cuenta", cardX + 12, lineY, 0xff888888);
        drawString(font, maskEmail("cuentas.oficial.cesardev@gmail.com"), cardX + 12, lineY + 11, 0xffffffff);
        lineY += 28;

        drawString(font, "Miembro desde", cardX + 12, lineY, 0xff888888);
        drawString(font, "Septiembre de 2026", cardX + 12, lineY + 11, 0xffcccccc);
        lineY += 32;

        // Button Administrar cuenta (Web)
        int bw = 160, bh = 20;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, bHover ? 0xfff0b020 : 0xffd99b16);
        fill(bx, by, bx + bw, by + 1, 0xfff5c542);
        fill(bx, by, bx + 1, by + bh, 0xfff5c542);
        fill(bx, by + bh - 1, bx + bw, by + bh, 0xff9e6c07);
        fill(bx + bw - 1, by, bx + bw, by + bh, 0xff9e6c07);
        drawString(font, "Administrar cuenta (Web)  >", bx + 8, by + 6, 0xff1c1202);

        // Button Cerrar sesion
        int sbx = bx + bw + 10;
        bool sHover = (xm >= sbx && xm <= sbx + 100 && ym >= by && ym <= by + bh);
        fill(sbx, by, sbx + 100, by + bh, sHover ? 0x80ff4444 : 0x5033363a);
        fill(sbx, by, sbx + 100, by + 1, 0xff555555);
        fill(sbx, by + bh - 1, sbx + 100, by + bh, 0xff555555);
        fill(sbx, by, sbx + 1, by + bh, 0xff555555);
        fill(sbx + 100 - 1, by, sbx + 100, by + bh, 0xff555555);
        drawString(font, "Cerrar sesion", sbx + 14, by + 6, 0xffffffff);
    } else {
        drawString(font, "Estas usando un perfil local guardado unicamente en este dispositivo.", cardX + 12, lineY, 0xffaaaaaa);
        drawString(font, "Puedes vincular una cuenta PocketMC cuando quieras para obtener tu ID global.", cardX + 12, lineY + 12, 0xff888888);
        lineY += 36;

        int bw = 180, bh = 22;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, bHover ? 0xfff0b020 : 0xffd99b16);
        fill(bx, by, bx + bw, by + 1, 0xfff5c542);
        fill(bx, by, bx + 1, by + bh, 0xfff5c542);
        fill(bx, by + bh - 1, bx + bw, by + bh, 0xff9e6c07);
        fill(bx + bw - 1, by, bx + bw, by + bh, 0xff9e6c07);
        drawString(font, "Vincular Cuenta PocketMC  >", bx + 10, by + 7, 0xff1c1202);
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

    drawString(font, "IDENTIDAD", _rightPanelX + 10, _rightPanelY + 8, 0xffffcc00);
    drawString(font, "Como te identificas dentro del juego y con otros jugadores.", _rightPanelX + 10, _rightPanelY + 20, 0xff888888);

    int cardX = _rightPanelX + 10;
    int cardY = _rightPanelY + 36;
    int cardW = _rightPanelW - 20;
    int cardH = _rightPanelH - 46;

    drawPanel(cardX, cardY, cardW, cardH, 0xe0101214, 0xff383b40);

    int lineY = cardY + 12;

    if (!isOnline) {
        drawString(font, "Nombre de jugador local", cardX + 12, lineY, 0xff888888);
        drawString(font, localUname, cardX + 12, lineY + 11, 0xffffffff);
        lineY += 28;

        drawString(font, "Tipo de identidad", cardX + 12, lineY, 0xff888888);
        drawString(font, "Perfil local (Sin nombre de usuario online vinculado)", cardX + 12, lineY + 11, 0xffffaa55);
        lineY += 28;

        drawString(font, "Este nombre esta guardado unicamente en este dispositivo.", cardX + 12, lineY, 0xffaaaaaa);
        lineY += 30;

        int bw = 160, bh = 20;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, bHover ? 0xff404448 : 0xff282a2e);
        fill(bx, by, bx + bw, by + 1, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx, by + bh - 1, bx + bw, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx, by, bx + 1, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx + bw - 1, by, bx + bw, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        drawString(font, "Editar nombre local  >", bx + 10, by + 6, bHover ? 0xffffcc00 : 0xffffffff);
    } else {
        drawString(font, "Nombre visible dentro del juego (Display Name)", cardX + 12, lineY, 0xff888888);
        drawString(font, activeDispName, cardX + 12, lineY + 11, 0xffffffff);
        lineY += 26;

        drawString(font, "Nombre de usuario unico (@handle)", cardX + 12, lineY, 0xff888888);
        drawString(font, "@" + activeUname, cardX + 12, lineY + 11, 0xffffcc00);
        lineY += 26;

        drawString(font, "PocketMC Player ID (UUID permanente)", cardX + 12, lineY, 0xff888888);
        drawString(font, id.playerId.empty() ? "No disponible" : id.playerId, cardX + 12, lineY + 11, 0xff55ff55);
        lineY += 30;

        // Button Copiar ID
        int bw = 90, bh = 20;
        int bx = cardX + 12, by = lineY;
        bool cHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, _copiedFeedbackTicks > 0 ? 0xff10b981 : (cHover ? 0xff404448 : 0xff282a2e));
        fill(bx, by, bx + bw, by + 1, 0xff555555);
        fill(bx, by + bh - 1, bx + bw, by + bh, 0xff555555);
        fill(bx, by, bx + 1, by + bh, 0xff555555);
        fill(bx + bw - 1, by, bx + bw, by + bh, 0xff555555);
        drawString(font, _copiedFeedbackTicks > 0 ? "Copiado!" : "Copiar ID", bx + 14, by + 6, _copiedFeedbackTicks > 0 ? 0xff022315 : 0xffffffff);

        // Button Editar perfil (Web)
        int ebx = bx + bw + 10;
        int ebw = 140;
        bool eHover = (xm >= ebx && xm <= ebx + ebw && ym >= by && ym <= by + bh);
        fill(ebx, by, ebx + ebw, by + bh, eHover ? 0xfff0b020 : 0xffd99b16);
        fill(ebx, by, ebx + ebw, by + 1, 0xfff5c542);
        fill(ebx, by, ebx + 1, by + bh, 0xfff5c542);
        fill(ebx, by + bh - 1, ebx + ebw, by + bh, 0xff9e6c07);
        fill(ebx + ebw - 1, by, ebx + ebw, by + bh, 0xff9e6c07);
        drawString(font, "Editar perfil (Web)  >", ebx + 10, by + 6, 0xff1c1202);
    }
}

// ─── TAB 3: Seguridad ────────────────────────────────────────────────────────

void ProfileScreen::renderTabSeguridad(int xm, int ym)
{
    bool isOnline = AccountManager::isOnline();

    drawPanel(_rightPanelX, _rightPanelY, _rightPanelW, _rightPanelH, 0xd815171a, 0xff2d3035);

    drawString(font, "SEGURIDAD", _rightPanelX + 10, _rightPanelY + 8, 0xffffcc00);
    drawString(font, "Opciones de autenticacion, contrasena y sesion.", _rightPanelX + 10, _rightPanelY + 20, 0xff888888);

    int cardX = _rightPanelX + 10;
    int cardY = _rightPanelY + 36;
    int cardW = _rightPanelW - 20;
    int cardH = _rightPanelH - 46;

    drawPanel(cardX, cardY, cardW, cardH, 0xe0101214, 0xff383b40);

    int lineY = cardY + 14;

    if (!isOnline) {
        drawIconTexture("gui/user/lock.png", cardX + 12, lineY, 16, 16, 0xffffcc00);
        drawString(font, "No hay una cuenta PocketMC vinculada.", cardX + 34, lineY + 4, 0xffffffff);
        lineY += 24;

        drawString(font, "Vincula una cuenta PocketMC para acceder a las opciones de seguridad,", cardX + 12, lineY, 0xffaaaaaa);
        drawString(font, "cambio de contrasena y administracion de sesiones activas.", cardX + 12, lineY + 12, 0xff888888);
        lineY += 36;

        int bw = 180, bh = 22;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, bHover ? 0xfff0b020 : 0xffd99b16);
        fill(bx, by, bx + bw, by + 1, 0xfff5c542);
        fill(bx, by, bx + 1, by + bh, 0xfff5c542);
        fill(bx, by + bh - 1, bx + bw, by + bh, 0xff9e6c07);
        fill(bx + bw - 1, by, bx + bw, by + bh, 0xff9e6c07);
        drawString(font, "Vincular Cuenta PocketMC  >", bx + 10, by + 7, 0xff1c1202);
    } else {
        drawString(font, "Sesion actual", cardX + 12, lineY, 0xff888888);
        drawString(font, ". Activa en este dispositivo", cardX + 12, lineY + 11, 0xff55ff55);
        lineY += 28;

        drawString(font, "Ultima autenticacion", cardX + 12, lineY, 0xff888888);
        drawString(font, "Hoy", cardX + 12, lineY + 11, 0xffffffff);
        lineY += 28;

        drawString(font, "Contrasena de la cuenta", cardX + 12, lineY, 0xff888888);
        drawString(font, "************", cardX + 12, lineY + 11, 0xffaaaaaa);
        lineY += 32;

        // Button Cambiar contraseña (Web)
        int bw = 160, bh = 20;
        int bx = cardX + 12, by = lineY;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);
        fill(bx, by, bx + bw, by + bh, bHover ? 0xff404448 : 0xff282a2e);
        fill(bx, by, bx + bw, by + 1, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx, by + bh - 1, bx + bw, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx, by, bx + 1, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        fill(bx + bw - 1, by, bx + bw, by + bh, bHover ? 0xffffcc00 : 0xff555555);
        drawString(font, "Cambiar contrasena (Web)  >", bx + 8, by + 6, bHover ? 0xffffcc00 : 0xffffffff);

        // Button Cerrar sesion
        int sbx = bx + bw + 10;
        bool sHover = (xm >= sbx && xm <= sbx + 100 && ym >= by && ym <= by + bh);
        fill(sbx, by, sbx + 100, by + bh, sHover ? 0x80ff4444 : 0x5033363a);
        fill(sbx, by, sbx + 100, by + 1, 0xff555555);
        fill(sbx, by + bh - 1, sbx + 100, by + bh, 0xff555555);
        fill(sbx, by, sbx + 1, by + bh, 0xff555555);
        fill(sbx + 100 - 1, by, sbx + 100, by + bh, 0xff555555);
        drawString(font, "Cerrar sesion", sbx + 14, by + 6, 0xffffffff);
    }
}

// ─── Interaction Handlers ───────────────────────────────────────────────────

void ProfileScreen::buttonClicked(Button* button)
{
    if (button == &_btnBack || button == &_btnClose) {
        minecraft->setScreen(new StartMenuScreen());
        return;
    }

    if (button == &_btnAction) {
        if (!AccountManager::isOnline()) {
            minecraft->setScreen(new LinkAccountScreen());
        } else {
            minecraft->platform()->openURL("http://localhost:5173/pages/profile.html");
        }
        return;
    }

    if (button == &_btnSignOut) {
        AccountManager::logout();
        init();
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
            int pad = 4;
            int secTopH = std::max(75, (int)(_rightPanelH * 0.38f));
            int secMidH = std::max(46, (int)(_rightPanelH * 0.24f));
            int sec3Y = _rightPanelY + secTopH + pad + secMidH + pad;
            int actionCardY = sec3Y + 15;
            int actionCardH = std::min(26, (_rightPanelH - secTopH - secMidH - pad * 2) / 2);
            int actionCardW = (_rightPanelW - 16 - 8) / 3;

            // Card 1: Editar nombre local
            if (x >= _rightPanelX + 8 && x <= _rightPanelX + 8 + actionCardW &&
                y >= actionCardY && y <= actionCardY + actionCardH) {
                minecraft->setScreen(new UsernameScreen());
                return;
            }

            // Gold Button click when Offline
            if (!isOnline) {
                int bx = _btnAction.x, by = _btnAction.y, bw = _btnAction.width, bh = _btnAction.height;
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->setScreen(new LinkAccountScreen());
                    return;
                }
            }
        }

        // 3. Tab 1 (Cuenta) Interactions
        else if (_selectedTab == 1) {
            int cardX = _rightPanelX + 10;
            int cardY = _rightPanelY + 36;
            if (isOnline) {
                int lineY = cardY + 12 + 28 * 4 + 4;
                int bx = cardX + 12, by = lineY, bw = 160, bh = 20;
                // Administrar cuenta (Web)
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->platform()->openURL("http://localhost:5173/pages/profile.html");
                    return;
                }
                // Cerrar sesion
                int sbx = bx + bw + 10;
                if (x >= sbx && x <= sbx + 100 && y >= by && y <= by + bh) {
                    AccountManager::logout();
                    init();
                    return;
                }
            } else {
                int lineY = cardY + 12 + 28 * 2 + 36;
                int bx = cardX + 12, by = lineY, bw = 180, bh = 22;
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->setScreen(new LinkAccountScreen());
                    return;
                }
            }
        }

        // 4. Tab 2 (Identidad) Interactions
        else if (_selectedTab == 2) {
            int cardX = _rightPanelX + 10;
            int cardY = _rightPanelY + 36;
            if (!isOnline) {
                int lineY = cardY + 12 + 28 * 2 + 30;
                int bx = cardX + 12, by = lineY, bw = 160, bh = 20;
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->setScreen(new UsernameScreen());
                    return;
                }
            } else {
                int lineY = cardY + 12 + 26 * 2 + 30;
                int bx = cardX + 12, by = lineY, bw = 90, bh = 20;
                // Copiar ID
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    copyToClipboard(AccountManager::getIdentity().playerId);
                    return;
                }
                // Editar perfil (Web)
                int ebx = bx + bw + 10, ebw = 140;
                if (x >= ebx && x <= ebx + ebw && y >= by && y <= by + bh) {
                    minecraft->platform()->openURL("http://localhost:5173/pages/edit-profile.html");
                    return;
                }
            }
        }

        // 5. Tab 3 (Seguridad) Interactions
        else if (_selectedTab == 3) {
            int cardX = _rightPanelX + 10;
            int cardY = _rightPanelY + 36;
            if (!isOnline) {
                int lineY = cardY + 14 + 24 + 36;
                int bx = cardX + 12, by = lineY, bw = 180, bh = 22;
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->setScreen(new LinkAccountScreen());
                    return;
                }
            } else {
                int lineY = cardY + 14 + 28 * 2 + 32;
                int bx = cardX + 12, by = lineY, bw = 160, bh = 20;
                // Cambiar contrasena (Web)
                if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                    minecraft->platform()->openURL("http://localhost:5173/pages/forgot-password.html");
                    return;
                }
                // Cerrar sesion
                int sbx = bx + bw + 10;
                if (x >= sbx && x <= sbx + 100 && y >= by && y <= by + bh) {
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
