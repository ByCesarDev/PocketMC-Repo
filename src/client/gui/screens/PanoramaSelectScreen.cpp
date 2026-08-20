#include "PanoramaSelectScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../renderer/GuiShader.h"
#include "../../renderer/gles.h"
#include "locale/I18n.h"
#include <algorithm>

static void renderPanoramaPreviewBox(Minecraft* mc, const std::string& folderPath, int guiScreenWidth, int guiScreenHeight, int boxX, int boxY, int boxW, int boxH, int ticks, float a) {
    if (!mc || boxW <= 0 || boxH <= 0) return;

    int physX = (int)(boxX * Gui::GuiScale);
    int physY = (int)((guiScreenHeight - (boxY + boxH)) * Gui::GuiScale);
    int physW = (int)(boxW * Gui::GuiScale);
    int physH = (int)(boxH * Gui::GuiScale);

    if (physW <= 0 || physH <= 0) return;

    // Unbind 2D GUI Shader before 3D perspective rendering
    GuiShader::unbind();

    glDisable2(GL_SCISSOR_TEST);
    glViewport(physX, physY, physW, physH);

    Tesselator& t = Tesselator::instance;

    glDisable2(GL_DEPTH_TEST);
    glDisable2(GL_FOG);
    glDisable2(GL_ALPHA_TEST);
    glDisable2(GL_CULL_FACE);
    glDisable2(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(85.0f, (float)boxW / (float)boxH, 0.05f, 10.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);

    // Rotations
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
    glRotatef((ticks + a) * 0.1f, 0.0f, 1.0f, 0.0f);

    std::string panPath = folderPath;
    if (panPath.empty()) panPath = "gui/panorama/";
    if (panPath.back() != '/' && panPath.back() != '\\') panPath += "/";

    for (int i = 0; i < 6; i++) {
        mc->textures->loadAndBindTexture(panPath + "panorama_" + std::to_string(i) + ".png");

        glPushMatrix();
        if (i == 1) glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        if (i == 2) glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        if (i == 3) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        if (i == 4) glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        if (i == 5) glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        t.begin();
        t.vertexUV(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f);
        t.vertexUV( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f);
        t.vertexUV( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f);
        t.vertexUV(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f);
        t.draw();

        glPopMatrix();
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable2(GL_DEPTH_TEST);
    glEnable2(GL_CULL_FACE);
    glEnable2(GL_ALPHA_TEST);

    // Restore full screen viewport & rebind 2D GUI Shader
    glViewport(0, 0, mc->width, mc->height);
    GuiShader::bind();
}

class PanoramaSelectionList : public ScrolledSelectionList {
private:
    PanoramaSelectScreen* parent;

public:
    PanoramaSelectionList(PanoramaSelectScreen* parent, Minecraft* mc, int width, int height, int y0, int y1, int itemHeight)
        : ScrolledSelectionList(mc, width, height, y0, y1, itemHeight), parent(parent) {
        setRenderEdgeShadows(false);
        setRenderSelection(false);
    }

protected:
    virtual int getNumberOfItems() {
        return (int)parent->panoramaList.size();
    }

    virtual void selectItem(int item, bool doubleClick) {
        if (item >= 0 && item < (int)parent->panoramaList.size()) {
            parent->setSelectedPanorama(item);
        }
    }

    virtual bool isSelectedItem(int item) {
        return item == parent->selectedIndex;
    }

    virtual int getMaxPosition() {
        return getNumberOfItems() * itemHeight;
    }

    virtual int getItemAtPosition(int x, int y) {
        if (y < y0 || y > y1) {
            return -1;
        }

        int itemX = parent->leftX + 10;
        int itemW = parent->leftW - 20;

        if (x < itemX || x > itemX + itemW) {
            return -1;
        }

        int clickSlotPos = (int)(y - y0 - headerHeight + (int)yo - 4);
        int slot = clickSlotPos / itemHeight;
        if (slot >= 0 && clickSlotPos >= 0 && slot < getNumberOfItems()) {
            return slot;
        }
        return -1;
    }

    virtual void renderItem(int i, int x, int y, int h, Tesselator& t) {
        if (i < 0 || i >= (int)parent->panoramaList.size()) return;

        const auto& info = parent->panoramaList[i];
        bool isSel = (i == parent->selectedIndex);
        bool isActive = (i == parent->activeIndex);

        int itemW = parent->leftW - 20;
        int itemX = parent->leftX + 10;

        // Selection / Hover frame
        if (isSel) {
            fill(itemX - 2, y - 2, itemX + itemW + 2, y + h + 2, 0x80FFFFFF);
            fill(itemX - 1, y - 1, itemX + itemW + 1, y + h + 1, 0xFF000000);
            fill(itemX, y, itemX + itemW, y + h, 0x60444444);
        } else {
            fill(itemX, y, itemX + itemW, y + h, (i % 2) ? 0x40000000 : 0x60000000);
        }

        // Thumbnail / icon box
        int iconSize = h - 4;
        int iconX = itemX + 3;
        int iconY = y + 2;
        fill(iconX, iconY, iconX + iconSize, iconY + iconSize, 0xff333333);

        // Bind thumbnail texture if path exists
        if (!info.iconPath.empty()) {
            minecraft->textures->loadAndBindTexture(info.iconPath);
            glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);
            t.begin();
            t.vertexUV((float)iconX, (float)(iconY + iconSize), 0, 0, 1);
            t.vertexUV((float)(iconX + iconSize), (float)(iconY + iconSize), 0, 1, 1);
            t.vertexUV((float)(iconX + iconSize), (float)iconY, 0, 1, 0);
            t.vertexUV((float)iconX, (float)iconY, 0, 0, 0);
            t.draw();
        }

        // Text title
        std::string titleStr = info.name;
        if (isActive) {
            titleStr += " (Active)";
        }

        int textX = iconX + iconSize + 6;
        int textColor = isActive ? 0xffff00 : (isSel ? 0xffffff : 0xdddddd);
        drawString(minecraft->font, titleStr.c_str(), textX, y + 5, textColor);
    }

    virtual void renderBackground() {
        // Transparent - allows background to render behind
    }
};

PanoramaSelectScreen::PanoramaSelectScreen()
    : btnDone(nullptr), btnCancel(nullptr), btnDefault(nullptr),
      selectedIndex(0), activeIndex(0), scrolledList(nullptr),
      leftX(0), leftY(0), leftW(0), leftH(0),
      rightX(0), rightY(0), rightW(0), rightH(0) {}

PanoramaSelectScreen::~PanoramaSelectScreen() {
    delete scrolledList;
}

void PanoramaSelectScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();

    panoramaList = PanoramaRegistry::getAllPanoramas();

    std::string currentPath = minecraft ? minecraft->options.getStringValue(OPTIONS_PANORAMA_PATH) : "gui/panorama/";
    selectedIndex = 0;
    activeIndex = 0;

    for (size_t i = 0; i < panoramaList.size(); ++i) {
        if (panoramaList[i].folderPath == currentPath) {
            selectedIndex = (int)i;
            activeIndex = (int)i;
            break;
        }
    }

    btnDone = new Button(0, 0, 0, 140, 20, I18n::get("gui.done"));
    btnCancel = new Button(1, 0, 0, 140, 20, I18n::get("gui.cancel"));
    btnDefault = new Button(2, 0, 0, 80, 20, "DEFAULT");

    buttons.push_back(btnDone);
    buttons.push_back(btnCancel);
    buttons.push_back(btnDefault);

    if (scrolledList) { delete scrolledList; scrolledList = nullptr; }
    scrolledList = new PanoramaSelectionList(this, minecraft, width, height, 35, height - 45, 26);

    setupPositions();
}

