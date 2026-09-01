#ifndef NET_MINECRAFT_CLIENT_PLAYER__PlayerIdentity_H__
#define NET_MINECRAFT_CLIENT_PLAYER__PlayerIdentity_H__

#include <string>

// Fase 21 — Separación interna de identidad
// El resto del juego debe consultar PlayerIdentity en lugar de
// acceder directamente a AccountManager o Supabase.

enum class AccountType {
    OFFLINE,
    ONLINE
};

struct PlayerIdentity {
    AccountType accountType = AccountType::OFFLINE;
    std::string playerId    = "";   // UUID de Supabase (online) o "" (offline)
    std::string username    = "";   // @CesarDev (online) o "Cesar" (offline local)
    std::string displayName = "";   // nombre visible / alias
    bool authenticated      = false;

    bool isOnline() const {
        return accountType == AccountType::ONLINE && authenticated;
    }

    std::string getAccountTypeString() const {
        return (accountType == AccountType::ONLINE) ? "Online" : "Offline";
    }

    // Construye una identidad offline básica desde el username local
    static PlayerIdentity makeOffline(const std::string& localUsername) {
        PlayerIdentity id;
        id.accountType  = AccountType::OFFLINE;
        id.username     = localUsername;
        id.displayName  = localUsername;
        id.authenticated = false;
        return id;
    }
};

#endif // NET_MINECRAFT_CLIENT_PLAYER__PlayerIdentity_H__
