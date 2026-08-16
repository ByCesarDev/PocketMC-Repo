LOCAL_PATH := $(call my-dir)

# ============================================================================
# RakNet static library (built from project/lib_projects/raknet)
# ============================================================================
include $(CLEAR_VARS)
RPK_LOCAL_PATH := $(LOCAL_PATH)
include $(RPK_LOCAL_PATH)/../../lib_projects/raknet/jni/Android.mk
LOCAL_PATH := $(RPK_LOCAL_PATH)

# ============================================================================
# libpng static library (built from source downloaded by build.sh)
# ============================================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libpng
LOCAL_C_INCLUDES := $(LIBPNG_DIR)
LOCAL_CFLAGS := -O2 -DNDEBUG -DPNG_ARM_NEON_OPT=0
LOCAL_SRC_FILES := \
	$(LIBPNG_DIR)/png.c \
	$(LIBPNG_DIR)/pngerror.c \
	$(LIBPNG_DIR)/pngget.c \
	$(LIBPNG_DIR)/pngmem.c \
	$(LIBPNG_DIR)/pngpread.c \
	$(LIBPNG_DIR)/pngread.c \
	$(LIBPNG_DIR)/pngrio.c \
	$(LIBPNG_DIR)/pngrtran.c \
	$(LIBPNG_DIR)/pngrutil.c \
	$(LIBPNG_DIR)/pngset.c \
	$(LIBPNG_DIR)/pngtrans.c \
	$(LIBPNG_DIR)/pngwio.c \
	$(LIBPNG_DIR)/pngwrite.c \
	$(LIBPNG_DIR)/pngwtran.c \
	$(LIBPNG_DIR)/pngwutil.c
include $(BUILD_STATIC_LIBRARY)

# ============================================================================
# MinecraftPE shared library
# ============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := minecraftpe

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../../src \
	$(LOCAL_PATH)/../../lib_projects/raknet/jni/RaknetSources \
	$(LIBPNG_DIR)

# ---- Platform / App ----
LOCAL_SRC_FILES := \
	$(LOCAL_PATH)/../../../src/NinecraftApp.cpp \
	$(LOCAL_PATH)/../../../src/Performance.cpp \
	$(LOCAL_PATH)/../../../src/SharedConstants.cpp \
	$(LOCAL_PATH)/../../../src/main_android_java.cpp

# ---- Client core ----
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/../../../src/client/IConfigListener.cpp \
	$(LOCAL_PATH)/../../../src/client/Minecraft.cpp \
	$(LOCAL_PATH)/../../../src/client/MouseHandler.cpp \
	$(LOCAL_PATH)/../../../src/client/Option.cpp \
	$(LOCAL_PATH)/../../../src/client/OptionStrings.cpp \
	$(LOCAL_PATH)/../../../src/client/Options.cpp \
	$(LOCAL_PATH)/../../../src/client/OptionsFile.cpp

# ---- Gamemodes ----
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/../../../src/client/gamemode/CreativeMode.cpp \
	$(LOCAL_PATH)/../../../src/client/gamemode/GameMode.cpp \
	$(LOCAL_PATH)/../../../src/client/gamemode/SurvivalMode.cpp

# ---- Player ----
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/../../../src/client/player/LocalPlayer.cpp \
	$(LOCAL_PATH)/../../../src/client/player/RemotePlayer.cpp \
	$(LOCAL_PATH)/../../../src/client/player/input/KeyboardInput.cpp

# ---- Touch input ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/player/input/touchscreen/*.cpp)

# ---- GUI ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/gui/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/gui/components/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/gui/screens/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/gui/screens/crafting/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/gui/screens/touch/*.cpp)

# ---- Model ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/model/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/model/geom/*.cpp)

# ---- Particle ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/particle/*.cpp)

# ---- Renderer ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/renderer/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/renderer/culling/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/renderer/entity/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/renderer/ptexture/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/renderer/tileentity/*.cpp)

# ---- Sound ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/client/sound/*.cpp)

# ---- Locale ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/locale/*.cpp)

# ---- Mods ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/mods/*.cpp)

# ---- NBT ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/nbt/*.cpp)

# ---- Network ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/network/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/network/command/*.cpp)

# ---- Platform ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/platform/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/platform/input/*.cpp)

# ---- Server ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/server/*.cpp)

# ---- Util ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/util/*.cpp)

# ---- World ----
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/entity/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/entity/ai/control/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/entity/animal/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/entity/item/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/entity/monster/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/entity/player/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/entity/projectile/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/food/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/inventory/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/item/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/item/crafting/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/biome/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/chunk/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/dimension/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/levelgen/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/levelgen/feature/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/levelgen/synth/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/material/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/pathfinder/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/storage/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/tile/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/level/tile/entity/*.cpp)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../src/world/phys/*.cpp)

# Exclude OreFeature (same as CMake)
LOCAL_SRC_FILES := $(filter-out %/OreFeature.cpp, $(LOCAL_SRC_FILES))

# ---- Android audio backend ----
LOCAL_SRC_FILES += $(LOCAL_PATH)/../../../src/platform/audio/SoundSystemSL.cpp

# ---- Compiler flags ----
LOCAL_CFLAGS := -Wno-psabi
LOCAL_CPPFLAGS += -std=c++14 -frtti -fexceptions

# ---- Preprocessor defines ----
LOCAL_CPPFLAGS += -DANDROID -DOPENGL_ES -DNO_EGL -DPRE_ANDROID23

# ---- Static libraries ----
LOCAL_STATIC_LIBRARIES := libpng RakNet

# ---- System libraries (linked via LDLIBS, not SHARED_LIBRARIES) ----
LOCAL_LDLIBS := \
	-lEGL \
	-lGLESv1_CM \
	-landroid \
	-llog \
	-lOpenSLES \
	-lz

include $(BUILD_SHARED_LIBRARY)
