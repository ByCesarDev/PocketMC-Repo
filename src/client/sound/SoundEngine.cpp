#include "SoundEngine.h"
#include "../Options.h"
#include "../Minecraft.h"
#include "../../world/entity/Mob.h"


#include <vector>
#include <string>
#include <fstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <mfplay.h>
#pragma comment(lib, "mfplay.lib")

static IMFPMediaPlayer* g_pMFPlayer = NULL;

static void cleanupMFPlayer() {
	if (g_pMFPlayer) {
		g_pMFPlayer->Stop();
		g_pMFPlayer->Release();
		g_pMFPlayer = NULL;
	}
}
#endif

static const std::vector<std::string> g_musicTracks = {
	"data/sound/music/C418 - Blind Spots.m4a",
	"data/sound/music/C418 - Clark.m4a",
	"data/sound/music/C418 - Danny.m4a",
	"data/sound/music/C418 - Dry Hands.m4a",
	"data/sound/music/C418 - Haggstrom.m4a",
	"data/sound/music/C418 - Key.m4a",
	"data/sound/music/C418 - Living Mice.m4a",
	"data/sound/music/C418 - Mice on Venus.m4a",
	"data/sound/music/C418 - Minecraft.m4a",
	"data/sound/music/C418 - Oxygene.m4a",
	"data/sound/music/C418 - Subwoofer Lullaby.m4a",
	"data/sound/music/C418 - Sweden.m4a",
	"data/sound/music/C418 - Wet Hands.m4a"
};

SoundEngine::SoundEngine( float maxDistance )
:	idCounter(0),
	mc(0),
	_x(0),
	_y(0),
	_z(0),
	_yRot(0),
	_invMaxDistance(1.0f / maxDistance),
	isMusicPlaying(false),
	musicDelayTimer(40),
	musicTrackGraceTicks(0),
	lastTrackIndex(-1),
	lastMusicVolume(1.0f),
	currentFadeTicks(0),
	fadeTicksMax(100),
	isFadingOut(false),
	fadeOutTicks(100),
	zeroVolumeTimer(0)
{

}

SoundEngine::~SoundEngine()
{

}

