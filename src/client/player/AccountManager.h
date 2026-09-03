#ifndef NET_MINECRAFT_CLIENT_PLAYER__AccountManager_H__
#define NET_MINECRAFT_CLIENT_PLAYER__AccountManager_H__

// Fase 21/22 — AccountManager refactorizado
// Responsable de: login, logout, sesión, refresh token, estado Offline/Online.
// Delega recuperación de perfil a ProfileManager.
// Delega requests HTTP a ApiClient.

#include "PlayerIdentity.h"
#include <string>

class AccountManager {
private:
    static PlayerIdentity m_identity;
    static std::string    m_accessToken;
    static std::string    m_refreshToken;
    static std::string    m_sessionFilePath;

public:
    // Inicializa: carga sesión desde disco y restaura perfil desde cache.
    static void init(const std::string& dataDir = "");

    // ── Getters ────────────────────────────────────────────────────────────
    static const PlayerIdentity& getIdentity();
    static bool isOnline();
    static std::string getUsername(const std::string& fallbackLocalName = "");
    static std::string getDisplayName(const std::string& fallbackLocalName = "");
    static std::string getPlayerId();
    static std::string getAccessToken();
    static std::string getRefreshToken();
    static std::string getAccountTypeString();

    // ── Auth ────────────────────────────────────────────────────────────────
    // Autentica con Supabase y llama a ProfileManager::fetchAndCache.
    static bool loginWithSupabase(const std::string& email,
                                  const std::string& password,
                                  std::string& errorMsgOut);

    // Borra sesión y cache — vuelve a Offline.
    static void logout();

    // ── Session persistence ────────────────────────────────────────────────
    static bool loadSession();
    static bool saveSession();
    static bool refreshSession();
};

#endif // NET_MINECRAFT_CLIENT_PLAYER__AccountManager_H__
