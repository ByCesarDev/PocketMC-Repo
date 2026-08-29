#include "Options.h"
#include "OptionStrings.h"
#include "Minecraft.h"
#include "../platform/log.h"
#include "../world/Difficulty.h"
#include <cmath>

#include <memory>

bool Options::debugGl = false;

// OPTIONS TABLE

OptionInt difficulty("difficulty", Difficulty::NORMAL, 0, Difficulty::COUNT);
OptionBool hidegui("hidegui", false);
OptionInt thirdPersonView("thirdperson", 0, 0, 2);
OptionBool renderDebug("renderDebug", false);
OptionBool smoothCamera("smoothCamera", false);
OptionBool fixedCamera("fixedCamera", false);
OptionBool isFlying("isflying", false);
OptionBool barOnTop("barOnTop", false);
OptionBool allowSprint("allowSprint", true);
OptionBool rpiCursor("rpiCursor", false);
OptionBool autoJump("autoJump", true);


OptionFloat flySpeed("flySpeed", 1.f);
OptionFloat cameraSpeed("cameraSpeed", 1.f);

OptionInt guiScale("guiScale", 0, 0, 5);

OptionString skin("skin", "Default");

#ifdef RPI
OptionString username("username", "StevePi");
#else 
OptionString username("username", "Steve");
#endif

// language option maps to an index in the languages list. Default 0 = en_US
OptionInt language("language", 0, 0, 1);

OptionBool destroyVibration("destroyVibration", true);
OptionBool isLeftHanded("isLeftHanded", false);
OptionBool isJoyTouchArea("isJoyTouchArea", false);

OptionFloat musicVolume("music", 1.f, MUSIC_MIN_VALUE, MUSIC_MAX_VALUE);
OptionFloat soundVolume("sound", 1.f, SOUND_MIN_VALUE, SOUND_MAX_VALUE);

OptionFloat sensitivityOpt("sensitivity", 0.5f, SENSITIVITY_MIN_VALUE, SENSITIVITY_MAX_VALUE);

OptionBool invertYMouse("invertMouse", false);
OptionInt viewDistance("renderDistance", 2, 0, 3);

OptionBool anaglyph3d("anaglyph3d", false);
OptionBool limitFramerate("limitFramerate", false);
OptionBool vsync("vsync", true);
OptionFloat fovOpt("fov", 70.f, 70.f, 110.f);
OptionFloat brightnessOpt("gamma", 0.f, 0.f, 1.f);

OptionBool fancyGraphics("fancyGraphics", false);
OptionBool viewBobbing("viewBobbing", true);
OptionBool ambientOcclusion("ao", true);
OptionBool smoothLighting(OptionStrings::Graphics_SmoothLightning, true);

OptionBool useTouchscreen("useTouchscreen", true);

OptionBool serverVisible("servervisible", true);

OptionInt keyForward("key.forward", Keyboard::KEY_W);
OptionInt keyLeft("key.left", Keyboard::KEY_A);
OptionInt keyBack("key.back", Keyboard::KEY_S);
OptionInt keyRight("key.right", Keyboard::KEY_D);
OptionInt keyJump("key.jump", Keyboard::KEY_SPACE);
OptionInt keyInventory("key.inventory", Keyboard::KEY_E);
OptionInt keySneak("key.sneak", Keyboard::KEY_LSHIFT);
OptionInt keyDrop("key.drop", Keyboard::KEY_Q);
OptionInt keyChat("key.chat", Keyboard::KEY_T);
OptionInt keyFog("key.fog", Keyboard::KEY_F);
OptionInt keyUse("key.use", Keyboard::KEY_U);

// TODO: make human readable keycodes here
OptionInt keyMenuNext("key.menu.next", 40);
OptionInt keyMenuPrev("key.menu.previous", 38);
OptionInt keyMenuOk("key.menu.ok", 13);
OptionInt keyMenuCancel("key.menu.cancel", 8);

OptionBool firstLaunch("firstLaunch", true);

OptionString lastIp("lastip");

