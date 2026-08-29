#include "SafeAreaScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "locale/I18n.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

SafeAreaScreen::SafeAreaScreen()
    : btnConfirm(nullptr), isDraggingSlider(false),
      dialogX(0), dialogY(0), dialogW(260), dialogH(136),
      sliderX(0), sliderY(0), sliderW(240), sliderH(16) {}

SafeAreaScreen::~SafeAreaScreen() {}

void SafeAreaScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();

    btnConfirm = new Button(0, 0, 0, 200, 20, I18n::get("gui.done"));
    buttons.push_back(btnConfirm);

    setupPositions();
}

void SafeAreaScreen::setupPositions() {
    dialogW = std::min(width - 30, 260);
    dialogH = 140;
    dialogX = width / 2 - dialogW / 2;
    dialogY = height / 2 - dialogH / 2;

    sliderX = dialogX + 12;
    sliderY = dialogY + 36;
    sliderW = dialogW - 24;
    sliderH = 16;

    if (btnConfirm) {
        btnConfirm->width = dialogW - 24;
        btnConfirm->height = 20;
        btnConfirm->x = dialogX + 12;
        btnConfirm->y = dialogY + dialogH - 28;
    }
}

void SafeAreaScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    }
}

void SafeAreaScreen::drawCornerGuides(int marginX, int marginY) {
    const int len = 20;
    const int thick = 3;
    const int pad = 8;

    int left = marginX + pad;
    int right = width - marginX - pad;
    int top = marginY + pad;
    int bottom = height - marginY - pad;

    // Drop shadow color & white color
    int shadowCol = 0x80000000;
    int bracketCol = 0xffffffff;

    // Top-Left (⌜)
    fill(left + 1, top + 1, left + len + 1, top + thick + 1, shadowCol);
    fill(left + 1, top + 1, left + thick + 1, top + len + 1, shadowCol);
    fill(left, top, left + len, top + thick, bracketCol);
    fill(left, top, left + thick, top + len, bracketCol);

    // Top-Right (⌝)
    fill(right - len + 1, top + 1, right + 1, top + thick + 1, shadowCol);
    fill(right - thick + 1, top + 1, right + 1, top + len + 1, shadowCol);
    fill(right - len, top, right, top + thick, bracketCol);
    fill(right - thick, top, right, top + len, bracketCol);

    // Bottom-Left (⌞)
    fill(left + 1, bottom - thick + 1, left + len + 1, bottom + 1, shadowCol);
    fill(left + 1, bottom - len + 1, left + thick + 1, bottom + 1, shadowCol);
    fill(left, bottom - thick, left + len, bottom, bracketCol);
    fill(left, bottom - len, left + thick, bottom, bracketCol);

    // Bottom-Right (⌟)
    fill(right - len + 1, bottom - thick + 1, right + 1, bottom + 1, shadowCol);
    fill(right - thick + 1, bottom - len + 1, right + 1, bottom + 1, shadowCol);
    fill(right - len, bottom - thick, right, bottom, bracketCol);
    fill(right - thick, bottom - len, right, bottom, bracketCol);
}

void SafeAreaScreen::render(int xm, int ym, float a) {
    // Semi-transparent dark backdrop
    fill(0, 0, width, height, 0x70000000);

    float val = minecraft->options.getProgressValue(OPTIONS_SAFE_AREA);
    if (val < 0.5f) val = 0.5f;
    if (val > 1.0f) val = 1.0f;

    int marginX = (int)((1.0f - val) * (width / 2.0f));
    int marginY = (int)((1.0f - val) * (height / 2.0f));

    // Draw the 4 corner guides
    drawCornerGuides(marginX, marginY);

    // Dialog Window Background (Bedrock style)
    fill(dialogX - 1, dialogY - 1, dialogX + dialogW + 1, dialogY + dialogH + 1, 0xffffffff);
    fill(dialogX, dialogY, dialogX + dialogW, dialogY + dialogH, 0xf0161616);

    // Header Title
    drawString(font, I18n::get("options.safeArea.title"), dialogX + 12, dialogY + 8, 0xffffff);

    // Safe Area Value Label
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << (val * 100.0f);
    std::string labelText = I18n::get("options.safeArea.label") + ss.str() + "%";
    drawString(font, labelText, dialogX + 12, dialogY + 23, 0xcccccc);

    // Slider Bar
    if (isDraggingSlider) {
        float relX = (float)(xm - sliderX) / (float)sliderW;
        relX = std::max(0.0f, std::min(1.0f, relX));
        float newVal = 0.5f + relX * 0.5f;
        minecraft->options.set(OPTIONS_SAFE_AREA, newVal);
        val = newVal;
    }

    float normProgress = (val - 0.5f) / 0.5f;
    normProgress = std::max(0.0f, std::min(1.0f, normProgress));

    // Slider Track
    fill(sliderX, sliderY, sliderX + sliderW, sliderY + sliderH, 0xff3b3b3b);
    fill(sliderX, sliderY, sliderX + (int)(normProgress * sliderW), sliderY + sliderH, 0xff5c5c5c);
    fill(sliderX, sliderY, sliderX + sliderW, sliderY + 1, 0xff222222);
    fill(sliderX, sliderY, sliderX + 1, sliderY + sliderH, 0xff222222);

    // Slider Thumb Handle
    int thumbX = sliderX + (int)(normProgress * (sliderW - 8));
    fill(thumbX, sliderY - 2, thumbX + 8, sliderY + sliderH + 2, 0xffffffff);
    fill(thumbX + 1, sliderY - 1, thumbX + 7, sliderY + sliderH + 1, 0xffcccccc);

    // Description text (word-wrapped or split into 2 lines)
    std::string desc = I18n::get("options.safeArea.desc");
    if (font->width(desc) > dialogW - 24) {
        // Split text nicely
        size_t mid = desc.length() / 2;
        size_t spacePos = desc.find_last_of(' ', mid);
        if (spacePos != std::string::npos) {
            std::string line1 = desc.substr(0, spacePos);
            std::string line2 = desc.substr(spacePos + 1);
            drawString(font, line1, dialogX + 12, dialogY + 58, 0xaaaaaa);
            drawString(font, line2, dialogX + 12, dialogY + 70, 0xaaaaaa);
        } else {
            drawString(font, desc, dialogX + 12, dialogY + 62, 0xaaaaaa);
        }
    } else {
        drawString(font, desc, dialogX + 12, dialogY + 62, 0xaaaaaa);
    }

    Screen::render(xm, ym, a);
}

void SafeAreaScreen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum == 0) {
        if (x >= sliderX && x <= sliderX + sliderW && y >= sliderY - 2 && y <= sliderY + sliderH + 2) {
            isDraggingSlider = true;
            float relX = (float)(x - sliderX) / (float)sliderW;
            relX = std::max(0.0f, std::min(1.0f, relX));
            float newVal = 0.5f + relX * 0.5f;
            minecraft->options.set(OPTIONS_SAFE_AREA, newVal);
        }
    }
    Screen::mouseClicked(x, y, buttonNum);
}

void SafeAreaScreen::mouseReleased(int x, int y, int buttonNum) {
    if (buttonNum == 0) {
        isDraggingSlider = false;
    }
    Screen::mouseReleased(x, y, buttonNum);
}

void SafeAreaScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}

void SafeAreaScreen::removed() {
    if (minecraft) {
        minecraft->options.save();
    }
}
