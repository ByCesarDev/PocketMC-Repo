#include "CrashHandler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <deque>
#include <mutex>
#include <cstring>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <dbghelp.h>

    // Function pointer types for dynamic loading from dbghelp.dll
    typedef BOOL (WINAPI *pfn_MiniDumpWriteDump)(
        HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
        PMINIDUMP_EXCEPTION_INFORMATION,
        PMINIDUMP_USER_STREAM_INFORMATION,
        PMINIDUMP_CALLBACK_INFORMATION
    );
    typedef BOOL (WINAPI *pfn_SymInitialize)(HANDLE, PCSTR, BOOL);
    typedef BOOL (WINAPI *pfn_SymFromAddr)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);
#elif defined(__ANDROID__)
    #include <signal.h>
    #include <unistd.h>
    #include <unwind.h>
    #include <dlfcn.h>
    #include <cxxabi.h>
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
    HMODULE hDbgHelp = LoadLibraryA("dbghelp.dll");
    if (!hDbgHelp) return;

    pfn_MiniDumpWriteDump pMiniDump = (pfn_MiniDumpWriteDump)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if (!pMiniDump) {
        FreeLibrary(hDbgHelp);
        return;
    }

    HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = ep;
        mei.ClientPointers = FALSE;

        pMiniDump(
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

    FreeLibrary(hDbgHelp);
}
#elif defined(__ANDROID__)
struct AndroidBacktraceState {
    void** current;
    void** end;
};

static _Unwind_Reason_Code AndroidUnwindCallback(struct _Unwind_Context* context, void* arg) {
    AndroidBacktraceState* state = static_cast<AndroidBacktraceState*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

static size_t CaptureAndroidBacktrace(void** buffer, size_t maxFrames) {
    AndroidBacktraceState state = {buffer, buffer + maxFrames};
    _Unwind_Backtrace(AndroidUnwindCallback, &state);
    return state.current - buffer;
}
#endif

void CrashHandler::WriteCrashReport(const char* crashType, void* contextRecord) {
    std::string timestamp = GetTimestampString();
    std::string fileName = "crash_" + timestamp + ".log";
    std::ofstream out(fileName.c_str());

    if (!out.is_open()) return;

    out << "========================================================\n";
    out << "                PocketMC CRASH REPORT                  \n";
    out << "========================================================\n\n";
    out << "Timestamp:    " << timestamp << "\n";
    out << "Crash Reason: " << crashType << "\n";

    // 1. OS & Thread Info
#if defined(_WIN32)
    out << "Platform:     Windows\n";
    out << "Thread ID:    " << GetCurrentThreadId() << "\n";
#elif defined(__ANDROID__)
    out << "Platform:     Android (arm64-v8a / Bionic)\n";
    out << "Thread ID:    " << (uint64_t)pthread_self() << "\n";
#else
    out << "Platform:     Linux (POSIX)\n";
    out << "Thread ID:    " << (uint64_t)pthread_self() << "\n";
#endif

    // 2. Engine State Snapshot
    out << "\n[Game Engine State]\n";
    if (g_ContextCallback) {
        try {
            CrashContextSnapshot ctx = g_ContextCallback();
            out << " - Active Screen:   " << ctx.activeScreen << "\n";
            out << " - Player Position: " << ctx.playerPos << "\n";
            out << " - Current Chunk:   " << ctx.currentChunk << "\n";
            out << " - Level Name:      " << ctx.levelName << "\n";
            out << " - Tick Number:     " << ctx.gameTick << "\n";
            out << " - Average FPS:     " << ctx.fps << "\n";
        } catch (...) {
            out << " ! Failed to retrieve engine context\n";
        }
    } else {
        out << " - No engine callback registered.\n";
    }

    // 3. Breadcrumbs
    out << "\n[Recent Action Breadcrumbs (Last " << MAX_BREADCRUMBS << ")]\n";
    for (const auto& bc : g_Breadcrumbs) {
        out << " > " << bc << "\n";
    }

    // 4. Call Stack Unwinding
    out << "\n[Stack Trace]\n";

#if defined(_WIN32)
    void* stack[64];
    USHORT frames = CaptureStackBackTrace(0, 64, stack, nullptr);
    HANDLE process = GetCurrentProcess();

    HMODULE hDbgHelp = LoadLibraryA("dbghelp.dll");
    pfn_SymInitialize pSymInit = nullptr;
    pfn_SymFromAddr pSymAddr = nullptr;

    if (hDbgHelp) {
        pSymInit = (pfn_SymInitialize)GetProcAddress(hDbgHelp, "SymInitialize");
        pSymAddr = (pfn_SymFromAddr)GetProcAddress(hDbgHelp, "SymFromAddr");
        if (pSymInit) {
            pSymInit(process, nullptr, TRUE);
        }
    }

    for (USHORT i = 0; i < frames; ++i) {
        DWORD64 address = (DWORD64)(stack[i]);
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (pSymAddr && pSymAddr(process, address, &displacement, symbol)) {
            out << "  [" << i << "] " << symbol->Name << " + 0x" << std::hex << displacement << std::dec << "\n";
        } else {
            out << "  [" << i << "] 0x" << std::hex << address << std::dec << "\n";
        }
    }

    if (hDbgHelp) {
        FreeLibrary(hDbgHelp);
    }

    std::string dumpFile = "crash_" + timestamp + ".dmp";
    WriteMiniDump((EXCEPTION_POINTERS*)contextRecord, dumpFile);
    out << "\n[Minidump]\n -> Written to: " << dumpFile << "\n";

#elif defined(__ANDROID__)
    const size_t maxFrames = 64;
    void* buffer[maxFrames];
    size_t count = CaptureAndroidBacktrace(buffer, maxFrames);

    for (size_t i = 0; i < count; ++i) {
        const void* addr = buffer[i];
        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname) {
            int status = 0;
            char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
            const char* name = (status == 0 && demangled) ? demangled : info.dli_sname;

            out << "  [" << i << "] " << (info.dli_fname ? info.dli_fname : "unknown")
                << ": " << name << " + " << ((uintptr_t)addr - (uintptr_t)info.dli_saddr) << "\n";

            if (demangled) free(demangled);
        } else {
            out << "  [" << i << "] " << addr << "\n";
        }
    }

#else
    void* stack[64];
    int size = backtrace(stack, 64);
    char** symbols = backtrace_symbols(stack, size);
    if (symbols) {
        for (int i = 0; i < size; ++i) {
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
