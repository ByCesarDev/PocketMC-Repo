#include "CrashHandler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <deque>
#include <mutex>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <dbghelp.h>
    #pragma comment(lib, "DbgHelp.lib")
#else
    #include <signal.h>
    #include <execinfo.h>
    #include <unistd.h>
    #include <cxxabi.h>
#endif

namespace PocketMC {

static std::function<CrashContextSnapshot()> g_ContextCallback = nullptr;
static std::deque<std::string> g_Breadcrumbs;
static std::mutex g_BreadcrumbMutex;
static const size_t MAX_BREADCRUMBS = 20;

void CrashHandler::RegisterContextCallback(std::function<CrashContextSnapshot()> callback) {
    g_ContextCallback = callback;
}

void CrashHandler::AddBreadcrumb(const std::string& message) {
    std::lock_guard<std::mutex> lock(g_BreadcrumbMutex);
    if (g_Breadcrumbs.size() >= MAX_BREADCRUMBS) {
        g_Breadcrumbs.pop_front();
    }
    g_Breadcrumbs.push_back(message);
}

static std::string GetTimestampString() {
    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", localtime(&now));
    return std::string(buf);
}

#if defined(_WIN32)
static void WriteMiniDump(EXCEPTION_POINTERS* ep, const std::string& dumpPath) {
    HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = ep;
        mei.ClientPointers = FALSE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MiniDumpWithIndirectlyReferencedMemory,
            &mei,
            nullptr,
            nullptr
        );
        CloseHandle(hFile);
    }
}
#endif

void CrashHandler::WriteCrashReport(const char* crashType, void* contextRecord) {
    std::string timestamp = GetTimestampString();
    std::string fileName = "crash_" + timestamp + ".log";
    std::ofstream out(fileName);

    if (!out.is_open()) return;

    out << "========================================================\n";
    out << "                PocketMC CRASH REPORT                  \n";
    out << "========================================================\n\n";
    out << "Timestamp:    " << timestamp << "\n";
    out << "Crash Reason: " << crashType << "\n";

    // 1. Thread & OS Info
#if defined(_WIN32)
    out << "Platform:     Windows (x86_64 / MSVC)\n";
    out << "Thread ID:    " << GetCurrentThreadId() << "\n";
#elif defined(__ANDROID__)
    out << "Platform:     Android (Native NDK)\n";
#else
    out << "Platform:     Linux (POSIX)\n";
    out << "Thread ID:    " << pthread_self() << "\n";
#endif

    // 2. Engine & World Context Snapshot
    out << "\n[Game Engine State]\n";
    if (g_ContextCallback) {
        // Run safely inside try-catch to avoid secondary crash
        try {
            CrashContextSnapshot ctx = g_ContextCallback();
            out << " - Active Screen:   " << ctx.activeScreen << "\n";
            out << " - Player Position: " << ctx.playerPos << "\n";
            out << " - Current Chunk:   " << ctx.currentChunk << "\n";
            out << " - Level Name:      " << ctx.levelName << "\n";
            out << " - Tick Number:     " << ctx.gameTick << "\n";
            out << " - Average FPS:     " << ctx.fps << "\n";
        } catch (...) {
            out << " ! Failed to retrieve engine context (corrupted memory state)\n";
        }
    } else {
        out << " - No engine callback registered.\n";
    }

    // 3. Breadcrumbs
    out << "\n[Recent Action Breadcrumbs (Last " << MAX_BREADCRUMBS << ")]\n";
    for (const auto& bc : g_Breadcrumbs) {
        out << " > " << bc << "\n";
    }

    // 4. Call Stack Extraction
    out << "\n[Stack Trace]\n";
#if defined(_WIN32)
    void* stack[64];
    USHORT frames = CaptureStackBackTrace(0, 64, stack, nullptr);
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);

    for (USHORT i = 0; i < frames; ++i) {
        DWORD64 address = (DWORD64)(stack[i]);
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (SymFromAddr(process, address, &displacement, symbol)) {
            out << "  [" << i << "] " << symbol->Name << " + 0x" << std::hex << displacement << std::dec << "\n";
        } else {
            out << "  [" << i << "] 0x" << std::hex << address << std::dec << " (Unknown Symbol)\n";
        }
    }

    // Write Minidump
    std::string dumpFile = "crash_" + timestamp + ".dmp";
    WriteMiniDump((EXCEPTION_POINTERS*)contextRecord, dumpFile);
    out << "\n[Minidump]\n -> Written to: " << dumpFile << "\n";
#else
    // POSIX backtrace
    void* stack[64];
    int size = backtrace(stack, 64);
    char** symbols = backtrace_symbols(stack, size);
    if (symbols) {
        for (int i = 0; i < size; ++i) {
            // Attempt C++ demangle
            std::string line = symbols[i];
            size_t openParen = line.find('(');
            size_t plus = line.find('+', openParen);
            if (openParen != std::string::npos && plus != std::string::npos) {
                std::string mangled = line.substr(openParen + 1, plus - openParen - 1);
                int status = 0;
                char* demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
                if (status == 0 && demangled) {
                    out << "  [" << i << "] " << line.substr(0, openParen + 1) << demangled << line.substr(plus) << "\n";
                    free(demangled);
                    continue;
                }
            }
            out << "  [" << i << "] " << line << "\n";
        }
        free(symbols);
    }
#endif

    out << "\n================== END OF REPORT ==================\n";
    out.flush();
    out.close();
}

#if defined(_WIN32)
static LONG WINAPI Win32ExceptionHandler(EXCEPTION_POINTERS* ep) {
    const char* reason = "Unknown Exception";
    switch (ep->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:         reason = "EXCEPTION_ACCESS_VIOLATION (Segmentation Fault / Nullptr)"; break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    reason = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       reason = "EXCEPTION_INT_DIVIDE_BY_ZERO"; break;
        case EXCEPTION_STACK_OVERFLOW:           reason = "EXCEPTION_STACK_OVERFLOW"; break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:      reason = "EXCEPTION_ILLEGAL_INSTRUCTION"; break;
    }
    CrashHandler::WriteCrashReport(reason, ep);
    return EXCEPTION_CONTINUE_SEARCH;
}
#else
static void PosixSignalHandler(int sig, siginfo_t* info, void* ucontext) {
    const char* reason = "Unknown Signal";
    switch (sig) {
        case SIGSEGV: reason = "SIGSEGV (Invalid Memory Reference)"; break;
        case SIGABRT: reason = "SIGABRT (Abort Signal)"; break;
        case SIGFPE:  reason = "SIGFPE (Floating Point Exception)"; break;
        case SIGILL:  reason = "SIGILL (Illegal Instruction)"; break;
        case SIGBUS:  reason = "SIGBUS (Bus Error / Misaligned Memory Access)"; break;
    }
    CrashHandler::WriteCrashReport(reason, ucontext);
    _exit(1);
}
#endif

void CrashHandler::Initialize() {
#if defined(_WIN32)
    SetUnhandledExceptionFilter(Win32ExceptionHandler);
#else
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = PosixSignalHandler;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
    sigaction(SIGILL,  &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
#endif
}

} // namespace PocketMC