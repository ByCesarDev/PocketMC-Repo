#ifndef NET_MINECRAFT_CLIENT__Options_H__
#define NET_MINECRAFT_CLIENT__Options_H__

#define SOUND_MIN_VALUE 0.0f
#define SOUND_MAX_VALUE 1.0f
#define MUSIC_MIN_VALUE 0.0f
#define MUSIC_MAX_VALUE 1.0f
#define SENSITIVITY_MIN_VALUE 0.0f
#define SENSITIVITY_MAX_VALUE 1.0f
#define PIXELS_PER_MILLIMETER_MIN_VALUE 3.0f
#define PIXELS_PER_MILLIMETER_MAX_VALUE 4.0f

//package net.minecraft.client;

//#include "locale/Language.h"

#include <string>
#include <cstdio>
#include "../platform/input/Keyboard.h"
#include "../util/StringUtils.h"
#include "OptionsFile.h"
#include "Option.h"
#include <array>

enum OptionId {
    // General
    OPTIONS_DIFFICULTY,
    OPTIONS_HIDEGUI,
    OPTIONS_THIRD_PERSON_VIEW,
    OPTIONS_GUI_SCALE,
    OPTIONS_DESTROY_VIBRATION,
    OPTIONS_MUSIC_VOLUME,
    OPTIONS_SOUND_VOLUME,
    OPTIONS_SKIN,
    OPTIONS_USERNAME,
    OPTIONS_LANGUAGE,
    OPTIONS_SERVER_VISIBLE,
    OPTIONS_BAR_ON_TOP,
    OPTIONS_ALLOW_SPRINT,
    OPTIONS_AUTOJUMP,

    // Graphics
    OPTIONS_RENDER_DEBUG,
    OPTIONS_SMOOTH_CAMERA,
    OPTIONS_FIXED_CAMERA,
    OPTIONS_VIEW_DISTANCE,
    OPTIONS_VIEW_BOBBING,
    OPTIONS_AMBIENT_OCCLUSION,
    OPTIONS_SMOOTH_LIGHTNING,
    OPTIONS_ANAGLYPH_3D,
    OPTIONS_LIMIT_FRAMERATE,
    OPTIONS_VSYNC,
    OPTIONS_FANCY_GRAPHICS,
    OPTIONS_FOV,
    OPTIONS_BRIGHTNESS,

    // Cheats / debug
    OPTIONS_FLY_SPEED,
    OPTIONS_CAMERA_SPEED,
    OPTIONS_IS_FLYING,

    // Control
    OPTIONS_USE_MOUSE_FOR_DIGGING,
    OPTIONS_IS_LEFT_HANDED,
    OPTIONS_IS_JOY_TOUCH_AREA,
    OPTIONS_SENSITIVITY,
    OPTIONS_INVERT_Y_MOUSE,
    OPTIONS_USE_TOUCHSCREEN,

    OPTIONS_KEY_FORWARD,
    OPTIONS_KEY_LEFT,
    OPTIONS_KEY_BACK,
    OPTIONS_KEY_RIGHT,
    OPTIONS_KEY_JUMP,
    OPTIONS_KEY_INVENTORY,
    OPTIONS_KEY_SNEAK,
    OPTIONS_KEY_DROP,
    OPTIONS_KEY_CHAT,
    OPTIONS_KEY_FOG,
    OPTIONS_KEY_USE,

    OPTIONS_KEY_MENU_NEXT,
    OPTIONS_KEY_MENU_PREV,
    OPTIONS_KEY_MENU_OK,
    OPTIONS_KEY_MENU_CANCEL,

    OPTIONS_FIRST_LAUNCH,
    OPTIONS_LAST_IP,

    OPTIONS_RPI_CURSOR,

    // Skin Customization
    OPTIONS_SKIN_CAPE,
    OPTIONS_SKIN_JACKET,
    OPTIONS_SKIN_LEFT_SLEEVE,
    OPTIONS_SKIN_RIGHT_SLEEVE,
    OPTIONS_SKIN_LEFT_PANTS,
    OPTIONS_SKIN_RIGHT_PANTS,
    OPTIONS_SKIN_HAT,
    OPTIONS_SKIN_MAIN_HAND,
    OPTIONS_SKIN_MODEL,

    // Graphics additions
    OPTIONS_REDUCE_FPS_IDLE,
    OPTIONS_FULLSCREEN,
    OPTIONS_GRAPHICS_API,
    OPTIONS_BIOME_TRANSITION,
    OPTIONS_CHUNK_RENDER,
    OPTIONS_CHUNK_GENERATE,
    OPTIONS_CHUNK_SIMULATION,
    OPTIONS_CLOUDS,
    OPTIONS_PARTICLES,
    OPTIONS_MIPMAP_LEVEL,
    OPTIONS_ENTITY_SHADOWS,
    OPTIONS_ENTITY_DISTANCE,
    OPTIONS_MENU_BLUR,
    OPTIONS_CLOUD_DISTANCE,
    OPTIONS_SEE_THROUGH_LEAVES,
    OPTIONS_ENHANCED_TRANSPARENCY,
    OPTIONS_TEXTURE_FILTERING,
    OPTIONS_ANISOTROPIC_FILTERING,
    OPTIONS_WEATHER_RADIUS,
    OPTIONS_AUTOSAVE_INDICATOR,
    OPTIONS_SHOW_VIGNETTE,
    OPTIONS_ATTACK_INDICATOR,
    OPTIONS_CHUNK_BLUR,

