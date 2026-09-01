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

// Default: dev local. Overridden by init() or env var.
std::string baseUrl = "http://localhost:3000";

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
    // For HTTPS urls, delegate to HttpClient::download (handles WinHTTP/OpenSSL)
    // For HTTP (localhost API) we do a raw socket request with custom headers
    bool isHttps = (fullUrl.find("https://") == 0);

    if (isHttps) {
        // HttpClient::download doesn't support extra headers, so for HTTPS
        // we fall back to the raw socket GET path which only works for http.
        // TODO: Extend HttpClient for HTTPS GET+headers in the future.
        LOGW("[ApiClient] HTTPS GET with headers not yet supported — falling back.\n");
        std::vector<unsigned char> body;
        if (HttpClient::download(fullUrl, body) && !body.empty()) {
            outBody.assign(body.begin(), body.end());
            return true;
        }
        return false;
    }

    // Parse http://host:port/path
    std::string url = fullUrl;
    if (url.find("http://") == 0) url = url.substr(7);
    std::string host;
    int port = 80;
    std::string path = "/";

    size_t colonPos = url.find(':');
    size_t slashPos = url.find('/');

    if (colonPos != std::string::npos && (slashPos == std::string::npos || colonPos < slashPos)) {
        host = url.substr(0, colonPos);
        size_t end = (slashPos != std::string::npos) ? slashPos : url.size();
        port = atoi(url.substr(colonPos + 1, end - colonPos - 1).c_str());
        if (slashPos != std::string::npos) path = url.substr(slashPos);
    } else {
        if (slashPos != std::string::npos) {
            host = url.substr(0, slashPos);
            path = url.substr(slashPos);
        } else {
            host = url;
        }
    }

    int socketFd = -1;

#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)port);
    addr.sin_addr.s_addr = inet_addr(host == "localhost" ? "127.0.0.1" : host.c_str());
    socketFd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd != INVALID_SOCKET) {
        if (connect(socketFd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            closesocket(socketFd);
            socketFd = -1;
        }
    }
#else
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host == "localhost" ? "127.0.0.1" : host.c_str());
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd >= 0) {
        if (connect(socketFd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            close(socketFd);
            socketFd = -1;
        }
    }
#endif

    if (socketFd < 0) {
        LOGW("[ApiClient] rawGet: connect failed to %s:%d\n", host.c_str(), port);
        return false;
    }

    std::string req;
    req += "GET " + path + " HTTP/1.1\r\n";
    req += "Host: " + host + "\r\n";
    req += "User-Agent: MinecraftPE\r\n";
    req += "Connection: close\r\n";
    if (!extraHeaders.empty()) req += extraHeaders;
    req += "\r\n";

    send(socketFd, req.c_str(), (int)req.size(), 0);

    char buf[4096];
    std::vector<unsigned char> raw;
    int n;
    while ((n = recv(socketFd, buf, sizeof(buf), 0)) > 0) {
        raw.insert(raw.end(), buf, buf + n);
    }
#if defined(_WIN32)
    closesocket(socketFd);
#else
    close(socketFd);
#endif

    if (raw.empty()) return false;

    const std::string delim = "\r\n\r\n";
    auto it = std::search(raw.begin(), raw.end(), delim.begin(), delim.end());
    if (it == raw.end()) return false;

    size_t headerLen = it - raw.begin();
    std::string headers(reinterpret_cast<const char*>(raw.data()), headerLen);

    // Check 2xx
    size_t sp1 = headers.find(' ');
    if (sp1 == std::string::npos) return false;
    size_t sp2 = headers.find(' ', sp1 + 1);
    int status = atoi(headers.substr(sp1 + 1, sp2 - sp1 - 1).c_str());

    size_t bodyStart = headerLen + delim.size();
    outBody.assign(raw.begin() + bodyStart, raw.end());

    return (status >= 200 && status < 300);
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