void PanoramaSelectScreen::setupPositions() {
    const int pad = 10;
    const int topY = 32;
    const int bottomH = 30;

    int usableW = width - pad * 3;
    leftW = std::min(240, usableW / 2);
    if (leftW < 140) leftW = usableW;

    leftX = pad;
    leftY = topY;
    leftH = height - topY - bottomH - pad;

    rightX = leftX + leftW + pad;
    rightY = topY;
    rightW = width - rightX - pad;
    rightH = leftH;

    if (rightW < 140) {
        rightX = pad;
        rightY = leftY + leftH / 2 + pad;
        rightW = usableW;
        rightH = height - rightY - bottomH - pad;
        leftH = rightY - leftY - pad;
    }

    if (btnDefault) {
        btnDefault->width = 80;
        btnDefault->x = width - pad - 80;
        btnDefault->y = 6;
    }

    int btnW = std::min(140, (width - pad * 3) / 2);
    if (btnDone) {
        btnDone->width = btnW;
        btnDone->x = width / 2 - btnW - 5;
        btnDone->y = height - 26;
    }
    if (btnCancel) {
        btnCancel->width = btnW;
        btnCancel->x = width / 2 + 5;
        btnCancel->y = height - 26;
    }

    if (scrolledList) {
        scrolledList->setXBounds((float)(leftX + 2), (float)(leftX + leftW - 2));
        scrolledList->setDimensions(leftW, height, leftY + 18, leftY + leftH - 5);
    }
}

