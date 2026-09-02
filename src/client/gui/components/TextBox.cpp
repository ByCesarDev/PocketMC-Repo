#include "TextBox.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/input/Mouse.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// delegate constructors
TextBox::TextBox(int id, const std::string& msg)
 : TextBox(id, 0, 0, msg)
{
}

TextBox::TextBox(int id, int x, int y, const std::string& msg)
 : TextBox(id, x, y, 24, Font::DefaultLineHeight + 4, msg)
{
}

TextBox::TextBox(int id, int x, int y, int w, int h, const std::string& msg)
 : GuiElement(true, true, x, y, w, h),
   id(id), hint(msg), focused(false), blink(false), blinkTicks(0), isPassword(false)
{
}

void TextBox::setFocus(Minecraft* minecraft) {
    if (!focused) {
        minecraft->platform()->showKeyboard();
        focused = true;
        blinkTicks = 0;
        blink = false;
    }
}

bool TextBox::loseFocus(Minecraft* minecraft) {
    if (focused) {
        minecraft->platform()->hideKeyboard();
        focused = false;
        return true;
    }
    return false;
}

void TextBox::mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) {
    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (pointInside(x, y)) {
            setFocus(minecraft);
        } else {
            loseFocus(minecraft);
        }
    }
}

void TextBox::charPressed(Minecraft* minecraft, char c)  {
    if (focused && c >= 32 && c < 127 && (int)text.size() < 256) {
        text.push_back(c);
    }
}

void TextBox::keyPressed(Minecraft* minecraft, int key) {
    if (!focused) return;

    bool ctrl = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL);

    if (key == Keyboard::KEY_BACKSPACE && !text.empty()) {
        if (ctrl) {
            // Ctrl+Backspace: borrar palabra entera
            while (!text.empty() && text.back() == ' ') text.pop_back();
            while (!text.empty() && text.back() != ' ') text.pop_back();
        } else {
            text.pop_back();
        }
        return;
    }

    if (ctrl) {
        if (key == Keyboard::KEY_V) {
            // Ctrl+V — pegar desde portapapeles
#if defined(_WIN32)
            if (OpenClipboard(NULL)) {
                HANDLE hData = GetClipboardData(CF_TEXT);
                if (hData) {
                    const char* clipText = static_cast<const char*>(GlobalLock(hData));
                    if (clipText) {
                        for (const char* p = clipText; *p != '\0' && (int)text.size() < 256; ++p) {
                            unsigned char c = (unsigned char)*p;
                            if (c >= 32 && c < 127) {
                                text.push_back((char)c);
                            }
                        }
                        GlobalUnlock(hData);
                    }
                }
                CloseClipboard();
            }
#elif defined(__APPLE__) || defined(USE_GLFW)
            // GLFW clipboard
            extern GLFWwindow* g_glfwWindow;
            if (g_glfwWindow) {
                const char* clipText = glfwGetClipboardString(g_glfwWindow);
                if (clipText) {
                    for (const char* p = clipText; *p != '\0' && (int)text.size() < 256; ++p) {
                        unsigned char c = (unsigned char)*p;
                        if (c >= 32 && c < 127) {
                            text.push_back((char)c);
                        }
                    }
                }
            }
#endif
            return;
        }

        if (key == Keyboard::KEY_A) {
            // Ctrl+A — limpiar campo (equivalente a seleccionar todo)
            text.clear();
            return;
        }

        if (key == Keyboard::KEY_C) {
            // Ctrl+C — copiar al portapapeles
#if defined(_WIN32)
            if (!text.empty() && OpenClipboard(NULL)) {
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
#elif defined(__APPLE__) || defined(USE_GLFW)
            extern GLFWwindow* g_glfwWindow;
            if (g_glfwWindow && !text.empty()) {
                glfwSetClipboardString(g_glfwWindow, text.c_str());
            }
#endif
            return;
        }
    }
}

void TextBox::tick(Minecraft* minecraft) {
    blinkTicks++;
    if (blinkTicks >= 5) {
        blink = !blink;
        blinkTicks = 0;
    }
}

void TextBox::render(Minecraft* minecraft, int xm, int ym) {
    uint32_t borderColor = focused ? 0xffffffff : 0xff484c52;
    uint32_t bgColor = focused ? 0x90101214 : 0x60101214;
    fill(x, y, x + width, y + height, borderColor);
    fill(x + 1, y + 1, x + width - 1, y + height - 1, bgColor);

    glEnable2(GL_SCISSOR_TEST);
    glScissor(
        Gui::GuiScale * (x + 2), 
        minecraft->height - Gui::GuiScale * (y + height - 2), 
        Gui::GuiScale * (width - 2), 
        Gui::GuiScale * (height - 2)
    );

	int _y = y + (height - Font::DefaultLineHeight) / 2;

    if (text.empty() && !focused) {
        drawString(minecraft->font, hint, x + 2, _y, 0xff5e5e5e);
    }

    if (focused && blink) text.push_back('_');

    // Mask characters if this is a password field
    std::string displayText = text;
    if (isPassword && !text.empty()) {
        bool hasCursor = (focused && blink && displayText.back() == '_');
        std::string masked(hasCursor ? displayText.size() - 1 : displayText.size(), '*');
        if (hasCursor) masked.push_back('_');
        displayText = masked;
    }

    drawString(minecraft->font, displayText, x + 2, _y, 0xffffffff);

    if (focused && blink) text.pop_back();

    glDisable2(GL_SCISSOR_TEST);
}
