#include "ConsoleScreen.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../world/level/Level.h"
#include "../../../network/RakNetInstance.h"
#include "../../../network/ServerSideNetworkHandler.h"
#include "../../../network/packet/ChatPacket.h"
#include "../../../platform/log.h"
#include "../../../world/item/ItemInstance.h"
#include "../../../world/item/Item.h"
#include "../../../world/level/tile/Tile.h"

#include <sstream>
#include <cstdlib>
#include <cctype>

ConsoleScreen::ConsoleScreen()
:   _input(""),
    _cursorBlink(0)
{
}

void ConsoleScreen::init()
{
}

void ConsoleScreen::tick()
{
    _cursorBlink++;
}

bool ConsoleScreen::handleBackEvent(bool /*isDown*/)
{
    minecraft->setScreen(NULL);
    return true;
}

void ConsoleScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(NULL);
    } else if (eventKey == Keyboard::KEY_RETURN) {
        execute();
    } else if (eventKey == Keyboard::KEY_BACKSPACE) {
        if (!_input.empty())
            _input.erase(_input.size() - 1, 1);
    } else {
        super::keyPressed(eventKey);
    }
}

void ConsoleScreen::charPressed(char inputChar)
{
    if (inputChar >= 32 && inputChar < 127)
        _input += inputChar;
}

// ---------------------------------------------------------------------------
// execute: run _input as a command, print result, close screen
// ---------------------------------------------------------------------------
void ConsoleScreen::execute()
{
    if (_input.empty()) {
        minecraft->setScreen(NULL);
        return;
    }

    if (_input[0] == '/') {
        // Command
        std::string result = processCommand(_input);
        
        // If the client doesn't recognize it, but we're connected to a server, forward it
        if (result.find("Unknown command") == 0 && minecraft->netCallback && !minecraft->raknetInstance->isServer()) {
            ChatPacket chatPkt(_input);
            minecraft->raknetInstance->send(chatPkt);
        }
        else if (!result.empty()) {
            minecraft->gui.addMessage(result);
        }
    } else {
        // Chat message: <name> message
        std::string msg = std::string("<") + minecraft->player->name + "> " + _input;
        if (minecraft->netCallback && minecraft->raknetInstance->isServer()) {
            // Hosting a LAN game: displayGameMessage shows locally + broadcasts MessagePacket to clients
            static_cast<ServerSideNetworkHandler*>(minecraft->netCallback)->displayGameMessage(msg);
        } else if (minecraft->netCallback) {
            // Connected client: send ChatPacket to server; server echoes it back as MessagePacket
            ChatPacket chatPkt(msg);
            minecraft->raknetInstance->send(chatPkt);
        } else {
            // Singleplayer: show locally only
            minecraft->gui.addMessage(msg);
        }
    }

    minecraft->setScreen(NULL);
}

// ---------------------------------------------------------------------------
// processCommand
// ---------------------------------------------------------------------------
static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t");
    return s.substr(a, b - a + 1);
}

