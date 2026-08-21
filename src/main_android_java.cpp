#include "App.h"
#include "AppPlatform_android.h"

// JNI keycode constants
#include <android/keycodes.h>

#include "platform/input/Multitouch.h"
#include <unistd.h>
#include <pthread.h>

// Horrible, I know. / A
#ifndef MAIN_CLASS
#include "main.cpp"
#endif
// Note: android_java does NOT use android_native_app_glue (NativeActivity)
// #include "main_android.cpp"

// Include header AFTER MAIN_CLASS is defined
#include "main_android_java.h"


// References for JNI
static jobject g_pActivity  = 0;
static JavaVM* g_jvm        = 0;
static AppPlatform_android appPlatform;

static void setupExternalPath(JNIEnv* env, MAIN_CLASS* app)
{
    LOGI("setupExternalPath");

    if (env)
    {
        LOGI("Environment exists");
    }
    
    // Use external storage root directory directly (/storage/emulated/0) instead of Android/data
    jclass clazz = env->FindClass("android/os/Environment");
    jmethodID method = env->GetStaticMethodID(clazz, "getExternalStorageDirectory", "()Ljava/io/File;");
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    jobject file = env->CallStaticObjectMethod(clazz, method);

    if (!file) {
        LOGI("Failed to get external storage file object, using current working dir");
        app->externalStoragePath = ".";
        app->externalCacheStoragePath = ".";
        return;
    }

    jclass fileClass = env->GetObjectClass(file);
    jmethodID fileMethod = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jobject pathString = env->CallObjectMethod(file, fileMethod);

    const char* str = env->GetStringUTFChars((jstring) pathString, NULL);
    app->externalStoragePath = str;
    app->externalCacheStoragePath = str;
    LOGI("%s", str);

    // same fix as the native entry point: make sure cwd is writable
    if (chdir(str) != 0) {
        LOGI("chdir to %s failed: %s", str, strerror(errno));
    }

    env->ReleaseStringUTFChars((jstring)pathString, str);
}

static void pointerDown(int pointerId, int x, int y) {
    Multitouch::feed(1, 1, x, y, pointerId);
}
static void pointerUp(int pointerId, int x, int y) {
    Multitouch::feed(1, 0, x, y, pointerId);
}
static void pointerMove(int pointerId, int x, int y) {
    Multitouch::feed(0, 0, x, y, pointerId);
}


static App* gApp = 0;
static AppContext gContext;
static bool g_inNativeOnCreate = false;