void SoundEngine::init( Minecraft* mc, Options* options )
{
	this->mc = mc;
	this->options = options;

	if (/*!loaded && */(options == NULL || 
		(options->getProgressValue(OPTIONS_SOUND_VOLUME) != 0.f || 
		options->getProgressValue(OPTIONS_MUSIC_VOLUME) != 0.f))) 
	{
		loadLibrary();
	}

#if !defined(PRE_ANDROID23) && !defined(__APPLE__) && !defined(RPI)
	sounds.add("step.cloth", SA_cloth1);
	sounds.add("step.cloth", SA_cloth2);
	sounds.add("step.cloth", SA_cloth3);
	sounds.add("step.cloth", SA_cloth4);
	sounds.add("step.grass", SA_grass1);
	sounds.add("step.grass", SA_grass2);
	sounds.add("step.grass", SA_grass3);
	sounds.add("step.grass", SA_grass4);
	sounds.add("step.gravel", SA_gravel1);
	sounds.add("step.gravel", SA_gravel2);
	sounds.add("step.gravel", SA_gravel3);
	sounds.add("step.gravel", SA_gravel4);
	sounds.add("step.sand", SA_sand1);
	sounds.add("step.sand", SA_sand2);
	sounds.add("step.sand", SA_sand3);
	sounds.add("step.sand", SA_sand4);
	sounds.add("step.stone", SA_stone1);
	sounds.add("step.stone", SA_stone2);
	sounds.add("step.stone", SA_stone3);
	sounds.add("step.stone", SA_stone4);
	sounds.add("step.wood", SA_wood1);
	sounds.add("step.wood", SA_wood2);
	sounds.add("step.wood", SA_wood3);
	sounds.add("step.wood", SA_wood4);

	sounds.add("random.splash", SA_splash);
	sounds.add("random.explode", SA_explode);
	sounds.add("random.click", SA_click);
	sounds.add("fire.ignite", SA_click);

	sounds.add("random.door_open", SA_door_open);
	sounds.add("random.door_close", SA_door_close);
	sounds.add("random.pop", SA_pop);
	sounds.add("random.pop2", SA_pop2);
	sounds.add("random.hurt", SA_hurt);
	sounds.add("random.glass", SA_glass1);
	sounds.add("random.glass", SA_glass2);
	sounds.add("random.glass", SA_glass3);

	sounds.add("mob.sheep", SA_sheep1);
	sounds.add("mob.sheep", SA_sheep2);
	sounds.add("mob.sheep", SA_sheep3);
	sounds.add("mob.pig", SA_pig1);
	sounds.add("mob.pig", SA_pig2);
	sounds.add("mob.pig", SA_pig3);
	sounds.add("mob.pigdeath", SA_pigdeath);

	sounds.add("mob.cow", SA_cow1);
	sounds.add("mob.cow", SA_cow2);
	sounds.add("mob.cow", SA_cow3);
	sounds.add("mob.cow", SA_cow4);
	sounds.add("mob.cowhurt", SA_cowhurt1);
	sounds.add("mob.cowhurt", SA_cowhurt2);
	sounds.add("mob.cowhurt", SA_cowhurt3);

	sounds.add("mob.chicken", SA_chicken2);
	sounds.add("mob.chicken", SA_chicken3);
	sounds.add("mob.chickenhurt", SA_chickenhurt1);
	sounds.add("mob.chickenhurt", SA_chickenhurt2);

	sounds.add("mob.zombie", SA_zombie1);
	sounds.add("mob.zombie", SA_zombie2);
	sounds.add("mob.zombie", SA_zombie3); 
	sounds.add("mob.zombiedeath", SA_zombiedeath);
	sounds.add("mob.zombiehurt", SA_zombiehurt1);
	sounds.add("mob.zombiehurt", SA_zombiehurt2);

	sounds.add("mob.skeleton", SA_skeleton1);
	sounds.add("mob.skeleton", SA_skeleton2);
	sounds.add("mob.skeleton", SA_skeleton3);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt1);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt2);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt3);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt4);

	sounds.add("mob.spider", SA_spider1);
	sounds.add("mob.spider", SA_spider2);
	sounds.add("mob.spider", SA_spider3);
	sounds.add("mob.spider", SA_spider4);
	sounds.add("mob.spiderdeath", SA_spiderdeath);

	sounds.add("mob.zombiepig.zpig", SA_zpig1);
	sounds.add("mob.zombiepig.zpig", SA_zpig2);
	sounds.add("mob.zombiepig.zpig", SA_zpig3);
	sounds.add("mob.zombiepig.zpig", SA_zpig4);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry1);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry2);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry3);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry4);
	sounds.add("mob.zombiepig.zpigdeath", SA_zpigdeath);
	sounds.add("mob.zombiepig.zpighurt", SA_zpighurt1);
	sounds.add("mob.zombiepig.zpighurt", SA_zpighurt2);

	sounds.add("damage.fallbig", SA_fallbig1);
	sounds.add("damage.fallbig", SA_fallbig2);
	sounds.add("damage.fallsmall", SA_fallsmall);

	sounds.add("random.bow", SA_bow);
	sounds.add("random.bowhit", SA_bowhit1);
	sounds.add("random.bowhit", SA_bowhit2);
	sounds.add("random.bowhit", SA_bowhit3);
	sounds.add("random.bowhit", SA_bowhit4);

	sounds.add("mob.creeper", SA_creeper1);
	sounds.add("mob.creeper", SA_creeper2);
	sounds.add("mob.creeper", SA_creeper3);
	sounds.add("mob.creeper", SA_creeper4);
	sounds.add("mob.creeperdeath", SA_creeperdeath);
	sounds.add("random.eat", SA_eat1);
	sounds.add("random.eat", SA_eat2);
	sounds.add("random.eat", SA_eat3);
	sounds.add("random.fuse", SA_fuse);

#endif
}

void SoundEngine::enable( bool status )
{
#if defined(__APPLE__)
	soundSystem.enable(status);
#endif
}

void SoundEngine::updateOptions()
{

}

void SoundEngine::destroy()
{
	//if (loaded) soundSystem.cleanup();
#ifdef _WIN32
	cleanupMFPlayer();
#endif
}

static std::string resolveMusicPath(const std::string& relPath) {
	{
		std::ifstream f(relPath.c_str());
		if (f.good()) return relPath;
	}
	{
		std::string p2 = "../" + relPath;
		std::ifstream f(p2.c_str());
		if (f.good()) return p2;
	}
	{
		std::string p3 = "../../" + relPath;
		std::ifstream f(p3.c_str());
		if (f.good()) return p3;
	}
	return relPath;
}

