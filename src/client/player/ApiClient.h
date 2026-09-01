#ifndef NET_MINECRAFT_CLIENT_PLAYER__ApiClient_H__
#define NET_MINECRAFT_CLIENT_PLAYER__ApiClient_H__

// Fase 22 — ApiClient separado
// Centraliza toda la comunicación con la PocketMC API REST.
// Ninguna otra clase debe construir requests HTTP directamente.

#include <string>
#include <vector>

namespace ApiClient {

    // URL base de la API — se configura en init()
    // Dev:  http://localhost:3000
    // Prod: https://api-pocketmc.onrender.com
    extern std::string baseUrl;

    // Inicializa el cliente con la URL base.
    // Llamar desde NinecraftApp::init() antes de cualquier request.
    void init(const std::string& url = "");

    // GET /api/v1/health — verifica que la API esté online
    // Retorna true si status == "ok"
    bool checkHealth();

    // GET /api/v1/me — requiere Bearer token
    // Devuelve JSON del perfil en outJson. Retorna true si 200 OK.
    bool getMe(const std::string& accessToken, std::string& outJson);

    // PATCH /api/v1/me — requiere Bearer token
    // Envía { "username": newUsername } y devuelve el JSON actualizado.
    bool updateMe(const std::string& accessToken, const std::string& newUsername, std::string& outJson);

    // GET /api/v1/usernames/:username/available
    // Retorna true si el username está disponible.
    bool checkUsernameAvailable(const std::string& username);

    // GET /api/v1/users/:username — perfil público
    bool getPublicProfile(const std::string& username, std::string& outJson);

} // namespace ApiClient

#endif // NET_MINECRAFT_CLIENT_PLAYER__ApiClient_H__
