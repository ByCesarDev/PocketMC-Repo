#include "AccountManager.h"
#include "ProfileManager.h"
#include "ApiClient.h"
#include "../../platform/HttpClient.h"
#include "../../platform/log.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// ─── Supabase anon key ───────────────────────────────────────────────────────
// This is the public anon key — safe to embed in the client.
static const std::string SUPABASE_URL     = "https://nzkzkwnfpnzplsqekggv.supabase.co";
static const std::string SUPABASE_ANON_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im56a3prd25mcG56cGxzcWVrZ2d2Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODgyODk0NjgsImV4cCI6MjEwMzg2NTQ2OH0.qt-_QhOV78F_-06pbh37S_48UYNl-FZwOAErBaLRhsw";

// ─── Static members ──────────────────────────────────────────────────────────
PlayerIdentity AccountManager::m_identity;
std::string    AccountManager::m_accessToken;
std::string    AccountManager::m_refreshToken;
std::string    AccountManager::m_sessionFilePath = "data/account_session.txt";

// ─── JSON value extractor ────────────────────────────────────────────────────
static std::string jsonVal(const std::string& json, const std::string& key) {
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
    }
    size_t end = vStart;
    while (end < json.size() && json[end] != ',' && json[end] != '}') end++;
    std::string val = json.substr(vStart, end - vStart);
    while (!val.empty() && (val.back() == ' ' || val.back() == '\n' || val.back() == '\r')) val.pop_back();
    return val;
}

// ─── init ────────────────────────────────────────────────────────────────────
void AccountManager::init(const std::string& dataDir) {
    if (!dataDir.empty()) {
        m_sessionFilePath = dataDir + "/account_session.txt";
        ProfileManager::cacheFilePath = dataDir + "/profile_cache.txt";
    }

    // Fase 23 — API URL from env var (dev vs production)
    // On Windows, check environment variable PMC_API_URL.
    // Default: http://localhost:3000
    std::string apiUrl;
#if defined(_WIN32)
    char envBuf[512] = {};
    DWORD len = GetEnvironmentVariableA("PMC_API_URL", envBuf, sizeof(envBuf));
    if (len > 0) apiUrl = std::string(envBuf, len);
#else
    const char* envVal = getenv("PMC_API_URL");
    if (envVal) apiUrl = envVal;
#endif
    ApiClient::init(apiUrl);

    if (loadSession()) {
        // 1. Immediately hydrate profile from local cache so identity is ready offline
        ProfileManager::loadFromCache(m_identity);
        m_identity.accountType   = AccountType::ONLINE;
        m_identity.authenticated = true;

        // 2. Try to refresh session via Supabase refresh token or access token
        if (!m_refreshToken.empty()) {
            refreshSession();
        } else if (!m_accessToken.empty()) {
            ProfileManager::fetchAndCache(m_accessToken, m_identity);
        }
    }
}

// ─── Getters ─────────────────────────────────────────────────────────────────
const PlayerIdentity& AccountManager::getIdentity() { return m_identity; }

bool AccountManager::isOnline() {
    return m_identity.accountType == AccountType::ONLINE;
}

std::string AccountManager::getUsername(const std::string& fallback) {
    if (m_identity.accountType == AccountType::ONLINE && !m_identity.username.empty()) {
        return m_identity.username;
    }
    return fallback;
}

std::string AccountManager::getDisplayName(const std::string& fallback) {
    if (m_identity.accountType == AccountType::ONLINE) {
        if (!m_identity.displayName.empty()) return m_identity.displayName;
        if (!m_identity.username.empty()) return m_identity.username;
    }
    return fallback;
}

std::string AccountManager::getPlayerId()     { return m_identity.playerId; }
std::string AccountManager::getAccessToken()  { return m_accessToken; }
std::string AccountManager::getRefreshToken() { return m_refreshToken; }
std::string AccountManager::getAccountTypeString() {
    return m_identity.getAccountTypeString();
}