    // Music/Sound Additions
    OPTIONS_MUSIC_BLOCKS_VOLUME,
    OPTIONS_WEATHER_VOLUME,
    OPTIONS_BLOCKS_VOLUME,
    OPTIONS_HOSTILE_VOLUME,
    OPTIONS_PASSIVE_VOLUME,
    OPTIONS_PLAYER_VOLUME,
    OPTIONS_AMBIENT_VOLUME,
    OPTIONS_NARRATOR_VOLUME,
    OPTIONS_UI_VOLUME,
    OPTIONS_SOUND_DEVICE,
    OPTIONS_SHOW_SUBTITLES,
    OPTIONS_DIRECTIONAL_AUDIO,
    OPTIONS_DEFAULT_MUSIC,
    OPTIONS_MUSIC_WARNING,

    // Controls Additions
    OPTIONS_SPRINT_INTERVAL,
    OPTIONS_ADMIN_TAB,

    // Mouse Additions
    OPTIONS_DISCRETE_SCROLL,
    OPTIONS_DIRECT_INPUT,
    OPTIONS_WHEEL_SENSITIVITY,
    OPTIONS_INVERT_X_MOUSE,
    OPTIONS_ALLOW_CURSOR_CHANGES,

    // Chat Settings
    OPTIONS_CHAT_VISIBLE,
    OPTIONS_CHAT_COLORS,
    OPTIONS_WEB_LINKS,
    OPTIONS_WARN_WEB_LINKS,
    OPTIONS_CHAT_OPACITY,
    OPTIONS_CHAT_BG_OPACITY,
    OPTIONS_CHAT_SIZE,
    OPTIONS_CHAT_LINE_SPACING,
    OPTIONS_CHAT_SLOWDOWN,
    OPTIONS_CHAT_WIDTH,
    OPTIONS_CHAT_MAX_HEIGHT_OPEN,
    OPTIONS_CHAT_MAX_HEIGHT_CLOSED,
    OPTIONS_SUGGEST_COMMANDS,
    OPTIONS_HIGHLIGHT_MATCHING_NAMES,
    OPTIONS_REDUCE_F3,
    OPTIONS_ONLY_SECURE_MSGS,
    OPTIONS_SHOW_UNSENT_MSGS,

    // Accessibility Additions
    OPTIONS_HIGH_CONTRAST,
    OPTIONS_WARNING_DURATION,
    OPTIONS_BOBBING,
    OPTIONS_DISTORTION_EFFECTS,
    OPTIONS_FOV_EFFECTS,
    OPTIONS_DARKNESS_PULSE,
    OPTIONS_DAMAGE_TILT,
    OPTIONS_FLASH_SPEED,
    OPTIONS_FLASH_INTENSITY,
    OPTIONS_HIDE_LIGHTNING,
    OPTIONS_MONOCHROME_LOGO,
    OPTIONS_PANORAMA_SPEED,
    OPTIONS_HIDE_SPLASH_TEXTS,
    OPTIONS_NARRATOR_SHORTCUT,
    OPTIONS_SPIN_WITH_MINECARTS,
    OPTIONS_HIGH_CONTRAST_BORDERS,

	// Should be last!
	OPTIONS_COUNT
};

class Minecraft;
typedef std::vector<std::string> StringVector;

class Options
{
public:
    static bool debugGl;

    Options(Minecraft* minecraft, const std::string& workingDirectory = "") 
	: minecraft(minecraft) {
        // elements werent initialized so i was getting a garbage pointer and a crash
        m_options.fill(nullptr);
        initTable();
	    // load() is deferred to init() where path is configured correctly
    }

    void initTable();

    int getIntValue(OptionId key) {
        auto option = opt<OptionInt>(key);
        return (option)? option->get() : 0;
    }

    std::string getStringValue(OptionId key) {
        auto option = opt<OptionString>(key);
        return (option)? option->get() : "";
    }

    float getProgressValue(OptionId key) {
        auto option = opt<OptionFloat>(key);
        return (option)? option->get() : 0.f;
    }

    bool getBooleanValue(OptionId key) {
        auto option = opt<OptionBool>(key);
        return (option)? option->get() : false;
    }

    float getProgrssMin(OptionId key) {
        auto option = opt<OptionFloat>(key);
        return (option)? option->getMin() : 0.f;
    }

    float getProgrssMax(OptionId key) {
        auto option = opt<OptionFloat>(key);
        return (option)? option->getMax() : 0.f;
    }

    Option* getOpt(OptionId id) { return m_options[id]; }

    void load();
    void save();
    void set(OptionId key, int value);
    void set(OptionId key, float value);
    void set(OptionId key, const std::string& value);
	void setOptionsFilePath(const std::string& path);
	void toggle(OptionId key);

	void notifyOptionUpdate(OptionId key, bool value);
	void notifyOptionUpdate(OptionId key, float value);
	void notifyOptionUpdate(OptionId key, int value);
    void notifyOptionUpdate(OptionId key, const std::string& value);

private:
    template<typename T>
    T* opt(OptionId key) { 
        if (m_options[key] == nullptr) return nullptr;
        return dynamic_cast<T*>(m_options[key]); 
    }

	std::array<Option*, OPTIONS_COUNT> m_options;
	OptionsFile optionsFile;

	Minecraft* minecraft;
};

#endif /*NET_MINECRAFT_CLIENT__Options_H__*/