std::string ConsoleScreen::processCommand(const std::string& raw)
{
    // strip leading '/'
    std::string line = raw;
    if (!line.empty() && line[0] == '/') line = line.substr(1);
    line = trim(line);

    // tokenise
    std::vector<std::string> args;
    {
        std::istringstream ss(line);
        std::string tok;
        while (ss >> tok) args.push_back(tok);
    }

    if (args.empty()) return "";

    Level* level = minecraft->level;
    if (!level) return "No level loaded.";

    // -----------------------------------------------------------------------
    // /give <id> [cantidad]
    // -----------------------------------------------------------------------
    if (args[0] == "give") {
        if (args.size() < 2) return "Usage: /give <id> [cantidad]";

        // Permission: either cheats allowed or creative mode
        if (!level->getLevelData()->getAllowCheats() && level->getLevelData()->getGameType() != GameType::Creative)
            return "Cheats are not allowed on this level.";

        int id = std::atoi(args[1].c_str());
        int count = 1;
        if (args.size() >= 3) {
            count = std::atoi(args[2].c_str());
            if (count <= 0) count = 1;
            if (count > 64) count = 64;
        }

        if (!minecraft->player) return "No player available.";

        ItemInstance* instance = NULL;

        if (id >= 0 && id < Tile::NUM_BLOCK_TYPES && Tile::tiles[id]) {
            instance = new ItemInstance(Tile::tiles[id], count);
        } else if (id >= 0 && id < Item::MAX_ITEMS && Item::items[id]) {
            instance = new ItemInstance(Item::items[id], count);
        } else {
            return std::string("Unknown id: ") + args[1];
        }

        bool added = minecraft->player->inventory->add(instance);

        std::ostringstream out;
        if (added) {
            out << "Gave " << count << " of id " << id << " to " << minecraft->player->name;
            return out.str();
        } else {
            // Inventory full or couldn't add: drop at player's feet
            minecraft->player->drop(instance, false);
            out << "Inventory full, dropped " << count << " of id " << id;
            return out.str();
        }
    }

    // -----------------------------------------------------------------------
    // /gamemode <0/1/creative/survival>
    // -----------------------------------------------------------------------
    if (args[0] == "gamemode" || args[0] == "gm") {
        if (args.size() < 2) return "Usage: /gamemode <0/1/survival/creative>";

        std::string mode = args[1];
        bool creative = false;
        bool valid = false;

        if (mode == "1" || mode == "creative" || mode == "c") {
            creative = true;
            valid = true;
        } else if (mode == "0" || mode == "survival" || mode == "s") {
            creative = false;
            valid = true;
        }

        if (valid) {
            minecraft->setIsCreativeMode(creative);
            return "Your game mode has been updated to " + std::string(creative ? "Creative" : "Survival");
        } else {
            return "Unknown gamemode: " + mode;
        }
    }

    // -----------------------------------------------------------------------
    // /time ...
    // -----------------------------------------------------------------------
    if (args[0] == "time") {
        if (args.size() < 2)
            return "Usage: /time <add|set|query> ...";

        const std::string& sub = args[1];

        // -- time add <value> -----------------------------------------------
        if (sub == "add") {
            if (args.size() < 3) return "Usage: /time add <value>";
            long delta = std::atol(args[2].c_str());
            long newTime = level->getTime() + delta;
            level->setTime(newTime);
            std::ostringstream out;
            out << "Set the time to " << (newTime % Level::TICKS_PER_DAY);
            return out.str();
        }

        // -- time set <value|day|night|noon|midnight> -----------------------
        if (sub == "set") {
            if (args.size() < 3) return "Usage: /time set <value|day|night|noon|midnight>";
            const std::string& val = args[2];

            long t = -1;
            if      (val == "day")      t = 1000;
            else if (val == "noon")     t = 6000;
            else if (val == "night")    t = 13000;
            else if (val == "midnight") t = 18000;
            else {
                // numeric â€” accept positive integers only
                bool numeric = true;
                for (char c : val)
                    if (!std::isdigit((unsigned char)c)) { numeric = false; break; }
                if (!numeric) return std::string("Unknown value: ") + val;
                t = std::atol(val.c_str());
            }

            // Preserve the total day count so only the time-of-day changes
            long dayCount = level->getTime() / Level::TICKS_PER_DAY;
            long newTime  = dayCount * Level::TICKS_PER_DAY + (t % Level::TICKS_PER_DAY);
            level->setTime(newTime);
            std::ostringstream out;
            out << "Set the time to " << t;
            return out.str();
        }

        // -- time query <daytime|gametime|day> ------------------------------
        if (sub == "query") {
            if (args.size() < 3) return "Usage: /time query <daytime|gametime|day>";
            const std::string& what = args[2];

            long total   = level->getTime();
            long daytime = total % Level::TICKS_PER_DAY;
            long day     = total / Level::TICKS_PER_DAY;

            std::ostringstream out;
            if      (what == "daytime")  { out << "The time of day is " << daytime; }
            else if (what == "gametime") { out << "The game time is "   << total;   }
            else if (what == "day")      { out << "The day is "         << day;     }
            else return std::string("Unknown query: ") + what;
            return out.str();
        }

        return "Unknown sub-command. Usage: /time <add|set|query> ...";
    }

    // -----------------------------------------------------------------------
    // /fly
    // -----------------------------------------------------------------------
    if (args[0] == "fly") {
        if (!minecraft->player) return "No player available.";
        minecraft->player->abilities.mayfly = !minecraft->player->abilities.mayfly;
        if (!minecraft->player->abilities.mayfly) {
            minecraft->player->abilities.flying = false;
        }
        std::string status = minecraft->player->abilities.mayfly ? "enabled" : "disabled";
        return "Flight " + status + " for " + minecraft->player->name;
    }

    // -----------------------------------------------------------------------
    // /tp <dimension> <x> <y> <z>
    // -----------------------------------------------------------------------
    if (args[0] == "tp") {
        if (args.size() < 5) return "Usage: /tp <dimension> <x> <y> <z>";
        if (!minecraft->player) return "No player available.";

        try {
            int dim = std::atoi(args[1].c_str());
            float x = std::atof(args[2].c_str());
            float y = std::atof(args[3].c_str());
            float z = std::atof(args[4].c_str());

            if (level->dimension->id != dim) {
                level->changeDimension(dim);
            }
            minecraft->player->moveTo(x, y, z, minecraft->player->yRot, minecraft->player->xRot);
            minecraft->player->xd = minecraft->player->yd = minecraft->player->zd = 0;
            return "Teleported to dimension " + std::to_string(dim) + " at " + std::to_string((int)x) + ", " + std::to_string((int)y) + ", " + std::to_string((int)z);
        } catch (...) {
            return "Invalid coordinates or dimension.";
        }
    }

    return std::string("Unknown command: /") + args[0];
}

// ---------------------------------------------------------------------------
// render
// ---------------------------------------------------------------------------
void ConsoleScreen::render(int /*xm*/, int /*ym*/, float /*a*/)
{
    // Dim the game world slightly
    fillGradient(0, 0, width, height, 0x00000000, 0x40000000);

    const int boxH  = 12;
    const int boxY  = height - boxH - 2;
    const int boxX0 = 2;
    const int boxX1 = width - 2;

    // Input box background
    fill(boxX0, boxY, boxX1, boxY + boxH, 0xc0000000);
    // Border
    fill(boxX0,     boxY,          boxX1, boxY + 1,   0xff808080);
    fill(boxX0,     boxY + boxH - 1, boxX1, boxY + boxH, 0xff808080);
    fill(boxX0,     boxY,          boxX0 + 1, boxY + boxH, 0xff808080);
    fill(boxX1 - 1, boxY,          boxX1,     boxY + boxH, 0xff808080);

    // Input text + blinking cursor
    std::string displayed = _input;
    if ((_cursorBlink / 10) % 2 == 0)
        displayed += '_';

    // Placeholder hint when empty
    font->drawShadow(displayed, (float)(boxX0 + 2), (float)(boxY + 2), 0xffffffff);
}