// Skin Customization
OptionBool skinCape("skinCape", true);
OptionBool skinJacket("skinJacket", true);
OptionBool skinLeftSleeve("skinLeftSleeve", true);
OptionBool skinRightSleeve("skinRightSleeve", true);
OptionBool skinLeftPants("skinLeftPants", true);
OptionBool skinRightPants("skinRightPants", true);
OptionBool skinHat("skinHat", true);
OptionBool skinMainHand("skinMainHand", true);
OptionString skinModel("skinModel", "normal");

// Graphics additions
OptionBool reduceFpsIdle("reduceFpsIdle", false);
OptionBool fullscreenOpt("fullscreen", false);
OptionInt graphicsApi("graphicsApi", 0, 0, 2);
OptionBool biomeTransition("biomeTransition", true);
OptionBool chunkRender("chunkRender", true);
OptionBool chunkGenerate("chunkGenerate", true);
OptionBool chunkSimulation("chunkSimulation", true);
OptionBool clouds("clouds", true);
OptionBool particles("particles", true);
OptionInt mipmapLevel("mipmapLevel", 4, 0, 4);
OptionBool entityShadows("entityShadows", true);
OptionFloat entityDistance("entityDistance", 1.0f, 0.0f, 2.0f);
OptionBool menuBlur("menuBlur", true);
OptionFloat cloudDistance("cloudDistance", 1.0f, 0.0f, 2.0f);
OptionBool seeThroughLeaves("seeThroughLeaves", true);
OptionBool enhancedTransparency("enhancedTransparency", true);
OptionBool textureFiltering("textureFiltering", true);
OptionBool anisotropicFiltering("anisotropicFiltering", false);
OptionInt weatherRadius("weatherRadius", 2, 0, 4);
OptionBool autosaveIndicator("autosaveIndicator", true);
OptionBool showVignette("showVignette", true);
OptionBool attackIndicator("attackIndicator", true);
OptionBool chunkBlur("chunkBlur", false);

// Music/Sound Additions
OptionFloat musicBlocksVolume("musicBlocksVolume", 1.0f, 0.0f, 1.0f);
OptionFloat weatherVolume("weatherVolume", 1.0f, 0.0f, 1.0f);
OptionFloat blocksVolume("blocksVolume", 1.0f, 0.0f, 1.0f);
OptionFloat hostileVolume("hostileVolume", 1.0f, 0.0f, 1.0f);
OptionFloat passiveVolume("passiveVolume", 1.0f, 0.0f, 1.0f);
OptionFloat playerVolume("playerVolume", 1.0f, 0.0f, 1.0f);
OptionFloat ambientVolume("ambientVolume", 1.0f, 0.0f, 1.0f);
OptionFloat narratorVolume("narratorVolume", 1.0f, 0.0f, 1.0f);
OptionFloat uiVolume("uiVolume", 1.0f, 0.0f, 1.0f);
OptionString soundDevice("soundDevice", "Default");
OptionBool showSubtitles("showSubtitles", false);
OptionBool directionalAudio("directionalAudio", false);
OptionBool defaultMusic("defaultMusic", true);
OptionBool musicWarning("musicWarning", true);

// Controls Additions
OptionFloat sprintInterval("sprintInterval", 0.5f, 0.0f, 1.0f);
OptionBool adminTab("adminTab", false);

// Mouse Additions
OptionBool discreteScroll("discreteScroll", false);
OptionBool directInput("directInput", false);
OptionFloat wheelSensitivity("wheelSensitivity", 1.0f, 0.0f, 2.0f);
OptionBool invertXMouse("invertXMouse", false);
OptionBool allowCursorChanges("allowCursorChanges", true);