void PanoramaSelectScreen::setSelectedPanorama(int index) {
    if (index >= 0 && index < (int)panoramaList.size()) {
        selectedIndex = index;
    }
}

void PanoramaSelectScreen::buttonClicked(Button* button) {
    if (button == btnDone) {
        if (selectedIndex >= 0 && selectedIndex < (int)panoramaList.size()) {
            minecraft->options.set(OPTIONS_PANORAMA_PATH, panoramaList[selectedIndex].folderPath);
            minecraft->options.save();
        }
        minecraft->popScreen();
    } else if (button == btnCancel) {
        minecraft->popScreen();
    } else if (button == btnDefault) {
        setSelectedPanorama(0); // Select default PocketMC panorama in list
    }
}

void PanoramaSelectScreen::render(int xm, int ym, float a) {
    // 1. Render active game background panorama
    renderBackground();

    // 2. Draw title header and button default
    drawCenteredString(font, I18n::get("options.panoramaTitle"), width / 2, 10, 0xffffff);

    // 3. Render Left & Right Dark Panels
    fill(leftX, leftY, leftX + leftW, leftY + leftH, 0x90000000);
    fill(rightX, rightY, rightX + rightW, rightY + rightH, 0x90000000);

    // Panel Headers
    drawString(font, I18n::get("options.availablePanoramas"), leftX + 10, leftY + 5, 0xffaaaaaa);
    drawString(font, I18n::get("options.selectedPanoramaPreview"), rightX + 10, rightY + 5, 0xffaaaaaa);

    // 4. Render Left Scrolled List
    if (scrolledList) {
        scrolledList->render(xm, ym, a);
    }

    // 5. Render Right Live 3D Rotating Cubemap Preview Box
    if (selectedIndex >= 0 && selectedIndex < (int)panoramaList.size()) {
        const auto& info = panoramaList[selectedIndex];

        int boxX = rightX + 10;
        int boxY = rightY + 22;
        int boxW = rightW - 20;
        int boxH = std::min(140, rightH - 65);

        if (boxH > 40) {
            // Preview Frame Outer Outline
            fill(boxX - 2, boxY - 2, boxX + boxW + 2, boxY + boxH + 2, 0xff888888);
            fill(boxX - 1, boxY - 1, boxX + boxW + 1, boxY + boxH + 1, 0xff000000);

            // Live 3D Rotating Cubemap Preview
            renderPanoramaPreviewBox(minecraft, info.folderPath, width, height, boxX, boxY, boxW, boxH, (int)Screen::getPanoramaTime(), a);
        }

        // Info Text
        int textY = boxY + boxH + 10;
        std::string selText = "Selected: " + info.name;
        drawString(font, selText.c_str(), boxX, textY, 0xffff00);
        font->drawWordWrap(info.description, (float)boxX, (float)(textY + 14), (float)boxW, 0xffdddddd);
    }

    Screen::render(xm, ym, a);
}

void PanoramaSelectScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
}

void PanoramaSelectScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
}

void PanoramaSelectScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (scrolledList) {
        scrolledList->mouseWheel(dx, dy, xm, ym);
    }
    Screen::mouseWheel(dx, dy, xm, ym);
}

void PanoramaSelectScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void PanoramaSelectScreen::removed() {
}
