#pragma once
#include <string>
#include <vector>
#include <functional>

namespace PocketMC {

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

    static void RegisterContextCallback(std::function<CrashContextSnapshot()> callback);
    static void AddBreadcrumb(const std::string& message);
    static void LogFatalError(const std::string& message);

    // Moved to public so OS signal/exception handlers can call it
    static void WriteCrashReport(const char* crashType, void* contextRecord);
};

} // namespace PocketMC
