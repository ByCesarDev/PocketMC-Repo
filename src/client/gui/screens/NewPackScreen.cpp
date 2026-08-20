#include "NewPackScreen.h"
#include "SkindexScreen.h"
#include "../../Minecraft.h"
#include "../Font.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../AppPlatform.h"
#include "../../../locale/I18n.h"
#include "../../../world/level/storage/FolderMethods.h"
#include <algorithm>

NewPackScreen::NewPackScreen()
:   _btnDone(0, I18n::get("gui.done")),
    _btnCancel(1, I18n::get("gui.cancel")),
    tName(0, "")
{
}

NewPackScreen::~NewPackScreen() {}

void NewPackScreen::init() {
    _btnDone.active = false;
    buttons.push_back(&_btnDone);
    buttons.push_back(&_btnCancel);
    textBoxes.push_back(&tName);
    
    tName.text = "";
    setupPositions();
}

void NewPackScreen::setupPositions() {
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

void NewPackScreen::tick() {
    for (auto* tb : textBoxes) {
        tb->tick(minecraft);
    }
}

void NewPackScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_RETURN) {
        if (!tName.text.empty()) {
            buttonClicked(&_btnDone);
        }
    }
    Screen::keyPressed(eventKey);
    _btnDone.active = !tName.text.empty();
}

void NewPackScreen::removed() {
    minecraft->platform()->hideKeyboard();
}

#include <fstream>
#include <cstdlib>
#include <cstdio>

static std::string generateSimpleUUID() {
    char uuidStr[37];
    snprintf(uuidStr, sizeof(uuidStr), "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
        rand() % 0x10000, rand() % 0x10000,
        rand() % 0x10000,
        (rand() % 0x1000) | 0x4000,
        (rand() % 0x4000) | 0x8000,
        rand() % 0x10000, rand() % 0x10000, rand() % 0x10000);
    return std::string(uuidStr);
}

void NewPackScreen::buttonClicked(Button* button) {
    if (button == &_btnCancel) {
        minecraft->setScreen(new SkindexScreen());
    } else if (button == &_btnDone && !tName.text.empty()) {
        std::string newName = tName.text;
        // Clean folder name
        static char ILLEGAL_FILE_CHARACTERS[] = {
            '/', '\n', '\r', '\t', '\0', '\f', '`', '?', '*', '\\', '<', '>', '|', '\"', ':'
        };
        for (int i = 0; i < sizeof(ILLEGAL_FILE_CHARACTERS) / sizeof(char); ++i) {
            size_t pos;
            while ((pos = newName.find(ILLEGAL_FILE_CHARACTERS[i])) != std::string::npos) {
                newName.erase(pos, 1);
            }
        }
        
        if (!newName.empty()) {
            createFolderIfNotExists("games");
            createFolderIfNotExists("games/com.mojang");
            createFolderIfNotExists("games/com.mojang/skins");
            std::string packDir = "games/com.mojang/skins/" + newName;
            createFolderIfNotExists(packDir.c_str());

            // 1. Generate manifest.json
            std::ofstream manifestFile(packDir + "/manifest.json");
            if (manifestFile.is_open()) {
                manifestFile << "{\n"
                             << "  \"format_version\": 1,\n"
                             << "  \"header\": {\n"
                             << "    \"name\": \"" << newName << "\",\n"
                             << "    \"description\": \"Custom Skin Pack\",\n"
                             << "    \"version\": [1, 0, 0],\n"
                             << "    \"uuid\": \"" << generateSimpleUUID() << "\"\n"
                             << "  },\n"
                             << "  \"modules\": [\n"
                             << "    {\n"
                             << "      \"type\": \"skin_pack\",\n"
                             << "      \"uuid\": \"" << generateSimpleUUID() << "\",\n"
                             << "      \"version\": [1, 0, 0]\n"
                             << "    }\n"
                             << "  ]\n"
                             << "}\n";
                manifestFile.close();
            }

            // 2. Generate skins.json
            std::ofstream skinsFile(packDir + "/skins.json");
            if (skinsFile.is_open()) {
                skinsFile << "{\n"
                          << "  \"serialize_name\": \"" << newName << "\",\n"
                          << "  \"localization_name\": \"" << newName << "\",\n"
                          << "  \"skins\": []\n"
                          << "}\n";
                skinsFile.close();
            }
        }
        minecraft->setScreen(new SkindexScreen());
    }
}

void NewPackScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, I18n::get("skindex.newPackTitle"), width / 2, height / 2 - 50, 0xffffff);
    super::render(xm, ym, a);
}