// Chat Settings
OptionBool chatVisible("chatVisible", true);
OptionBool chatColors("chatColors", true);
OptionBool webLinks("webLinks", true);
OptionBool warnWebLinks("warnWebLinks", true);
OptionFloat chatOpacity("chatOpacity", 1.0f, 0.0f, 1.0f);
OptionFloat chatBgOpacity("chatBgOpacity", 0.5f, 0.0f, 1.0f);
OptionFloat chatSize("chatSize", 1.0f, 0.0f, 2.0f);
OptionFloat chatLineSpacing("chatLineSpacing", 0.0f, 0.0f, 1.0f);
OptionBool chatSlowdown("chatSlowdown", false);
OptionFloat chatWidth("chatWidth", 1.0f, 0.0f, 2.0f);
OptionFloat chatMaxHeightOpen("chatMaxHeightOpen", 1.0f, 0.0f, 2.0f);
OptionFloat chatMaxHeightClosed("chatMaxHeightClosed", 1.0f, 0.0f, 2.0f);
OptionBool suggestCommands("suggestCommands", true);
OptionBool highlightMatchingNames("highlightMatchingNames", true);
OptionBool reduceF3("reduceF3", false);
OptionBool onlySecureMsgs("onlySecureMsgs", false);
OptionBool showUnsentMsgs("showUnsentMsgs", true);

// Accessibility Additions
OptionBool highContrast("highContrast", false);
OptionFloat warningDuration("warningDuration", 1.0f, 0.0f, 2.0f);
OptionBool bobbingOpt("bobbingOpt", true);
OptionBool distortionEffects("distortionEffects", true);
OptionBool fovEffects("fovEffects", true);
OptionBool darknessPulse("darknessPulse", true);
OptionBool damageTilt("damageTilt", true);
OptionFloat flashSpeed("flashSpeed", 1.0f, 0.0f, 2.0f);
OptionFloat flashIntensity("flashIntensity", 1.0f, 0.0f, 2.0f);
OptionBool hideLightning("hideLightning", false);
OptionBool monochromeLogo("monochromeLogo", false);
OptionFloat panoramaSpeed("panoramaSpeed", 1.0f, 0.0f, 2.0f);
OptionString panoramaPath("panoramaPath", "gui/panorama/");
OptionBool hideSplashTexts("hideSplashTexts", false);
OptionBool narratorShortcut("narratorShortcut", false);
OptionBool spinWithMinecarts("spinWithMinecarts", true);
OptionBool highContrastBorders("highContrastBorders", false);

// UI Profile & Safe Area
OptionInt uiProfile("uiProfile", 0, 0, 1);
OptionFloat safeArea("safeArea", 1.0f, 0.5f, 1.0f);

