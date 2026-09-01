#include "ProfileManager.h"
#include "ApiClient.h"
#include "../../platform/log.h"
#include <fstream>
#include <string>

std::string ProfileManager::cacheFilePath = "data/profile_cache.txt";

// ─── Internal JSON extractor ────────────────────────────────────────────────

static std::string extractVal(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    size_t colon = json.find(':', pos);
    if (colon == std::string::npos) return "";
    size_t vStart = colon + 1;
    while (vStart < json.size() && (json[vStart] == ' ' || json[vStart] == '\t')) vStart++;
    if (vStart >= json.size()) return "";
    if (json[vStart] == '"') {
        size_t end = json.find('"', vStart + 1);
        if (end == std::string::npos) return "";
        return json.substr(vStart + 1, end - vStart - 1);
    } else {
        size_t end = vStart;
        while (end < json.size() && json[end] != ',' && json[end] != '}') end++;
        std::string val = json.substr(vStart, end - vStart);
        while (!val.empty() && (val.back() == ' ' || val.back() == '\n' || val.back() == '\r')) val.pop_back();
        return val;
    }
}

// ─── Public methods ─────────────────────────────────────────────────────────

bool ProfileManager::fetchAndCache(const std::string& accessToken, PlayerIdentity& identity) {
    std::string json;
    bool ok = ApiClient::getMe(accessToken, json);

    if (!ok || json.empty()) {
        LOGW("[ProfileManager] fetchAndCache failed — trying local cache.\n");
        return loadFromCache(identity);
    }

    identity.accountType  = AccountType::ONLINE;
    identity.authenticated = true;
    identity.playerId     = extractVal(json, "id");
    identity.username     = extractVal(json, "username");
    identity.displayName  = extractVal(json, "display_name");
    if (identity.displayName.empty()) identity.displayName = identity.username;

    LOGI("[ProfileManager] Profile fetched: @%s (ID: %s)\n",
         identity.username.c_str(), identity.playerId.c_str());

    saveCache(identity);
    return true;
}

bool ProfileManager::loadFromCache(PlayerIdentity& identity) {
    std::ifstream f(cacheFilePath);
    if (!f.is_open()) return false;

    std::string line;
    while (std::getline(f, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        if (key == "playerId")    identity.playerId    = val;
        if (key == "username")    identity.username    = val;
        if (key == "displayName") identity.displayName = val;
    }
    f.close();

    if (!identity.playerId.empty() && !identity.username.empty()) {
        identity.accountType  = AccountType::ONLINE;
        identity.authenticated = false; // offline mode, no live token
        LOGI("[ProfileManager] Loaded from cache: @%s\n", identity.username.c_str());
        return true;
    }

    return false;
}

bool ProfileManager::updateUsername(const std::string& accessToken, const std::string& newUsername, PlayerIdentity& identity) {
    std::string outJson;
    bool ok = ApiClient::updateMe(accessToken, newUsername, outJson);
    if (!ok) {
        LOGW("[ProfileManager] updateUsername failed.\n");
        return false;
    }

    identity.username = extractVal(outJson, "username");
    if (identity.username.empty()) identity.username = newUsername;
    saveCache(identity);
    return true;
}

bool ProfileManager::saveCache(const PlayerIdentity& identity) {
    std::ofstream f(cacheFilePath);
    if (!f.is_open()) return false;

    f << "playerId="    << identity.playerId    << "\n";
    f << "username="    << identity.username    << "\n";
    f << "displayName=" << identity.displayName << "\n";

    f.close();
    LOGI("[ProfileManager] Cache saved.\n");
    return true;
}

void ProfileManager::clearCache() {
    std::ofstream f(cacheFilePath);
    // Just truncate the file — keeps it present but empty
    f.close();
    LOGI("[ProfileManager] Cache cleared.\n");
}
