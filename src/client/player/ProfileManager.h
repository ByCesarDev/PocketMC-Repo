#ifndef NET_MINECRAFT_CLIENT_PLAYER__ProfileManager_H__
#define NET_MINECRAFT_CLIENT_PLAYER__ProfileManager_H__

// Fase 20 + 22 — ProfileManager
// Responsable de:
//   - datos del perfil online obtenidos desde la API
//   - cache local del perfil (para uso offline)
//   - actualizaciones de username

#include "PlayerIdentity.h"
#include <string>

class ProfileManager {
public:
    // Carga el perfil desde la API usando el accessToken dado.
    // Rellena `identity` con los datos recibidos y actualiza el cache local.
    // Retorna true si la API respondió correctamente.
    static bool fetchAndCache(const std::string& accessToken, PlayerIdentity& identity);

    // Carga el perfil desde el cache local (sin red).
    // Retorna true si existía un cache válido.
    static bool loadFromCache(PlayerIdentity& identity);

    // Actualiza el username en la API y en el cache local.
    static bool updateUsername(const std::string& accessToken, const std::string& newUsername, PlayerIdentity& identity);

    // Guarda los datos del identity en el cache local (data/profile_cache.txt)
    static bool saveCache(const PlayerIdentity& identity);

    // Borra el cache local (al hacer logout)
    static void clearCache();

    // Ruta del archivo de cache
    static std::string cacheFilePath;
};

#endif // NET_MINECRAFT_CLIENT_PLAYER__ProfileManager_H__