// ─── loginWithSupabase ────────────────────────────────────────────────────────
bool AccountManager::loginWithSupabase(const std::string& identifier, const std::string& password, std::string& errorMsgOut) {
    if (identifier.empty() || password.empty()) {
        errorMsgOut = "Username and password are required.";
        return false;
    }

    std::string loginEmail = identifier;

    // If identifier is a username (no @), resolve to email via Supabase RPC
    if (identifier.find('@') == std::string::npos) {
        std::string rpcUrl = SUPABASE_URL + "/rest/v1/rpc/get_user_email_by_username";
        std::string rpcJson = "{\"p_username\":\"" + identifier + "\"}";
        std::string rpcHeaders = "apikey: " + SUPABASE_ANON_KEY + "\r\n";
        std::vector<unsigned char> rpcBody;

        if (HttpClient::post(rpcUrl, rpcJson, rpcBody, rpcHeaders) && !rpcBody.empty()) {
            std::string rpcResp(rpcBody.begin(), rpcBody.end());
            // RPC returns "email@domain.com" or {"email":"..."}
            // Strip surrounding quotes
            std::string email = rpcResp;
            while (!email.empty() && (email.front() == '"' || email.front() == ' ' || email.front() == '\n' || email.front() == '\r')) email.erase(email.begin());
            while (!email.empty() && (email.back() == '"' || email.back() == ' ' || email.back() == '\n' || email.back() == '\r')) email.pop_back();
            if (email.find('@') != std::string::npos) {
                loginEmail = email;
                LOGI("[AccountManager] Resolved @%s to email: %s\n", identifier.c_str(), loginEmail.c_str());
            }
        }
    }

    // Direct Supabase Auth
    std::string url = SUPABASE_URL + "/auth/v1/token?grant_type=password";
    std::string jsonBody = "{\"email\":\"" + loginEmail + "\",\"password\":\"" + password + "\"}";
    std::string extraHeaders = "apikey: " + SUPABASE_ANON_KEY + "\r\n";

    std::vector<unsigned char> outBody;
    LOGI("[AccountManager] Logging in as %s (email: %s)\n", identifier.c_str(), loginEmail.c_str());

    bool ok = HttpClient::post(url, jsonBody, outBody, extraHeaders);

    std::string responseStr;
    if (!outBody.empty()) responseStr.assign(outBody.begin(), outBody.end());

    if (responseStr.empty()) {
        errorMsgOut = "Failed to connect to authentication server.";
        return false;
    }

    std::string accessToken  = jsonVal(responseStr, "access_token");
    std::string refreshToken = jsonVal(responseStr, "refresh_token");

    if (accessToken.empty()) {
        std::string err = jsonVal(responseStr, "error_description");
        if (err.empty()) err = jsonVal(responseStr, "msg");
        if (err.empty()) err = jsonVal(responseStr, "message");
        if (err.empty()) err = "Invalid username or password.";
        errorMsgOut = err;
        return false;
    }

    m_accessToken  = accessToken;
    m_refreshToken = refreshToken;

    // Fetch full profile via ApiClient + ProfileManager
    m_identity.accountType   = AccountType::ONLINE;
    m_identity.authenticated = true;
    ProfileManager::fetchAndCache(m_accessToken, m_identity);

    saveSession();
    return true;
}

// ─── logout ──────────────────────────────────────────────────────────────────
void AccountManager::logout() {
    m_identity    = PlayerIdentity();  // reset to default Offline
    m_accessToken  = "";
    m_refreshToken = "";

    ProfileManager::clearCache();
    saveSession();

    LOGI("[AccountManager] Logged out.\n");
}

// ─── Session persistence ─────────────────────────────────────────────────────
bool AccountManager::loadSession() {
    std::ifstream f(m_sessionFilePath);
    if (!f.is_open()) return false;

    std::string line;
    std::string typeStr;
    while (std::getline(f, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);
        if (key == "type")         typeStr       = val;
        if (key == "accessToken")  m_accessToken  = val;
        if (key == "refreshToken") m_refreshToken = val;
    }
    f.close();

    if (typeStr == "online" && !m_accessToken.empty()) {
        m_identity.accountType   = AccountType::ONLINE;
        m_identity.authenticated = true;
        return true;
    }

    m_identity.accountType   = AccountType::OFFLINE;
    m_identity.authenticated = false;
    return false;
}

bool AccountManager::saveSession() {
    std::ofstream f(m_sessionFilePath);
    if (!f.is_open()) return false;

    f << "type=" << (m_identity.accountType == AccountType::ONLINE ? "online" : "offline") << "\n";
    f << "accessToken="  << m_accessToken  << "\n";
    f << "refreshToken=" << m_refreshToken << "\n";

    f.close();
    return true;
}

bool AccountManager::refreshSession() {
    if (m_refreshToken.empty()) {
        return false;
    }

    std::string url = SUPABASE_URL + "/auth/v1/token?grant_type=refresh_token";
    std::string jsonBody = "{\"refresh_token\":\"" + m_refreshToken + "\"}";
    std::string extraHeaders = "apikey: " + SUPABASE_ANON_KEY + "\r\n";

    std::vector<unsigned char> outBody;
    LOGI("[AccountManager] Refreshing Supabase session with refresh_token...\n");

    bool ok = HttpClient::post(url, jsonBody, outBody, extraHeaders);
    std::string responseStr;
    if (!outBody.empty()) responseStr.assign(outBody.begin(), outBody.end());

    if (!ok || responseStr.empty()) {
        LOGW("[AccountManager] Token refresh network unreachable — retaining cached online session.\n");
        return false;
    }

    std::string newAccessToken  = jsonVal(responseStr, "access_token");
    std::string newRefreshToken = jsonVal(responseStr, "refresh_token");

    if (!newAccessToken.empty()) {
        m_accessToken = newAccessToken;
        if (!newRefreshToken.empty()) {
            m_refreshToken = newRefreshToken;
        }
        m_identity.accountType   = AccountType::ONLINE;
        m_identity.authenticated = true;
        LOGI("[AccountManager] Session successfully refreshed!\n");
        saveSession();
        ProfileManager::fetchAndCache(m_accessToken, m_identity);
        return true;
    }

    std::string err = jsonVal(responseStr, "error");
    if (err == "invalid_grant" || err == "invalid_request") {
        LOGW("[AccountManager] Session expired on server (%s). Re-auth required.\n", err.c_str());
        m_identity.authenticated = false;
    } else {
        LOGW("[AccountManager] Refresh token response: %s\n", responseStr.c_str());
    }

    return false;
}