void SoundEngine::playRandomMusicTrack() {
	if (g_musicTracks.empty()) {
		LOGI("[SoundEngine] Error: g_musicTracks is empty!\n");
		return;
	}

	int nextTrackIndex = 0;
	if (lastTrackIndex == -1) {
		// First track is ALWAYS C418 - Sweden.m4a
		for (size_t i = 0; i < g_musicTracks.size(); ++i) {
			if (g_musicTracks[i].find("Sweden") != std::string::npos) {
				nextTrackIndex = (int)i;
				break;
			}
		}
	} else if (g_musicTracks.size() > 1) {
		do {
			nextTrackIndex = random.nextInt((int)g_musicTracks.size());
		} while (nextTrackIndex == lastTrackIndex);
	}
	lastTrackIndex = nextTrackIndex;

	std::string rawPath = g_musicTracks[nextTrackIndex];
	std::string trackPath = resolveMusicPath(rawPath);
	float volume = options ? options->getProgressValue(OPTIONS_MUSIC_VOLUME) : 1.0f;

	// Reset Fade-In/Out states and zero volume timer
	fadeTicksMax = 100; // 5 seconds fade (100 ticks @ 20tps)
	currentFadeTicks = 0;
	isFadingOut = false;
	fadeOutTicks = fadeTicksMax;
	zeroVolumeTimer = 0;

#ifdef _WIN32
	stopBackgroundMusic();

	std::string winPath = trackPath;
	std::replace(winPath.begin(), winPath.end(), '/', '\\');
	char absPath[MAX_PATH] = {0};
	if (GetFullPathNameA(winPath.c_str(), MAX_PATH, absPath, NULL) != 0) {
		winPath = absPath;
	}

	int wlen = MultiByteToWideChar(CP_UTF8, 0, winPath.c_str(), -1, NULL, 0);
	std::wstring wpath(wlen, 0);
	MultiByteToWideChar(CP_UTF8, 0, winPath.c_str(), -1, &wpath[0], wlen);

	HRESULT hr = MFPCreateMediaPlayer(wpath.c_str(), FALSE, 0, NULL, NULL, &g_pMFPlayer);
	if (SUCCEEDED(hr) && g_pMFPlayer) {
		setMusicVolume(0.0f); // Start at 0 volume for Fade-In
		hr = g_pMFPlayer->Play();
		if (SUCCEEDED(hr)) {
			isMusicPlaying = true;
			musicTrackGraceTicks = 100;
		} else {
			LOGI("[SoundEngine] MediaFoundation Play failed: 0x%08x\n", (unsigned int)hr);
			stopBackgroundMusic();
		}
	} else {
		LOGI("[SoundEngine] MediaFoundation MFPCreateMediaPlayer failed for '%s' (0x%08x)\n", winPath.c_str(), (unsigned int)hr);
		stopBackgroundMusic();
	}
#else
	if (mc && mc->platform()) {
		mc->platform()->playMusicTrack(trackPath, 0.0f);
		isMusicPlaying = true;
		musicTrackGraceTicks = 100;
	} else {
		isMusicPlaying = false;
	}
#endif
}

void SoundEngine::stopBackgroundMusic() {
#ifdef _WIN32
	cleanupMFPlayer();
#else
	if (mc && mc->platform()) {
		mc->platform()->stopMusicTrack();
	}
#endif
	if (isMusicPlaying) {
		LOGI("[SoundEngine] Stopped background music.\n");
	}
	isMusicPlaying = false;
	isFadingOut = false;
	musicTrackGraceTicks = 0;
	zeroVolumeTimer = 0;
}

void SoundEngine::setMusicVolume(float volume) {
	lastMusicVolume = volume;
#ifdef _WIN32
	if (g_pMFPlayer) {
		g_pMFPlayer->SetVolume(volume);
	}
#else
	if (mc && mc->platform()) {
		mc->platform()->setMusicVolumeTrack(volume);
	}
#endif
}

bool SoundEngine::isMusicTrackPlaying() {
#ifdef _WIN32
	if (!g_pMFPlayer) return false;
	MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;
	if (SUCCEEDED(g_pMFPlayer->GetState(&state))) {
		return (state == MFP_MEDIAPLAYER_STATE_PLAYING || 
		        state == MFP_MEDIAPLAYER_STATE_PAUSED || 
		        (state != MFP_MEDIAPLAYER_STATE_STOPPED && state != MFP_MEDIAPLAYER_STATE_EMPTY));
	}
	return false;
#else
	if (mc && mc->platform()) {
		return mc->platform()->isMusicTrackPlaying();
	}
	return false;
#endif
}