void Options::initTable() {
    m_options[OPTIONS_DIFFICULTY] = &difficulty;
    m_options[OPTIONS_HIDEGUI] = &hidegui;
    m_options[OPTIONS_THIRD_PERSON_VIEW] = &thirdPersonView;
    m_options[OPTIONS_RENDER_DEBUG] = &renderDebug;
    m_options[OPTIONS_SMOOTH_CAMERA] = &smoothCamera;
    m_options[OPTIONS_FIXED_CAMERA] = &fixedCamera;
	m_options[OPTIONS_IS_FLYING] = &isFlying;

	m_options[OPTIONS_FLY_SPEED] = &flySpeed;
	m_options[OPTIONS_CAMERA_SPEED] = &cameraSpeed;

	m_options[OPTIONS_GUI_SCALE] = &guiScale;

	m_options[OPTIONS_DESTROY_VIBRATION] = &destroyVibration;

	m_options[OPTIONS_IS_LEFT_HANDED] = &isLeftHanded;
	m_options[OPTIONS_IS_JOY_TOUCH_AREA] = &isJoyTouchArea;

	m_options[OPTIONS_MUSIC_VOLUME] = &musicVolume;
	m_options[OPTIONS_SOUND_VOLUME] = &soundVolume;

	#if defined(PLATFORM_DESKTOP) || defined(RPI)
		float sensitivity = sensitivityOpt.get();
		sensitivity *= 0.4f;
		sensitivityOpt.set(sensitivity);
	#endif


    m_options[OPTIONS_GUI_SCALE] = &guiScale;

	m_options[OPTIONS_SKIN] = &skin;
	m_options[OPTIONS_USERNAME] = &username;
	m_options[OPTIONS_LANGUAGE] = &language;

    m_options[OPTIONS_DESTROY_VIBRATION] = &destroyVibration;
    m_options[OPTIONS_IS_LEFT_HANDED] = &isLeftHanded;

    m_options[OPTIONS_MUSIC_VOLUME] = &musicVolume;
    m_options[OPTIONS_SOUND_VOLUME] = &soundVolume;

    m_options[OPTIONS_SENSITIVITY] = &sensitivityOpt;

    m_options[OPTIONS_INVERT_Y_MOUSE] = &invertYMouse;
    m_options[OPTIONS_VIEW_DISTANCE] = &viewDistance;

    m_options[OPTIONS_ANAGLYPH_3D] = &anaglyph3d;
    m_options[OPTIONS_LIMIT_FRAMERATE] = &limitFramerate;
    m_options[OPTIONS_VSYNC] = &vsync;
    m_options[OPTIONS_FANCY_GRAPHICS] = &fancyGraphics;
    m_options[OPTIONS_FOV] = &fovOpt;
    m_options[OPTIONS_BRIGHTNESS] = &brightnessOpt;
	m_options[OPTIONS_VIEW_BOBBING] = &viewBobbing;
	m_options[OPTIONS_AMBIENT_OCCLUSION] = &ambientOcclusion;
	m_options[OPTIONS_SMOOTH_LIGHTNING] = &smoothLighting;

    m_options[OPTIONS_USE_TOUCHSCREEN] = &useTouchscreen;

    m_options[OPTIONS_SERVER_VISIBLE] = &serverVisible;

    m_options[OPTIONS_KEY_FORWARD] = &keyForward;
    m_options[OPTIONS_KEY_LEFT] = &keyLeft;
    m_options[OPTIONS_KEY_BACK] = &keyBack;
    m_options[OPTIONS_KEY_RIGHT] = &keyRight;
    m_options[OPTIONS_KEY_JUMP] = &keyJump;
    m_options[OPTIONS_KEY_INVENTORY] = &keyInventory;
    m_options[OPTIONS_KEY_SNEAK] = &keySneak;
    m_options[OPTIONS_KEY_DROP] = &keyDrop;
    m_options[OPTIONS_KEY_CHAT] = &keyChat;
    m_options[OPTIONS_KEY_FOG] = &keyFog;
    m_options[OPTIONS_KEY_USE] = &keyUse;

    m_options[OPTIONS_KEY_MENU_NEXT] = &keyMenuNext;
    m_options[OPTIONS_KEY_MENU_PREV] = &keyMenuPrev;
    m_options[OPTIONS_KEY_MENU_OK] = &keyMenuOk;
    m_options[OPTIONS_KEY_MENU_CANCEL] = &keyMenuCancel;

	m_options[OPTIONS_FIRST_LAUNCH] = &firstLaunch;

	m_options[OPTIONS_BAR_ON_TOP] = &barOnTop;
	m_options[OPTIONS_ALLOW_SPRINT] = &allowSprint;
	m_options[OPTIONS_RPI_CURSOR] = &rpiCursor;

	m_options[OPTIONS_AUTOJUMP] = &autoJump;
	m_options[OPTIONS_LAST_IP] = &lastIp;

    m_options[OPTIONS_SKIN_CAPE] = &skinCape;
    m_options[OPTIONS_SKIN_JACKET] = &skinJacket;
    m_options[OPTIONS_SKIN_LEFT_SLEEVE] = &skinLeftSleeve;
    m_options[OPTIONS_SKIN_RIGHT_SLEEVE] = &skinRightSleeve;
    m_options[OPTIONS_SKIN_LEFT_PANTS] = &skinLeftPants;
    m_options[OPTIONS_SKIN_RIGHT_PANTS] = &skinRightPants;
    m_options[OPTIONS_SKIN_HAT] = &skinHat;
    m_options[OPTIONS_SKIN_MAIN_HAND] = &skinMainHand;
    m_options[OPTIONS_SKIN_MODEL] = &skinModel;
    m_options[OPTIONS_REDUCE_FPS_IDLE] = &reduceFpsIdle;
    m_options[OPTIONS_FULLSCREEN] = &fullscreenOpt;
    m_options[OPTIONS_GRAPHICS_API] = &graphicsApi;
    m_options[OPTIONS_BIOME_TRANSITION] = &biomeTransition;
    m_options[OPTIONS_CHUNK_RENDER] = &chunkRender;
    m_options[OPTIONS_CHUNK_GENERATE] = &chunkGenerate;
    m_options[OPTIONS_CHUNK_SIMULATION] = &chunkSimulation;
    m_options[OPTIONS_CLOUDS] = &clouds;
    m_options[OPTIONS_PARTICLES] = &particles;
    m_options[OPTIONS_MIPMAP_LEVEL] = &mipmapLevel;
    m_options[OPTIONS_ENTITY_SHADOWS] = &entityShadows;
    m_options[OPTIONS_ENTITY_DISTANCE] = &entityDistance;
    m_options[OPTIONS_MENU_BLUR] = &menuBlur;
    m_options[OPTIONS_CLOUD_DISTANCE] = &cloudDistance;
    m_options[OPTIONS_SEE_THROUGH_LEAVES] = &seeThroughLeaves;
    m_options[OPTIONS_ENHANCED_TRANSPARENCY] = &enhancedTransparency;
    m_options[OPTIONS_TEXTURE_FILTERING] = &textureFiltering;
    m_options[OPTIONS_ANISOTROPIC_FILTERING] = &anisotropicFiltering;
    m_options[OPTIONS_WEATHER_RADIUS] = &weatherRadius;
    m_options[OPTIONS_AUTOSAVE_INDICATOR] = &autosaveIndicator;
    m_options[OPTIONS_SHOW_VIGNETTE] = &showVignette;
    m_options[OPTIONS_ATTACK_INDICATOR] = &attackIndicator;
    m_options[OPTIONS_CHUNK_BLUR] = &chunkBlur;
    m_options[OPTIONS_MUSIC_BLOCKS_VOLUME] = &musicBlocksVolume;
    m_options[OPTIONS_WEATHER_VOLUME] = &weatherVolume;
    m_options[OPTIONS_BLOCKS_VOLUME] = &blocksVolume;
    m_options[OPTIONS_HOSTILE_VOLUME] = &hostileVolume;
    m_options[OPTIONS_PASSIVE_VOLUME] = &passiveVolume;
    m_options[OPTIONS_PLAYER_VOLUME] = &playerVolume;
    m_options[OPTIONS_AMBIENT_VOLUME] = &ambientVolume;
    m_options[OPTIONS_NARRATOR_VOLUME] = &narratorVolume;
    m_options[OPTIONS_UI_VOLUME] = &uiVolume;
    m_options[OPTIONS_SOUND_DEVICE] = &soundDevice;
    m_options[OPTIONS_SHOW_SUBTITLES] = &showSubtitles;
    m_options[OPTIONS_DIRECTIONAL_AUDIO] = &directionalAudio;
    m_options[OPTIONS_DEFAULT_MUSIC] = &defaultMusic;
    m_options[OPTIONS_MUSIC_WARNING] = &musicWarning;
    m_options[OPTIONS_SPRINT_INTERVAL] = &sprintInterval;
    m_options[OPTIONS_ADMIN_TAB] = &adminTab;
    m_options[OPTIONS_DISCRETE_SCROLL] = &discreteScroll;
    m_options[OPTIONS_DIRECT_INPUT] = &directInput;
    m_options[OPTIONS_WHEEL_SENSITIVITY] = &wheelSensitivity;
    m_options[OPTIONS_INVERT_X_MOUSE] = &invertXMouse;
    m_options[OPTIONS_ALLOW_CURSOR_CHANGES] = &allowCursorChanges;
    m_options[OPTIONS_CHAT_VISIBLE] = &chatVisible;
    m_options[OPTIONS_CHAT_COLORS] = &chatColors;
    m_options[OPTIONS_WEB_LINKS] = &webLinks;
    m_options[OPTIONS_WARN_WEB_LINKS] = &warnWebLinks;
    m_options[OPTIONS_CHAT_OPACITY] = &chatOpacity;
    m_options[OPTIONS_CHAT_BG_OPACITY] = &chatBgOpacity;
    m_options[OPTIONS_CHAT_SIZE] = &chatSize;
    m_options[OPTIONS_CHAT_LINE_SPACING] = &chatLineSpacing;
    m_options[OPTIONS_CHAT_SLOWDOWN] = &chatSlowdown;
    m_options[OPTIONS_CHAT_WIDTH] = &chatWidth;
    m_options[OPTIONS_CHAT_MAX_HEIGHT_OPEN] = &chatMaxHeightOpen;
    m_options[OPTIONS_CHAT_MAX_HEIGHT_CLOSED] = &chatMaxHeightClosed;
    m_options[OPTIONS_SUGGEST_COMMANDS] = &suggestCommands;
    m_options[OPTIONS_HIGHLIGHT_MATCHING_NAMES] = &highlightMatchingNames;
    m_options[OPTIONS_REDUCE_F3] = &reduceF3;
    m_options[OPTIONS_ONLY_SECURE_MSGS] = &onlySecureMsgs;
    m_options[OPTIONS_SHOW_UNSENT_MSGS] = &showUnsentMsgs;
    m_options[OPTIONS_HIGH_CONTRAST] = &highContrast;
    m_options[OPTIONS_WARNING_DURATION] = &warningDuration;
    m_options[OPTIONS_BOBBING] = &bobbingOpt;
    m_options[OPTIONS_DISTORTION_EFFECTS] = &distortionEffects;
    m_options[OPTIONS_FOV_EFFECTS] = &fovEffects;
    m_options[OPTIONS_DARKNESS_PULSE] = &darknessPulse;
    m_options[OPTIONS_DAMAGE_TILT] = &damageTilt;
    m_options[OPTIONS_FLASH_SPEED] = &flashSpeed;
    m_options[OPTIONS_FLASH_INTENSITY] = &flashIntensity;
    m_options[OPTIONS_HIDE_LIGHTNING] = &hideLightning;
    m_options[OPTIONS_MONOCHROME_LOGO] = &monochromeLogo;
    m_options[OPTIONS_PANORAMA_SPEED] = &panoramaSpeed;
    m_options[OPTIONS_PANORAMA_PATH] = &panoramaPath;
    m_options[OPTIONS_HIDE_SPLASH_TEXTS] = &hideSplashTexts;
    m_options[OPTIONS_NARRATOR_SHORTCUT] = &narratorShortcut;
    m_options[OPTIONS_SPIN_WITH_MINECARTS] = &spinWithMinecarts;
    m_options[OPTIONS_HIGH_CONTRAST_BORDERS] = &highContrastBorders;
    m_options[OPTIONS_UI_PROFILE] = &uiProfile;
    m_options[OPTIONS_SAFE_AREA] = &safeArea;
}

