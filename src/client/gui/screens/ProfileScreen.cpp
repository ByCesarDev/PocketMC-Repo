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

ProfileScreen::ProfileScreen()
:   _btnBack(1, "< Atras"),
    _btnClose(2, "X"),
    _btnAction(3, "Vincular Cuenta PocketMC  >"),
    _btnSignOut(4, "Cerrar sesion"),
    _selectedTab(0),
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

    if (AccountManager::isOnline()) {
        _btnAction.msg = "Administrar cuenta  >";
        _btnSignOut.msg = "Cerrar sesion";
    } else {
        _btnAction.msg = "Vincular Cuenta PocketMC  >";
        _btnSignOut.msg = "";
    }

    _btnBack.msg = "< Atras";
    _btnClose.msg = "X";

    buttons.push_back(&_btnBack);
    buttons.push_back(&_btnClose);
    buttons.push_back(&_btnAction);
    if (AccountManager::isOnline()) {
        buttons.push_back(&_btnSignOut);
    }

    tabButtons.push_back(&_btnBack);
    tabButtons.push_back(&_btnClose);
    tabButtons.push_back(&_btnAction);
    if (AccountManager::isOnline()) {
        tabButtons.push_back(&_btnSignOut);
    }

    setupPositions();
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

    // Right panel sub-cards sizing
    int secTopH = std::max(75, (int)(_rightPanelH * 0.38f));
    int accountCardW = std::min(150, (int)(_rightPanelW * 0.44f));
    int accountCardX = _rightPanelX + _rightPanelW - accountCardW - 8;
    int accountCardY = _rightPanelY + 6;

    // Button Vincular / Administrar inside account card
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

// ─── Drawing Helpers ─────────────────────────────────────────────────────────

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

    // Face front (8,8, 8,8)
    drawQuad((float)x, (float)y, (float)size, (float)size, 8.0f, 8.0f, 8.0f, 8.0f);
    // Hat overlay (40,8, 8,8)
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

    // 1. Head front face (8,8, 8,8)
    drawQuad(headX, headY, headW, headH, 8.0f, 8.0f, 8.0f, 8.0f);
    // Head hat overlay (40,8, 8,8)
    drawQuad(headX, headY, headW, headH, 40.0f, 8.0f, 8.0f, 8.0f);

    // 2. Right Arm / Shoulder (4x6 at 44,20)
    drawQuad(bodyX, bodyY, 4.0f * unit, bodyH, 44.0f, 20.0f, 4.0f, 6.0f);
    if (texH >= 64) drawQuad(bodyX, bodyY, 4.0f * unit, bodyH, 44.0f, 36.0f, 4.0f, 6.0f);

    // 3. Torso front (8x6 at 20,20)
    drawQuad(bodyX + 4.0f * unit, bodyY, 8.0f * unit, bodyH, 20.0f, 20.0f, 8.0f, 6.0f);
    if (texH >= 64) drawQuad(bodyX + 4.0f * unit, bodyY, 8.0f * unit, bodyH, 20.0f, 36.0f, 8.0f, 6.0f);

    // 4. Left Arm / Shoulder (4x6 at 44,20 or 36,52 if 64x64)
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
        // Selected style: golden border, subtle warm tint, right arrow indicator
        fill(x, y, x + w, y + h, 0x85242216);
        fill(x, y, x + w, y + 1, 0xffffcc00);
        fill(x, y + h - 1, x + w, y + h, 0xffffcc00);
        fill(x, y, x + 1, y + h, 0xffffcc00);
        fill(x + w - 1, y, x + w, y + h, 0xffffcc00);

        // Right indicator arrow notch
        drawString(font, ">", x + w - 8, y + (h - 8) / 2, 0xffffcc00);
    } else if (hover) {
        fill(x, y, x + w, y + h, 0x40ffffff);
    }

    int textX = x + 24;
    int textY = y + 4;

    // Draw Icon
    if (index == 0) {
        // Resumen tab: mini skin head avatar
        drawSkinFace(x + 5, y + (h - 14) / 2, 14);
    } else if (!iconPath.empty()) {
        drawIconTexture(iconPath, x + 6, y + (h - 12) / 2, 12, 12, isSelected ? 0xffffcc00 : 0xffcccccc);
    }

    // Title & Subtitle
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

    // Right chevron
    drawString(font, ">", x + w - 10, y + (h - 8) / 2, hover ? 0xffffcc00 : 0xff7e8288);
}

