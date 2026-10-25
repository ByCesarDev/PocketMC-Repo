#pragma once
#include <string>
#include <vector>
#include <functional>

namespace PocketMC {

// Functor to query engine state safely during a crash
struct CrashContextSnapshot {
    std::string activeScreen = "None";
    std::string playerPos    = "N/A";
    std::string currentChunk = "N/A";
    uint64_t    gameTick     = 0;
    uint32_t    fps          = 0;
    std::string levelName    = "None";
};

class CrashHandler {
public:
    static void Initialize();
    static void Shutdown();

    // Register a hook into the Minecraft instance to extract game state
    static void RegisterContextCallback(std::function<CrashContextSnapshot()> callback);

    // Breadcrumb trail to track what happened leading up to the crash
    static void AddBreadcrumb(const std::string& message);

    // Write manual crash log (e.g. fatal assertion)
    static void LogFatalError(const std::string& message);

private:
    static void WriteCrashReport(const char* crashType, void* contextRecord);
};

} // namespace PocketMC