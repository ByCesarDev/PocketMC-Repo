#ifndef MAIN_GLFW_H__
#define MAIN_GLFW_H__

#include <csignal>
#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

// A flag to ensure we only write the log once if multiple signals/exceptions fire
static bool g_has_crashed = false;

inline void write_crash_log(const std::string& reason) {
    if (g_has_crashed) return;
    g_has_crashed = true;

    std::ofstream log("crash_log.txt");
    if (log.is_open()) {
        log << "========================================\n";
        log << "EVLYMC CRASH LOG\n";
        log << "========================================\n";
        log << "Reason: " << reason << "\n";
        log.close();
    }
    std::cerr << "CRASH! " << reason << "\nSaved crash log to crash_log.txt\n";
}

inline void signal_handler(int sig) {
    std::string reason;
    switch (sig) {
        case SIGSEGV: reason = "SIGSEGV (Segmentation Fault / Access Violation)"; break;
        case SIGABRT: reason = "SIGABRT (Abort / assertion failed)"; break;
        case SIGFPE:  reason = "SIGFPE (Floating Point Exception)"; break;
        case SIGILL:  reason = "SIGILL (Illegal Instruction)"; break;
        default:      reason = "Signal " + std::to_string(sig); break;
    }
    write_crash_log(reason);
    std::exit(sig);
}

inline void custom_terminate_handler() {
    std::string reason = "Unhandled C++ exception";
    try {
        auto ex = std::current_exception();
        if (ex) {
            std::rethrow_exception(ex);
        }
    } catch (const std::exception& e) {
        reason += ": " + std::string(e.what());
    } catch (...) {
        reason += ": Unknown type";
    }
    write_crash_log(reason);
    std::abort();
}

#ifdef _WIN32
inline LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS* info) {
    std::string reason = "Windows Exception Code: 0x";
    char hex_code[32];
    std::sprintf(hex_code, "%lX", info->ExceptionRecord->ExceptionCode);
    reason += hex_code;
    
    char hex_addr[32];
    std::sprintf(hex_addr, "%p", info->ExceptionRecord->ExceptionAddress);
    reason += " at address " + std::string(hex_addr);

    switch (info->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            reason += " (Access Violation / Null pointer dereference)";
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            reason += " (Array Bounds Exceeded)";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            reason += " (Integer Divide by Zero)";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            reason += " (Stack Overflow)";
            break;
    }

    // Capture and print stack trace
    reason += "\nStack trace:\n";
    void* stack[62];
    unsigned short frames = CaptureStackBackTrace(0, 62, stack, NULL);
    for (unsigned short i = 0; i < frames; ++i) {
        char frame_info[64];
        std::sprintf(frame_info, "  #%d %p\n", i, stack[i]);
        reason += frame_info;
    }

    write_crash_log(reason);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#include "App.h"
#include "client/renderer/entity/PlayerRenderer.h"
#include "client/renderer/gles.h"
#include "GLFW/glfw3.h"

#include <cstdio>
#include <chrono>
#include <thread>
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"
#include "AppPlatform_glfw.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
static App* g_app = 0;

int transformKey(int glfwkey) {
	if (glfwkey >= GLFW_KEY_F1 && glfwkey <= GLFW_KEY_F12) {
		return glfwkey - 178;
	}

	switch (glfwkey) {
		case GLFW_KEY_ESCAPE: return Keyboard::KEY_ESCAPE;
		case GLFW_KEY_TAB: return Keyboard::KEY_TAB;
		case GLFW_KEY_BACKSPACE: return Keyboard::KEY_BACKSPACE;
		case GLFW_KEY_LEFT_SHIFT: return Keyboard::KEY_LSHIFT;
		case GLFW_KEY_ENTER: return Keyboard::KEY_RETURN;
		case GLFW_KEY_LEFT_CONTROL: return Keyboard::KEY_LEFT_CTRL;
		default: return glfwkey;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(action == GLFW_REPEAT) return;

	if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
		GLFWmonitor* monitor = glfwGetWindowMonitor(window);
		if (monitor) {
			// Currently fullscreen → go windowed
			glfwSetWindowMonitor(window, NULL, 80, 80, 854, 480, 0);
		} else {
			// Currently windowed → go fullscreen on primary monitor
			GLFWmonitor* primary = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(primary);
			glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		return;
	}

	Keyboard::feed(transformKey(key), action);
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
	Keyboard::feedText(codepoint);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	static double lastX = 0.0, lastY = 0.0;
	static bool firstMouse = true;

	if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

	double deltaX = xpos - lastX;
    double deltaY = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
		Mouse::feed(0, 0, xpos, ypos, deltaX, deltaY);
	} else { 
		Mouse::feed( MouseAction::ACTION_MOVE, 0, xpos, ypos);
	}
	Multitouch::feed(0, 0, xpos, ypos, 0);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if(action == GLFW_REPEAT) return;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		Mouse::feed( MouseAction::ACTION_LEFT, action, xpos, ypos);
		Multitouch::feed(1, action, xpos, ypos, 0);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		Mouse::feed( MouseAction::ACTION_RIGHT, action, xpos, ypos);
	}

	if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		Mouse::feed( MouseAction::ACTION_MIDDLE, action, xpos, ypos);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	Mouse::feed(3, 0, xpos, ypos, 0, yoffset);
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	if (g_app) g_app->setSize(width, height);
}

