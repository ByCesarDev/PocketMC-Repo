#include "ModListScreen.h"

#include "../../Minecraft.h"
#include "../../renderer/Textures.h"
#include "../../../platform/input/Mouse.h"
#include "locale/I18n.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

static std::string joinRoot(const std::string& a, const std::string& b)
{
    if (a.empty() || a == ".") return b;
    char last = a[a.size() - 1];
    if (last == '/' || last == '\\') return a + b;
    return a + "/" + b;
}

ModListScreen::ModListScreen()
    : bOpenFolder(1, I18n::get("mods.openFolder")),
      bDone(2, I18n::get("gui.done")),
      selected(0),
      scrollOffset(0),
      listX(0),
      listY(0),
      listW(0),
      listH(0),
      detailX(0),
      detailY(0),
      detailW(0),
      detailH(0)
{}

void ModListScreen::init()
{
    buttons.clear();
    tabButtons.clear();
    mods = ModRegistry::scanMods(minecraft->externalStoragePath);
    if (selected >= (int)mods.size()) selected = mods.empty() ? -1 : 0;
    buttons.push_back(&bOpenFolder);
    buttons.push_back(&bDone);
    tabButtons.push_back(&bOpenFolder);
    tabButtons.push_back(&bDone);
    setupPositions();
}

void ModListScreen::setupPositions()
{
    const int pad = 10;
    const int bottomH = 28;
    int usableW = width - pad * 2;
    listW = (std::min)(260, usableW / 2);
    if (listW < 140) listW = usableW;
    listX = pad;
    listY = 34;
    listH = height - listY - bottomH - pad;

    detailX = listX + listW + pad;
    detailY = listY;
    detailW = width - detailX - pad;
    detailH = listH;
    if (detailW < 120) {
        detailX = pad;
        detailY = listY + listH / 2 + pad;
        detailW = usableW;
        detailH = height - detailY - bottomH - pad;
        listH = detailY - listY - pad;
    }

    bOpenFolder.width = (std::min)(180, (std::max)(120, width / 3));
    bOpenFolder.height = 20;
    bOpenFolder.x = width / 2 - bOpenFolder.width - 6;
    bOpenFolder.y = height - 24;

    bDone.width = bOpenFolder.width;
    bDone.height = 20;
    bDone.x = width / 2 + 6;
    bDone.y = bOpenFolder.y;
}

void ModListScreen::render(int xm, int ym, float a)
{
    renderBackground();
    drawCenteredString(font, I18n::get("menu.mods"), width / 2, 12, 0xffffffff);

    fill(listX, listY, listX + listW, listY + listH, 0x90000000);
    fill(detailX, detailY, detailX + detailW, detailY + detailH, 0x90000000);

    const int rowH = 42;
    int first = scrollOffset / rowH;
    int end = (std::min)((int)mods.size(), first + listH / rowH + 2);
    for (int i = first; i < end; ++i) {
        int y = listY + i * rowH - scrollOffset;
        if (i == selected) {
            fill(listX + 1, y + 1, listX + listW - 1, y + rowH - 1, 0xc0223344);
            fill(listX, y, listX + listW, y + 1, 0xffffffff);
            fill(listX, y + rowH - 1, listX + listW, y + rowH, 0xffffffff);
        } else {
            fill(listX + 1, y + 1, listX + listW - 1, y + rowH - 1, (i % 2) ? 0x40000000 : 0x55000000);
        }

        int iconColor = mods[i].builtIn ? 0xff1477ff : 0xff45b04a;
        fill(listX + 6, y + 7, listX + 34, y + 35, iconColor);
        drawString(font, mods[i].name.c_str(), listX + 40, y + 6, 0xffffffff);
        std::string meta = mods[i].version.empty() ? mods[i].author : (mods[i].version + "  " + mods[i].author);
        drawString(font, meta.c_str(), listX + 40, y + 18, 0xffaaaaaa);
        std::string packs = "";
        if (mods[i].hasResourcePack) packs += "resource_packs ";
        if (mods[i].hasBeheviorPack) packs += "behevior_packs";
        drawString(font, packs.c_str(), listX + 40, y + 30, 0xff88ccff);
    }

    if (selected >= 0 && selected < (int)mods.size()) {
        const ModInfo& m = mods[selected];
        int x = detailX + 10;
        int y = detailY + 10;
        drawString(font, m.name.c_str(), x, y, 0xffffffff);
        drawString(font, ("Version: " + m.version).c_str(), x, y + 14, 0xffaaaaaa);
        drawString(font, ("Author: " + m.author).c_str(), x, y + 26, 0xffaaaaaa);
        drawString(font, m.description.c_str(), x, y + 48, 0xffdddddd);
        drawString(font, ("Path: " + m.path).c_str(), x, y + 74, 0xff999999);
        drawString(font, m.hasResourcePack ? "Resource Pack: yes" : "Resource Pack: no", x, y + 96, 0xffdddddd);
        drawString(font, m.hasBeheviorPack ? "Behevior Pack: yes" : "Behevior Pack: no", x, y + 108, 0xffdddddd);
    }

    Screen::render(xm, ym, a);
}

int ModListScreen::modAt(int x, int y) const
{
    if (x < listX || x > listX + listW || y < listY || y > listY + listH) return -1;
    int idx = (y - listY + scrollOffset) / 42;
    if (idx < 0 || idx >= (int)mods.size()) return -1;
    return idx;
}

void ModListScreen::mouseClicked(int x, int y, int buttonNum)
{
    int idx = modAt(x, y);
    if (idx >= 0) selected = idx;
    Screen::mouseClicked(x, y, buttonNum);
}

void ModListScreen::mouseWheel(int, int dy, int, int)
{
    scrollOffset -= dy * 12;
    if (scrollOffset < 0) scrollOffset = 0;
    int maxScroll = (std::max)(0, (int)mods.size() * 42 - listH);
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
}

bool ModListScreen::handleBackEvent(bool isDown)
{
    if (!isDown) minecraft->setScreen(NULL);
    return true;
}

bool ModListScreen::isInGameScreen()
{
    return false;
}

void ModListScreen::buttonClicked(Button* button)
{
    if (button == &bDone) {
        minecraft->setScreen(NULL);
        return;
    }
    if (button == &bOpenFolder) {
        openModsFolder();
        return;
    }
}

void ModListScreen::openModsFolder()
{
    ModRegistry::ensureFolders(minecraft->externalStoragePath);
    std::string path = joinRoot(minecraft->externalStoragePath, "games/com.mojang/mods");
#ifdef _WIN32
    std::replace(path.begin(), path.end(), '/', '\\');
    char fullPath[MAX_PATH];
    if (_fullpath(fullPath, path.c_str(), MAX_PATH)) {
        path = fullPath;
    }
#endif
    if (minecraft && minecraft->platform()) {
        minecraft->platform()->openURL(path);
    }
}