extern "C" {
JNIEXPORT jint JNICALL
JNI_OnLoad( JavaVM * vm, void * reserved )
{
    LOGI("Entering OnLoad\n");
    g_jvm = vm;
    return appPlatform.init(vm);
}

// Register/save a reference to the java main activity instance
JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeRegisterThis(JNIEnv* env, jobject clazz) {
    LOGI("@RegisterThis\n");
    g_pActivity = (jobject)env->NewGlobalRef( clazz );
}

// Unregister/delete the reference to the java main activity instance
JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeUnregisterThis(JNIEnv* env, jobject clazz) {
    LOGI("@UnregisterThis\n");
    env->DeleteGlobalRef( g_pActivity );
}

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeOnCreate(JNIEnv* env, jobject thiz, jint screenWidth, jint screenHeight) {
    LOGI("@nativeOnCreate w=%d h=%d\n", (int)screenWidth, (int)screenHeight);
    g_inNativeOnCreate = true;

    appPlatform.instance = g_pActivity;
    appPlatform.setScreenDimensions((int)screenWidth, (int)screenHeight);
    LOGI("nativeOnCreate: screen set, no initConsts needed\n");
    gContext.doRender = false;
    gContext.platform = &appPlatform;

    LOGI("nativeOnCreate: creating gApp\n");
    gApp = new MAIN_CLASS();
    LOGI("nativeOnCreate: gApp=%p\n", gApp);
    setupExternalPath(env, (MAIN_CLASS*)gApp);
    if (env->ExceptionOccurred()) {
        LOGI("nativeOnCreate: exception after setupExternalPath!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    LOGI("nativeOnCreate: done\n");
    g_inNativeOnCreate = false;
    //gApp->init(gContext);
}

void pickImage_JNI() {
    if (!g_pActivity || !g_jvm) return;
    JVMAttacher ta(g_jvm);
    JNIEnv* env = ta.getEnv();
    jclass cls = env->GetObjectClass(g_pActivity);
    jmethodID mid = env->GetMethodID(cls, "pickImage", "()V");
    if (mid) {
        env->CallVoidMethod(g_pActivity, mid);
    }
}

void extractAsset_JNI(const char* asset, const char* dest) {
    if (!g_pActivity || !g_jvm) return;
    JVMAttacher ta(g_jvm);
    JNIEnv* env = ta.getEnv();
    jclass cls = env->GetObjectClass(g_pActivity);
    jmethodID mid = env->GetMethodID(cls, "extractAsset", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (mid) {
        env->CallVoidMethod(g_pActivity, mid, env->NewStringUTF(asset), env->NewStringUTF(dest));
    }
}

static int s_surfaceCreatedCount = 0;

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_GLRenderer_nativeOnSurfaceCreated(JNIEnv* env, jclass cls);

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeOnSurfaceCreated(JNIEnv* env, jclass cls) {
    Java_net_bycesardev_pocketmc_GLRenderer_nativeOnSurfaceCreated(env, cls);
}

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_GLRenderer_nativeOnSurfaceCreated(JNIEnv* env, jclass cls) {
    s_surfaceCreatedCount++;
    if (g_inNativeOnCreate) {
        // Skip re-entrant surface callbacks that fire during nativeOnCreate
        return;
    }
    LOGI("@nativeOnSurfaceCreated #%d tid=%d\n", s_surfaceCreatedCount, (int)gettid());

     if (gApp) {
         // Don't call onGraphicsReset the first time
        if (gApp->isInited()) {
            LOGI("nativeOnSurfaceCreated: calling onGraphicsReset\n");
            gApp->onGraphicsReset(gContext);
        }

        if (!gApp->isInited()) {
            LOGI("nativeOnSurfaceCreated: calling init\n");
            gApp->init(gContext);
            LOGI("nativeOnSurfaceCreated: init done, isInited=%d\n", (int)gApp->isInited());
        }
     }
}

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_GLRenderer_nativeOnSurfaceChanged(JNIEnv* env, jclass cls, jint w, jint h);

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeOnSurfaceChanged(JNIEnv* env, jclass cls, jint w, jint h) {
    Java_net_bycesardev_pocketmc_GLRenderer_nativeOnSurfaceChanged(env, cls, w, h);
}

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_GLRenderer_nativeOnSurfaceChanged(JNIEnv* env, jclass cls, jint w, jint h) {
    LOGI("@nativeOnSurfaceChanged: %lu\n", (unsigned long)pthread_self());

    if (gApp) {
        gApp->setSize(w, h);

        if (!gApp->isInited())
            gApp->init(gContext);

        if (!gApp->isInited())
            LOGI("nativeOnSurfaceChanged: NOT INITED!\n");
    }
}

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_GLRenderer_nativeUpdate(JNIEnv* env, jclass cls);

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeUpdate(JNIEnv* env, jclass cls) {
    Java_net_bycesardev_pocketmc_GLRenderer_nativeUpdate(env, cls);
}

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_GLRenderer_nativeUpdate(JNIEnv* env, jclass cls) {
    //LOGI("@nativeUpdate: %p\n", pthread_self());
    if (gApp) {
        if (!gApp->isInited())
            gApp->init(gContext);

        gApp->update();

        if (gApp->wantToQuit())
            appPlatform.finish();
    }
}

//
// Keyboard events
//
// helper to convert Android keycodes to our internal Keyboard constants
static int androidKeyToInternal(int androidKey) {
    if (androidKey >= 29 && androidKey <= 54) {
        return androidKey - 29 + 'A'; // Maps AKEYCODE_A (29) .. AKEYCODE_Z (54) to 'A' (65) .. 'Z' (90)
    }
    if (androidKey >= 7 && androidKey <= 16) {
        return androidKey - 7 + '0'; // Maps AKEYCODE_0 (7) .. AKEYCODE_9 (16) to '0' (48) .. '9' (57)
    }
    switch(androidKey) {
        case 67: return Keyboard::KEY_BACKSPACE; // AKEYCODE_DEL
        case 66: return Keyboard::KEY_RETURN;    // AKEYCODE_ENTER
        case 62: return Keyboard::KEY_SPACE;     // AKEYCODE_SPACE
        case 61: return Keyboard::KEY_TAB;       // AKEYCODE_TAB
        case 111: return Keyboard::KEY_ESCAPE;   // AKEYCODE_ESCAPE
        case 59:
        case 60: return Keyboard::KEY_LSHIFT;    // AKEYCODE_SHIFT_LEFT / RIGHT
        case 113:
        case 114: return Keyboard::KEY_LEFT_CTRL; // AKEYCODE_CTRL_LEFT / RIGHT
        default:
            return androidKey; // fall back to raw code
    }
}

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeOnDestroy(JNIEnv* env) {
    LOGI("@nativeOnDestroy\n");

    delete gApp;
    gApp = 0;
    //gApp->onGraphicsReset(gContext);
}

JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeOnKeyDown(JNIEnv* env, jclass cls, jint keyCode) {
    LOGI("@nativeOnKeyDown: %d\n", keyCode);
    int mapped = androidKeyToInternal(keyCode);
    Keyboard::feed(mapped, true);
}
JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeTextChar(JNIEnv* env, jclass cls, jint unicodeChar) {
    // soft-keyboards may send a backspace as a character code
    if (unicodeChar == 8) {
        Keyboard::feed(Keyboard::KEY_BACKSPACE, true);
        Keyboard::feed(Keyboard::KEY_BACKSPACE, false);
    } else if (unicodeChar > 0 && unicodeChar < 128) {
        Keyboard::feedText((char)unicodeChar);
    }
}
JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeOnKeyUp(JNIEnv* env, jclass cls, jint keyCode) {
    LOGI("@nativeOnKeyUp: %d\n", (int)keyCode);
    int mapped = androidKeyToInternal(keyCode);
    Keyboard::feed(mapped, false);
}

JNIEXPORT jboolean JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeHandleBack(JNIEnv* env, jclass cls, jboolean isDown) {
    LOGI("@nativeHandleBack: %d\n", isDown);
    if (gApp) return gApp->handleBack(isDown)? JNI_TRUE : JNI_FALSE;
    return JNI_FALSE;
}

//
// Mouse events
//
JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeMouseDown(JNIEnv* env, jclass cls, jint pointerId, jint buttonId, jfloat x, jfloat y) {
    //LOGI("@nativeMouseDown: %f %f\n", x, y);
    mouseDown(1, x, y);
    pointerDown(pointerId, x, y);
}
JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeMouseUp(JNIEnv* env, jclass cls, jint pointerId, jint buttonId, jfloat x, jfloat y) {
    //LOGI("@nativeMouseUp: %f %f\n", x, y);
    mouseUp(1, x, y);
    pointerUp(pointerId, x, y);
}
JNIEXPORT void JNICALL
Java_net_bycesardev_pocketmc_MainActivity_nativeMouseMove(JNIEnv* env, jclass cls, jint pointerId, jfloat x, jfloat y) {
    //LOGI("@nativeMouseMove: %f %f\n", x, y);
    mouseMove(x, y);
    pointerMove(pointerId, x, y);
}
}
