#include "ApiClient.h"
#include "../../platform/HttpClient.h"
#include "../../platform/log.h"
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

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

namespace ApiClient {

// Default: Render production API. Overridden by init() or PMC_API_URL env var.
std::string baseUrl = "https://pocketmc-api.onrender.com";

void init(const std::string& url) {
    if (!url.empty()) {
        baseUrl = url;
        // Strip trailing slash
        if (!baseUrl.empty() && baseUrl.back() == '/') {
            baseUrl.pop_back();
        }
    }
    LOGI("[ApiClient] Base URL: %s\n", baseUrl.c_str());
}

// ─── Internal helper: raw GET request with custom headers ───────────────────

static bool rawGet(const std::string& fullUrl, const std::string& extraHeaders, std::string& outBody) {
    std::vector<unsigned char> body;
    if (HttpClient::get(fullUrl, body, extraHeaders)) {
        outBody.assign(body.begin(), body.end());
        return true;
    }
    return false;
}

// ─── JSON value extractor (simple, no full parser needed) ───────────────────

static std::string jsonVal(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    size_t colon = json.find(':', pos);
    if (colon == std::string::npos) return "";
    // Skip whitespace after colon
    size_t vStart = colon + 1;
    while (vStart < json.size() && (json[vStart] == ' ' || json[vStart] == '\t')) vStart++;
    if (vStart >= json.size()) return "";
    if (json[vStart] == '"') {
        size_t end = json.find('"', vStart + 1);
        if (end == std::string::npos) return "";
        return json.substr(vStart + 1, end - vStart - 1);
    } else {
        // Non-string (number / bool / null) — read until delimiter
        size_t end = vStart;
        while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != ']') end++;
        std::string val = json.substr(vStart, end - vStart);
        // trim
        while (!val.empty() && (val.back() == ' ' || val.back() == '\n' || val.back() == '\r')) val.pop_back();
        return val;
    }
}

// ─── Public API ─────────────────────────────────────────────────────────────

bool checkHealth() {
    std::vector<unsigned char> body;
    if (!HttpClient::download(baseUrl + "/api/v1/health", body) || body.empty()) return false;
    std::string json(body.begin(), body.end());
    return json.find("\"ok\"") != std::string::npos || json.find("ok") != std::string::npos;
}

bool getMe(const std::string& accessToken, std::string& outJson) {
    std::string authHeader = "Authorization: Bearer " + accessToken + "\r\n";
    return rawGet(baseUrl + "/api/v1/me", authHeader, outJson);
}

bool updateMe(const std::string& accessToken, const std::string& newUsername, std::string& outJson) {
    std::string url = baseUrl + "/api/v1/me";
    std::string jsonBody = "{\"username\":\"" + newUsername + "\"}";
    std::string extraHeaders = "Authorization: Bearer " + accessToken + "\r\n";

    std::vector<unsigned char> outBody;
    bool ok = HttpClient::post(url, jsonBody, outBody, extraHeaders);
    if (!outBody.empty()) outJson.assign(outBody.begin(), outBody.end());
    return ok;
}

bool checkUsernameAvailable(const std::string& username) {
    std::string outJson;
    bool ok = rawGet(baseUrl + "/api/v1/usernames/" + username + "/available", "", outJson);
    if (!ok || outJson.empty()) return false;
    // {"username":"...","available":true}
    std::string val = jsonVal(outJson, "available");
    return val == "true";
}

bool getPublicProfile(const std::string& username, std::string& outJson) {
    return rawGet(baseUrl + "/api/v1/users/" + username, "", outJson);
}

} // namespace ApiClient