void SoundEngine::updateMusic() {
	if (!options) return;

	float targetVol = options->getProgressValue(OPTIONS_MUSIC_VOLUME);

	if (targetVol <= 0.001f) {
		if (isMusicPlaying) {
			setMusicVolume(0.0f);
			zeroVolumeTimer++;
			if (zeroVolumeTimer > 600) { // 30 seconds at 0 volume before stopping
				stopBackgroundMusic();
				LOGI("[SoundEngine] Music volume remained at 0 for 30s. Stopping track.\n");
			}
		}
		return;
	}

	zeroVolumeTimer = 0;

	if (isMusicPlaying) {
		// 1. Handle Fade-Out
		if (isFadingOut) {
			if (fadeOutTicks > 0) {
				fadeOutTicks--;
				float factor = (float)fadeOutTicks / (float)fadeTicksMax;
				setMusicVolume(targetVol * factor);
			} else {
				stopBackgroundMusic();
				musicDelayTimer = random.nextInt(400) + 200;
				LOGI("[SoundEngine] Track finished (Fade-Out 5s complete). Next track in %d seconds...\n", musicDelayTimer / 20);
			}
			return;
		}

		// 2. Handle Fade-In
		if (currentFadeTicks < fadeTicksMax) {
			currentFadeTicks++;
			float factor = (float)currentFadeTicks / (float)fadeTicksMax;
			setMusicVolume(targetVol * factor);
		} else if (std::abs(targetVol - lastMusicVolume) > 0.01f) {
			setMusicVolume(targetVol);
		}

		// 3. Track Completion Check
		if (musicTrackGraceTicks > 0) {
			musicTrackGraceTicks--;
		} else if (!isMusicTrackPlaying()) {
			isFadingOut = true;
			fadeOutTicks = fadeTicksMax;
			LOGI("[SoundEngine] Starting 5s Fade-Out for track completion...\n");
		}
	} else {
		if (musicDelayTimer > 0) {
			musicDelayTimer--;
		} else {
			playRandomMusicTrack();
		}
	}
}

void SoundEngine::update( Mob* player, float a )
{
	updateMusic();

	if (/*!loaded || */options->getProgressValue(OPTIONS_SOUND_VOLUME) == 0) return;
	if (player == NULL) return;

	_x = player->xo + (player->x - player->xo) * a;
	_y = player->yo + (player->y - player->yo) * a;
	_z = player->zo + (player->z - player->zo) * a;
	_yRot = player->yRotO + (player->yRot - player->yRotO) * a;

	soundSystem.setListenerAngle(_yRot);
	//soundSystem.setListenerPos(_x, _y, _z); //@note: not used, since we translate all sounds to Player space
}

float SoundEngine::_getVolumeMult( float x, float y, float z )
{
	const float dx = x - _x;
	const float dy = y - _y;
	const float dz = z - _z;
	const float dist = Mth::sqrt(dx*dx + dy*dy + dz*dz);
	const float out =  Mth::clamp(1.1f - dist*_invMaxDistance, -1.0f, 1.0f);
	return out;
}

#if defined(PRE_ANDROID23)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	//volume *= (2.0f * _getVolumeMult(x, y, z))
	if ((volume *= options->getProgressValue(OPTIONS_SOUND_VOLUME)) <= 0) return;

	volume *= _getVolumeMult(x, y, z);
	mc->platform()->playSound(name, volume, pitch);
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->getProgressValue(OPTIONS_SOUND_VOLUME)) <= 0) return;

	//volume *= 2.0f;
	mc->platform()->playSound(name, volume, pitch);
}
#elif defined(__APPLE__)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	if ((volume *= options->getProgressValue(OPTIONS_SOUND_VOLUME)) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, x-_x, y-_y, z-_z, volume, pitch);
	}
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->getProgressValue(OPTIONS_SOUND_VOLUME)) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);
	if (/*!loaded || */options->getProgressValue(OPTIONS_SOUND_VOLUME) == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, 0, 0, 0, volume, pitch);
	}
}
#elif defined(RPI)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {}
#else
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	if ((volume *= options->getProgressValue(OPTIONS_SOUND_VOLUME)) <= 0) return;

	volume = Mth::clamp( volume * _getVolumeMult(x, y, z), 0.0f, 1.0f);
	if (/*!loaded || */options->getProgressValue(OPTIONS_SOUND_VOLUME) == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		float dist = SOUND_DISTANCE;
		if (volume > 1) dist *= volume;
		soundSystem.playAt(sound, x-_x, y-_y, z-_z, volume, pitch);
	}
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->getProgressValue(OPTIONS_SOUND_VOLUME)) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);
	if (/*!loaded || */options->getProgressValue(OPTIONS_SOUND_VOLUME) == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, 0, 0, 0, volume, pitch);
	}
}
#endif
