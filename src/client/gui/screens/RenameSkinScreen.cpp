#include "RenameSkinScreen.h"
#include "SkindexScreen.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../Font.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../AppPlatform.h"
#include "../../../locale/I18n.h"
#include <cstdio>
#include <algorithm>

RenameSkinScreen::RenameSkinScreen(const std::string& skinPath, int packIndex, int skinIndex)
:   _btnDone(0, I18n::get("gui.done")),
    _btnCancel(1, I18n::get("gui.cancel")),
    tName(0, ""),
    _skinPath(skinPath),
    _packIndex(packIndex),
    _skinIndex(skinIndex)
{
}

RenameSkinScreen::~RenameSkinScreen() {}

#include <fstream>
#include <iostream>

static std::string readCurrentLocalizationName(const std::string& packDir, const std::string& textureFilename) {
    std::string skinsJsonPath = packDir + "/skins.json";
    std::ifstream inFile(skinsJsonPath);
    if (!inFile.good()) return "";

    std::string content((std::istreambuf_iterator<char>(inFile)),
                        std::istreambuf_iterator<char>());
    inFile.close();

    size_t texPos = content.find("\"" + textureFilename + "\"");
    if (texPos != std::string::npos) {
        size_t startObj = content.rfind('{', texPos);
        size_t endObj = content.find('}', texPos);
        if (startObj != std::string::npos && endObj != std::string::npos && startObj < endObj) {
            std::string objStr = content.substr(startObj, endObj - startObj + 1);
            size_t locPos = objStr.find("\"localization_name\"");
            if (locPos != std::string::npos) {
                size_t colon = objStr.find(":", locPos);
                if (colon != std::string::npos) {
                    size_t q1 = objStr.find("\"", colon);
                    if (q1 != std::string::npos) {
                        size_t q2 = objStr.find("\"", q1 + 1);
                        if (q2 != std::string::npos) {
                            return objStr.substr(q1 + 1, q2 - q1 - 1);
                        }
                    }
                }
            }
        }
    }
    return "";
}

static void updateLocalizationNameInSkinsJson(const std::string& packDir, const std::string& textureFilename, const std::string& newLocName) {
    std::string skinsJsonPath = packDir + "/skins.json";
    std::ifstream inFile(skinsJsonPath);
    if (!inFile.good()) return;

    std::string content((std::istreambuf_iterator<char>(inFile)),
                        std::istreambuf_iterator<char>());
    inFile.close();

    size_t texPos = content.find("\"" + textureFilename + "\"");
    if (texPos != std::string::npos) {
        size_t startObj = content.rfind('{', texPos);
        size_t endObj = content.find('}', texPos);
        if (startObj != std::string::npos && endObj != std::string::npos && startObj < endObj) {
            std::string objStr = content.substr(startObj, endObj - startObj + 1);
            size_t locPos = objStr.find("\"localization_name\"");
            if (locPos != std::string::npos) {
                size_t colon = objStr.find(":", locPos);
                if (colon != std::string::npos) {
                    size_t q1 = objStr.find("\"", colon);
                    if (q1 != std::string::npos) {
                        size_t q2 = objStr.find("\"", q1 + 1);
                        if (q2 != std::string::npos) {
                            std::string updatedObj = objStr;
                            updatedObj.replace(q1 + 1, q2 - q1 - 1, newLocName);
                            content.replace(startObj, endObj - startObj + 1, updatedObj);
                        }
                    }
                }
            } else {
                std::string updatedObj = objStr;
                size_t firstBrace = updatedObj.find('{');
                if (firstBrace != std::string::npos) {
                    std::string insertStr = "\n      \"localization_name\": \"" + newLocName + "\",";
                    updatedObj.insert(firstBrace + 1, insertStr);
                    content.replace(startObj, endObj - startObj + 1, updatedObj);
                }
            }

            std::ofstream outFile(skinsJsonPath);
            if (outFile.is_open()) {
                outFile << content;
                outFile.close();
            }
        }
    }
}

void RenameSkinScreen::init() {
    _btnDone.active = false;
    buttons.push_back(&_btnDone);
    buttons.push_back(&_btnCancel);
    textBoxes.push_back(&tName);
    
    std::string folder = "";
    std::string textureFilename = _skinPath;
    size_t pos = _skinPath.find_last_of("\\/");
    if (pos != std::string::npos) {
        folder = _skinPath.substr(0, pos);
        textureFilename = _skinPath.substr(pos + 1);
    }
    
    std::string currentLocName = readCurrentLocalizationName(folder, textureFilename);
    if (currentLocName.empty()) {
        currentLocName = textureFilename;
        if (currentLocName.length() > 4 && currentLocName.substr(currentLocName.length() - 4) == ".png") {
            currentLocName = currentLocName.substr(0, currentLocName.length() - 4);
        }
    }

    tName.text = currentLocName;
    _btnDone.active = !tName.text.empty();
    
    setupPositions();
}

void RenameSkinScreen::setupPositions() {
    int cx = width / 2;
    int cy = height / 2;
    
    tName.width = 150;
    tName.height = 20;
    tName.x = (width - tName.width) / 2;
    tName.y = cy - 20;
    
    _btnCancel.width = 70;
    _btnCancel.height = 24;
    _btnCancel.x = width / 2 - _btnCancel.width - 4;
    _btnCancel.y = cy + 20;
    
    _btnDone.width = 70;
    _btnDone.height = 24;
    _btnDone.x = width / 2 + 4;
    _btnDone.y = cy + 20;
}

void RenameSkinScreen::tick() {
    for (auto* tb : textBoxes) {
        tb->tick(minecraft);
    }
}

void RenameSkinScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_RETURN) {
        if (!tName.text.empty()) {
            buttonClicked(&_btnDone);
        }
    }
    Screen::keyPressed(eventKey);
    _btnDone.active = !tName.text.empty();
}

void RenameSkinScreen::removed() {
    minecraft->platform()->hideKeyboard();
}

void RenameSkinScreen::buttonClicked(Button* button) {
    if (button == &_btnCancel) {
        minecraft->setScreen(new SkindexScreen(_packIndex, _skinIndex));
    } else if (button == &_btnDone && !tName.text.empty()) {
        std::string newName = tName.text;
        // Clean display name of quotes or newlines
        static char ILLEGAL_NAME_CHARACTERS[] = {
            '\n', '\r', '\t', '\0', '\f', '`', '\\', '"', ':'
        };
        for (int i = 0; i < sizeof(ILLEGAL_NAME_CHARACTERS) / sizeof(char); ++i) {
            size_t pos;
            while ((pos = newName.find(ILLEGAL_NAME_CHARACTERS[i])) != std::string::npos) {
                newName.erase(pos, 1);
            }
        }
        
        if (!newName.empty()) {
            std::string folder = "";
            std::string textureFilename = _skinPath;
            size_t pos = _skinPath.find_last_of("\\/");
            if (pos != std::string::npos) {
                folder = _skinPath.substr(0, pos);
                textureFilename = _skinPath.substr(pos + 1);
            }
            
            updateLocalizationNameInSkinsJson(folder, textureFilename, newName);
        }
        minecraft->setScreen(new SkindexScreen(_packIndex, _skinIndex));
    }
}

void RenameSkinScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("skindex.renameTitle"), width / 2, height / 2 - 50, 0xffffff);
    super::render(xm, ym, a);
}