// ─── Render Method ───────────────────────────────────────────────────────────

void ProfileScreen::render(int xm, int ym, float a)
{
    // 1. Render active game background panorama with dark semitransparent overlay
    renderBackground();
    fill(0, 0, width, height, 0x60000000);

    const PlayerIdentity& id = AccountManager::getIdentity();
    bool isOnline = AccountManager::isOnline();
    std::string localUname = minecraft->options.getStringValue(OPTIONS_USERNAME);
    std::string activeUname = AccountManager::getUsername(localUname);
    std::string activeDispName = AccountManager::getDisplayName(localUname);

    // ─── Top Bar ────────────────────────────────────────────────────────────
    int headerTextX = _btnBack.x + _btnBack.width + 10;
    drawString(font, "Perfil", headerTextX, 5, 0xffffffff);
    drawString(font, "Administra tu cuenta y tu identidad en PocketMC.", headerTextX, 16, 0xff888888);

    // Right User Badge in Top Bar
    int badgeW = 92;
    int badgeH = 20;
    int badgeX = _btnClose.x - badgeW - 6;
    int badgeY = 4;
    drawPanel(badgeX, badgeY, badgeW, badgeH, 0xa0141618, 0xff2e3034);

    // Mini skin bust/face in badge
    drawSkinFace(badgeX + 3, badgeY + 3, 14);

    // User name & status
    std::string shortDisp = activeDispName.size() > 8 ? activeDispName.substr(0, 7) + "." : activeDispName;
    drawString(font, shortDisp, badgeX + 20, badgeY + 2, 0xffffffff);
    if (isOnline) {
        drawString(font, "Online", badgeX + 20, badgeY + 11, 0xff55ff55);
    } else {
        drawString(font, "Offline", badgeX + 20, badgeY + 11, 0xffff5555);
    }

    // ─── Left Navigation Panel ──────────────────────────────────────────────
    drawPanel(_leftPanelX, _leftPanelY, _leftPanelW, _leftPanelH, 0xd8141618, 0xff282a2e);

    int tabH = (_leftPanelH - 12) / 5;
    tabH = std::min(28, std::max(22, tabH));
    int tabY = _leftPanelY + 6;

    drawNavTab(_leftPanelX + 4, tabY, _leftPanelW - 8, tabH, 0, "", "Resumen", "Informacion general", _selectedTab == 0, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + tabH + 2, _leftPanelW - 8, tabH, 1, "gui/user/user_icon.png", "Cuenta", "Estado y tipo de cuenta", _selectedTab == 1, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + (tabH + 2) * 2, _leftPanelW - 8, tabH, 2, "gui/user/user_icon.png", "Identidad", "Nombre de usuario y nombre visible", _selectedTab == 2, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + (tabH + 2) * 3, _leftPanelW - 8, tabH, 3, "gui/user/lock.png", "Seguridad", "Contrasena y sesion", _selectedTab == 3, xm, ym);
    drawNavTab(_leftPanelX + 4, tabY + (tabH + 2) * 4, _leftPanelW - 8, tabH, 4, "gui/user/icon_setting.png", "Ajustes", "Opciones de cuenta", _selectedTab == 4, xm, ym);

    // ─── Right Content Area (TU PERFIL + INFORMACION + ACCIONES) ────────────
    int pad = 4;
    int contentW = _rightPanelW;
    int contentH = _rightPanelH;

    int secTopH = std::max(75, (int)(contentH * 0.38f));
    int secMidH = std::max(46, (int)(contentH * 0.24f));
    int secBotH = contentH - secTopH - secMidH - pad * 2;

    int sec1Y = _rightPanelY;
    int sec2Y = sec1Y + secTopH + pad;
    int sec3Y = sec2Y + secMidH + pad;

    // ── SECTION 1: TU PERFIL & CUENTA ───────────────────────────────────────
    drawPanel(_rightPanelX, sec1Y, contentW, secTopH, 0xd815171a, 0xff2d3035);

    // Header "TU PERFIL"
    drawString(font, "TU PERFIL", _rightPanelX + 8, sec1Y + 6, 0xffffcc00);

    // Avatar Box with 2D Skin Bust (head + torso shoulders)
    int avatarBoxX = _rightPanelX + 8;
    int avatarBoxY = sec1Y + 18;
    int avatarSize = std::min(46, secTopH - 24);
    drawPanel(avatarBoxX, avatarBoxY, avatarSize, avatarSize, 0xff101214, 0xff3a3c40);
    drawSkinBust(avatarBoxX + 2, avatarBoxY + 2, avatarSize - 4);

    // Info beside avatar
    int profileInfoX = avatarBoxX + avatarSize + 8;
    int profileInfoY = avatarBoxY + 1;

    // Large Name
    drawString(font, activeDispName, profileInfoX, profileInfoY, 0xffffffff);

    // Badge OFFLINE / ONLINE
    int badgeTagY = profileInfoY + 11;
    if (!isOnline) {
        // [ OFFLINE ]
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
        // [ ONLINE ]
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
    if (!isOnline) {
        drawString(font, ". Offline", accountCardX + 8, accountCardY + 25, 0xffff5555);
    } else {
        drawString(font, ". Online", accountCardX + 8, accountCardY + 25, 0xff55ff55);
    }

    drawString(font, "Estado", accountCardX + 8, accountCardY + 36, 0xff888888);
    if (!isOnline) {
        drawString(font, ". Sin vincular", accountCardX + 8, accountCardY + 45, 0xffffcc00);
    } else {
        drawString(font, ". Vinculada / Activa", accountCardX + 8, accountCardY + 45, 0xff55ff55);
    }

    // Custom Golden CTA Button Rendering for "Vincular Cuenta PocketMC >"
    if (!isOnline) {
        int bx = _btnAction.x;
        int by = _btnAction.y;
        int bw = _btnAction.width;
        int bh = _btnAction.height;
        bool bHover = (xm >= bx && xm <= bx + bw && ym >= by && ym <= by + bh);

        // Gold background fill
        fill(bx, by, bx + bw, by + bh, bHover ? 0xfff0b020 : 0xffd99b16);
        // Highlight top/left border
        fill(bx, by, bx + bw, by + 1, 0xfff5c542);
        fill(bx, by, bx + 1, by + bh, 0xfff5c542);
        // Shadow bottom/right border
        fill(bx, by + bh - 1, bx + bw, by + bh, 0xff9e6c07);
        fill(bx + bw - 1, by, bx + bw, by + bh, 0xff9e6c07);

        // Dark text with right arrow
        drawString(font, "Vincular Cuenta PocketMC", bx + 6, by + (bh - 8) / 2, 0xff1c1202);
        drawString(font, ">", bx + bw - 10, by + (bh - 8) / 2, 0xff1c1202);
    }

    // ── SECTION 2: INFORMACION ──────────────────────────────────────────────
    drawPanel(_rightPanelX, sec2Y, contentW, secMidH, 0xd815171a, 0xff2d3035);
    drawString(font, "INFORMACION", _rightPanelX + 8, sec2Y + 4, 0xffffcc00);

    int infoCardGap = 4;
    int infoCardY = sec2Y + 15;
    int infoCardH = secMidH - 20;
    int infoCardW = (contentW - 16 - infoCardGap * 3) / 4;

    // Card 1: Nombre de usuario (icon: user_icon.png)
    drawInfoCard(_rightPanelX + 8, infoCardY, infoCardW, infoCardH, "gui/user/user_icon.png", "Nombre de usuario", isOnline ? "@" + activeUname : localUname, 0xffffcc00);

    // Card 2: Creado el / PocketMC ID (icon: calendar.png)
    std::string pidVal = isOnline ? (id.playerId.size() > 10 ? id.playerId.substr(0, 9) + ".." : id.playerId) : "Modo local";
    drawInfoCard(_rightPanelX + 8 + (infoCardW + infoCardGap), infoCardY, infoCardW, infoCardH, "gui/user/calendar.png", isOnline ? "PocketMC ID" : "Creado el", isOnline ? pidVal : "Modo local", 0xffffcc00);

    // Card 3: Dispositivo (icon: smartphone.png)
    drawInfoCard(_rightPanelX + 8 + (infoCardW + infoCardGap) * 2, infoCardY, infoCardW, infoCardH, "gui/user/smartphone.png", "Dispositivo", "Este dispositivo", 0xffffcc00);

    // Card 4: Modo de juego (icon: control.png)
    drawInfoCard(_rightPanelX + 8 + (infoCardW + infoCardGap) * 3, infoCardY, infoCardW, infoCardH, "gui/user/control.png", "Modo de juego", isOnline ? "Online" : "Offline", 0xffffcc00);

    // ── SECTION 3: ACCIONES RAPIDAS & NOTA ───────────────────────────────────
    drawPanel(_rightPanelX, sec3Y, contentW, secBotH, 0xd815171a, 0xff2d3035);
    drawString(font, "ACCIONES RAPIDAS", _rightPanelX + 8, sec3Y + 4, 0xffffcc00);

    int actionCardGap = 4;
    int actionCardY = sec3Y + 15;
    int actionCardH = std::min(26, secBotH / 2);
    int actionCardW = (contentW - 16 - actionCardGap * 2) / 3;

    // Card 1: Editar nombre local (icon: pencil_edit_icon.png)
    drawActionCard(_rightPanelX + 8, actionCardY, actionCardW, actionCardH, "gui/user/pencil_edit_icon.png", "Editar nombre local", "Cambia tu nombre de usuario offline.", xm, ym);

    // Card 2: Exportar perfil (icon: user_icon.png)
    drawActionCard(_rightPanelX + 8 + (actionCardW + actionCardGap), actionCardY, actionCardW, actionCardH, "", "Exportar perfil", "Guarda una copia de tu perfil local.", xm, ym);

    // Card 3: Borrar perfil local (icon: icon_setting.png)
    drawActionCard(_rightPanelX + 8 + (actionCardW + actionCardGap) * 2, actionCardY, actionCardW, actionCardH, "", "Borrar perfil local", "Eliminar este perfil de tu dispositivo.", xm, ym);

    // Bottom info note box
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

    // ─── Render Registered Buttons (Atrás, X, Administrar/SignOut if Online) ──
    // Note: if offline, _btnAction was drawn custom with gold style above
    if (isOnline) {
        _btnAction.render(minecraft, xm, ym);
        _btnSignOut.render(minecraft, xm, ym);
    }
    _btnBack.render(minecraft, xm, ym);
    _btnClose.render(minecraft, xm, ym);
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
        // Check tabs clicks
        int tabH = (_leftPanelH - 12) / 5;
        tabH = std::min(28, std::max(22, tabH));
        int tabY0 = _leftPanelY + 6;

        for (int i = 0; i < 5; ++i) {
            int ty = tabY0 + i * (tabH + 2);
            if (x >= _leftPanelX + 4 && x <= _leftPanelX + _leftPanelW - 4 &&
                y >= ty && y <= ty + tabH) {
                _selectedTab = i;
                break;
            }
        }

        // Check Action Card: "Editar nombre local" click (Card 1)
        int pad = 4;
        int secTopH = std::max(75, (int)(_rightPanelH * 0.38f));
        int secMidH = std::max(46, (int)(_rightPanelH * 0.24f));
        int sec3Y = _rightPanelY + secTopH + pad + secMidH + pad;
        int actionCardY = sec3Y + 15;
        int actionCardH = std::min(26, (_rightPanelH - secTopH - secMidH - pad * 2) / 2);
        int actionCardW = (_rightPanelW - 16 - 8) / 3;

        if (x >= _rightPanelX + 8 && x <= _rightPanelX + 8 + actionCardW &&
            y >= actionCardY && y <= actionCardY + actionCardH) {
            minecraft->setScreen(new UsernameScreen());
            return;
        }

        // Check Gold Button click when Offline
        if (!AccountManager::isOnline()) {
            int bx = _btnAction.x;
            int by = _btnAction.y;
            int bw = _btnAction.width;
            int bh = _btnAction.height;
            if (x >= bx && x <= bx + bw && y >= by && y <= by + bh) {
                minecraft->setScreen(new LinkAccountScreen());
                return;
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