void error_callback(int error, const char* desc) {
	printf("Error: %s\n", desc);
}


void loop() {
	using clock = std::chrono::steady_clock;
	auto frameStart = clock::now();

	g_app->update();

	glfwSwapBuffers(((AppPlatform_glfw*)g_app->platform())->window);
	glfwPollEvents();

	glfwSwapInterval(((MAIN_CLASS*)g_app)->options.getBooleanValue(OPTIONS_VSYNC) ? 1 : 0);
	if(((MAIN_CLASS*)g_app)->options.getBooleanValue(OPTIONS_LIMIT_FRAMERATE)) {
		auto frameEnd = clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);
		auto target = std::chrono::microseconds(33333); // ~30 fps
		if(elapsed < target)
			std::this_thread::sleep_for(target - elapsed);
	}
}

int main(void) {
    // Register crash handlers
#ifndef _WIN32
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGFPE, signal_handler);
    std::signal(SIGILL, signal_handler);
#endif
    std::signal(SIGABRT, signal_handler);
    std::set_terminate(custom_terminate_handler);
#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif

	AppContext appContext;

#ifndef STANDALONE_SERVER
	// Platform init.
	appContext.platform = new AppPlatform_glfw();
#if defined(__EMSCRIPTEN__)
	EM_ASM(
		FS.mkdir('/games');
		FS.mkdir('/games/com.mojang');
        FS.mkdir('/games/com.mojang/minecraftWorlds');
        FS.mount(IDBFS, {}, '/games');
        FS.syncfs(true, function (err) {});
    );
#endif

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
#ifndef __EMSCRIPTEN__
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#else
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	AppPlatform_glfw* platform = (AppPlatform_glfw*)appContext.platform;

	platform->window = glfwCreateWindow(appContext.platform->getScreenWidth(), appContext.platform->getScreenHeight(), "Minecraft PE 0.6.1", NULL, NULL);
	
	if (platform->window == NULL) {
		return 1;
	}

	glfwSetKeyCallback(platform->window, key_callback);
	glfwSetCharCallback(platform->window, character_callback);
	glfwSetCursorPosCallback(platform->window, cursor_position_callback);
	glfwSetMouseButtonCallback(platform->window, mouse_button_callback);
	glfwSetScrollCallback(platform->window, scroll_callback);
	glfwSetWindowSizeCallback(platform->window, window_size_callback);

	glfwMakeContextCurrent(platform->window);
	#ifndef __EMSCRIPTEN__
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(0);
	#endif
#endif

	App* app = new MAIN_CLASS();

	g_app = app;
	((MAIN_CLASS*)g_app)->externalStoragePath = ".";
	((MAIN_CLASS*)g_app)->externalCacheStoragePath = ".";
	g_app->init(appContext);
	g_app->setSize(appContext.platform->getScreenWidth(), appContext.platform->getScreenHeight());

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(loop, 0, 1);
#else
	// Main event loop
	while(!glfwWindowShouldClose(platform->window) && !app->wantToQuit()) {
		loop();
	}
#endif

	delete app;

#ifndef STANDALONE_SERVER
	// Exit.
	glfwDestroyWindow(platform->window);
	glfwTerminate();
#endif

	appContext.platform->finish();
	
	delete appContext.platform;

	return 0;
}

#endif /*MAIN_GLFW_H__*/