void Options::set(OptionId key, const std::string& value) {
	auto option = opt<OptionString>(key);

	if (option) {
		option->set(value);
		notifyOptionUpdate(key, value);
	}
}

void Options::set(OptionId key, float value) {
	auto option = opt<OptionFloat>(key);

	if (option) {
		option->set(value);
		notifyOptionUpdate(key, value);
	}
}

void Options::set(OptionId key, int value) {
	auto option = opt<OptionInt>(key);

	if (option) {
		option->set(value);
		notifyOptionUpdate(key, value);
	}
}

void Options::toggle(OptionId key) {
	auto optionBool = opt<OptionBool>(key);

	if (optionBool) {
		optionBool->toggle();
		notifyOptionUpdate(key, optionBool->get());
		return;
	}

	auto optionInt = opt<OptionInt>(key);
	if (optionInt) {
		if (key == OPTIONS_THIRD_PERSON_VIEW) {
			optionInt->set((optionInt->get() + 1) % 3);
		} else {
			// fallback toggle for other ints? for now only third person
			optionInt->set(optionInt->get() == 0 ? 1 : 0);
		}
	}
}

void Options::load() {
	StringVector optionStrings = optionsFile.getOptionStrings();

	for (auto i = 0; i < optionStrings.size(); i += 2) {
		const std::string& key = optionStrings[i];
		const std::string& value = optionStrings[i+1];

		// FIXME: woah this is so slow 
		auto opt = std::find_if(m_options.begin(), m_options.end(), [&](auto& it) {
			return it != nullptr && it->getStringId() == key;
		});

		if (opt == m_options.end()) continue;

		(*opt)->parse(value);
/*
        // //LOGI("reading key: %s (%s)\n", key.c_str(), value.c_str());
        
		// // Multiplayer
		// // if (key == OptionStrings::Multiplayer_Username) username = value;
		// if (key == OptionStrings::Multiplayer_ServerVisible) {
		// 	m_options[OPTIONS_SERVER_VISIBLE] = readBool(value);
		// }

		// // Controls
        // if (key == OptionStrings::Controls_Sensitivity) {
		// 	float sens = readFloat(value);

		// 	// sens is in range [0,1] with default/center at 0.5 (for aesthetics)
        //     // We wanna map it to something like [0.3, 0.9] BUT keep 0.5 @ ~0.5...
        //     m_options[OPTIONS_SENSITIVITY] = 0.3f + std::pow(1.1f * sens, 1.3f) * 0.42f;
        // }

		// if (key == OptionStrings::Controls_InvertMouse) {
		// 	m_options[OPTIONS_INVERT_Y_MOUSE] = readBool(value);
		// }

		// if (key == OptionStrings::Controls_IsLefthanded) {
		// 	m_options[OPTIONS_IS_LEFT_HANDED] = readBool(value);
		// }
		
		// if (key == OptionStrings::Controls_UseTouchJoypad) {
		// 	m_options[OPTIONS_IS_JOY_TOUCH_AREA] = readBool(value) && minecraft->useTouchscreen();
		// }

		// // Feedback
		// if (key == OptionStrings::Controls_FeedbackVibration) {
		// 	m_options[OPTIONS_DESTROY_VIBRATION] = readBool(value);
		// }

		// // Graphics
		// if (key == OptionStrings::Graphics_Fancy) {
		// 	m_options[OPTIONS_FANCY_GRAPHICS] = readBool(value);
		// }

		// // Graphics extras
		// if (key == OptionStrings::Graphics_Vsync) {
		// 	m_options[OPTIONS_VSYNC] = readBool(value);
		// }

		// if (key == OptionStrings::Graphics_GUIScale) {
		// 	m_options[OPTIONS_GUI_SCALE] = readInt(value) % 5;
		// }

		// // Game
		// if (key == OptionStrings::Game_DifficultyLevel) {
		// 	readInt(value, difficulty);
		// 	// Only support peaceful and normal right now
		// 	if (difficulty != Difficulty::PEACEFUL && difficulty != Difficulty::NORMAL)
		// 		difficulty = Difficulty::NORMAL;
		// }*/
	}
}

void Options::save() {
	StringVector stringVec;
	
	for (auto& it : m_options) {
		if (it) stringVec.push_back(it->serialize());
	}

	optionsFile.save(stringVec);
}

void Options::setOptionsFilePath(const std::string& path) {
	if (path.empty() || path == ".") {
		optionsFile.setOptionsPath("games/com.mojang/options.txt");
	} else if (path.find("games/com.mojang") != std::string::npos) {
		optionsFile.setOptionsPath(path + "/options.txt");
	} else {
		optionsFile.setOptionsPath(path + "/games/com.mojang/options.txt");
	}
}

void Options::notifyOptionUpdate(OptionId key, bool value) {
	minecraft->optionUpdated(key, value);
}

void Options::notifyOptionUpdate(OptionId key, float value) {
	minecraft->optionUpdated(key, value);
}

void Options::notifyOptionUpdate(OptionId key, int value) {
	minecraft->optionUpdated(key, value);
}

void Options::notifyOptionUpdate(OptionId key, const std::string& value) {
	minecraft->optionUpdated(key, value);
}
