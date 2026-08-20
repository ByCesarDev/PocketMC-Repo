#include "StartMenuScreen.h"
#include <algorithm>
#include "UsernameScreen.h"
#include "SelectWorldScreen.h"
#include "ProgressScreen.h"
#include "JoinGameScreen.h"
#include "OptionsScreen.h"
#include "PauseScreen.h"
#include "SkindexScreen.h"
#include "ModListScreen.h"
#include "PrerenderTilesScreen.h" // test button
#include "../components/ImageButton.h"

#include "../../../util/Mth.h"

#include "../Font.h"
#include "../components/ScrolledSelectionList.h"

#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../../AppPlatform.h"
#include "../../../LicenseCodes.h"
#include "SimpleChooseLevelScreen.h"
#include "../../renderer/Textures.h"
#include "../../renderer/TextureData.h"
#include "../../renderer/GuiShader.h"
#include "../../model/HumanoidModel.h"
#include "../../../SharedConstants.h"
#include "../../../locale/I18n.h"

// Some kind of default settings, might be overridden in ::init
StartMenuScreen::StartMenuScreen()
:	bHost(    2, 0, 0, 160, 24, I18n::get("menu.startGame")),
	bJoin(    3, 0, 0, 160, 24, I18n::get("menu.joinGame")),
	bOptions( 4, 0, 0, 160, 24, I18n::get("menu.options")),
	bProfile( 6, 0, 0, 60, 24, I18n::get("menu.profile")),
	bSkindex(7, 0, 0, 60, 24, I18n::get("menu.skindex")),
	bMods(8, 0, 0, 60, 24, I18n::get("menu.mods")),
	bQuit(    5, 0, 0, 160, 24, I18n::get("menu.quit")),
	panoramaTicks(0)
{
}

StartMenuScreen::~StartMenuScreen()
{
}

void StartMenuScreen::init()
{
	bJoin.active = bHost.active = bOptions.active = true;

	if (minecraft->options.getStringValue(OPTIONS_USERNAME).empty()) {
		return; // tick() will redirect to UsernameScreen
	}

	buttons.push_back(&bHost);
	buttons.push_back(&bOptions);
	buttons.push_back(&bJoin);
	buttons.push_back(&bQuit);
	buttons.push_back(&bProfile);
	buttons.push_back(&bSkindex);
	buttons.push_back(&bMods);

	tabButtons.push_back(&bHost);
	tabButtons.push_back(&bOptions);
	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bQuit);
	tabButtons.push_back(&bProfile);
	tabButtons.push_back(&bSkindex);
	tabButtons.push_back(&bMods);

	copyright = "\xffMojang AB";

	std::string versionString = Common::getGameVersionString();

	std::string _username = minecraft->options.getStringValue(OPTIONS_USERNAME);
	if (_username.empty()) _username = "unknown";
	username = "Username: " + _username;

	#ifdef DEMO_MODE
	#ifdef __APPLE__
		version = versionString + " (Lite)";
	#else
		version = versionString + " (Demo)";
	#endif
	#else
		#ifdef RPI
			version = "v0.1.1 alpha";
		#else
			version = versionString;
		#endif
	#endif
}

void StartMenuScreen::setupPositions() {
	// Full-width buttons
	int fullW = (std::max)(160, (std::min)(200, width / 2));
	int btnH  = 24;
	int gap   = 4;

	bHost.width  = fullW;  bHost.height  = btnH;
	bJoin.width  = fullW;  bJoin.height  = btnH;
	bQuit.width  = fullW;  bQuit.height  = btnH;

	// Half-width buttons for the Mods|Options row
	int halfGap = 4;
	int halfW   = (fullW - halfGap) / 2;
	bMods.width    = halfW;  bMods.height    = btnH;
	bOptions.width = halfW;  bOptions.width  = fullW - halfW - halfGap;
	bOptions.height = btnH;

	// Layout: 4 rows (Singleplayer, Multiplayer, Mods+Options, Quit)
	int totalH = btnH * 4 + gap * 3;
	int yBase  = (height - totalH) / 2;
	int cx     = width / 2;

	// Row 1 â€“ Singleplayer
	bHost.x = cx - fullW / 2;
	bHost.y = yBase;

	// Row 2 â€“ Multiplayer
	bJoin.x = cx - fullW / 2;
	bJoin.y = bHost.y + btnH + gap;

	// Row 3 â€“ Mods (left) | Options (right), same combined width as full-width buttons
	bMods.x    = cx - fullW / 2;
	bMods.y    = bJoin.y + btnH + gap;
	bOptions.x = bMods.x + bMods.width + halfGap;
	bOptions.y = bMods.y;

	// Row 4 â€“ Save and Quit
	bQuit.x = cx - fullW / 2;
	bQuit.y = bMods.y + btnH + gap;

	// Bottom-corner buttons (skin widget row)
	bProfile.width = (std::max)(60, font->width(bProfile.msg) + 16);
	bSkindex.width = (std::max)(60, font->width(bSkindex.msg) + 16);
	bProfile.height = 20;  bSkindex.height = 20;
	bProfile.x = 2;
	bProfile.y = height - bProfile.height - 14;
	bSkindex.x = width - bSkindex.width - 2;
	bSkindex.y = height - bSkindex.height - 14;
}

void StartMenuScreen::tick() {
	panoramaTicks++;
}

void StartMenuScreen::buttonClicked(Button* button) {

	if (button->id == bHost.id)
	{
        #if defined(DEMO_MODE) || defined(APPLE_DEMO_PROMOTION)
			minecraft->setScreen( new SimpleChooseLevelScreen("_DemoLevel") );
		#else
			minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
		#endif
	}
	if (button->id == bJoin.id)
	{
		minecraft->locateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
	if (button->id == bOptions.id)
	{
		minecraft->setScreen(new OptionsScreen());
	}
	if (button->id == bProfile.id)
	{
		minecraft->setScreen(new UsernameScreen());
	}
	if (button->id == bSkindex.id)
	{
		minecraft->setScreen(new SkindexScreen());
	}
	if (button->id == bMods.id)
	{
		minecraft->setScreen(new ModListScreen());
	}
	if (button->id == bQuit.id)
	{
		minecraft->quit();
	}
}

bool StartMenuScreen::isInGameScreen() { return false; }

void StartMenuScreen::render( int xm, int ym, float a )
{
	float panoTime = getPanoramaTime();
	renderPanorama((int)panoTime, panoTime - (int)panoTime);

#if defined(RPI)
	TextureId id = minecraft->textures->loadTexture("gui/pi_title.png");
#else
	TextureId id = minecraft->textures->loadTexture("gui/title.png");
#endif
	const TextureData* data = minecraft->textures->getTemporaryTextureData(id);

	if (data) {
		minecraft->textures->bind(id);

		const float maxW = 274.0f;
		const float scale = Mth::Min((float)width * 0.85f, maxW) / maxW;
		const float w = maxW * scale;
		const float h = w * ((float)data->h / (float)data->w);
		
		const float x = (float)width / 2;
		const float y = height / 16;

		// Render title text
		Tesselator& t = Tesselator::instance;
		glColor4f2(1, 1, 1, 1);
		t.begin();
		t.vertexUV(x - w/2, y + h, blitOffset, 0, 1);
		t.vertexUV(x + w/2, y + h, blitOffset, 1, 1);
		t.vertexUV(x + w/2, y, blitOffset, 1, 0);
		t.vertexUV(x - w/2, y, blitOffset, 0, 0);
		t.draw();
	}

#if defined(RPI)
	if (Textures::isTextureIdValid(minecraft->textures->loadAndBindTexture("gui/logo/raknet_high_72.png")))
		blit(0, height - 12, 0, 0, 43, 12, 256, 72+72);
#endif

	drawString(font, version, width - font->width(version) - 2, height - 10, 0xffcccccc);
	drawString(font, copyright, 2, height - 10, 0xffffff);
	
	Screen::render(xm, ym, a);

	// Draw skin preview above bSkindex button
	{
		std::string skinPath = minecraft->options.getStringValue(OPTIONS_SKIN);
		if (skinPath.empty() || skinPath == "Default") skinPath = "mob/char.png";

		TextureId skinTexId = minecraft->textures->loadTexture(skinPath);

		// Detect skin dimensions for correct UV layout
		int skinW = 64, skinH = 64;
		const TextureData* tdata = minecraft->textures->getTemporaryTextureData(skinTexId);
		if (tdata) { skinW = tdata->w; skinH = tdata->h; }

		// Position: just above bSkindex button, small preview
		int centerX = bSkindex.x + bSkindex.width / 2;
		int centerY = bSkindex.y - 45;

		// Username label above the skin (with transparent dark background)
		std::string uname = minecraft->options.getStringValue(OPTIONS_USERNAME);
		if (!uname.empty()) {
			int textW = font->width(uname);
			fill(centerX - textW / 2 - 3, centerY - 35 - 2, centerX + textW / 2 + 3, centerY - 35 + 10, 0x50000000);
			drawCenteredString(font, uname, centerX, centerY - 35, 0xffffff);
		}

		minecraft->textures->bind(skinTexId);

		glEnable2(GL_DEPTH_TEST);
		GuiShader::unbind();
		glPushMatrix();
		glTranslatef((float)centerX, (float)centerY, -100);
		float ss = 25.0f; // Slightly larger
		glScalef(-ss, ss, ss);
		glRotatef(180.0f, 0, 1, 0);
		glRotatef(10.0f, 1, 0, 0); // Pitch
		glRotatef(20.0f, 0, 1, 0); // Yaw for 3D effect
#if defined(ANDROID)
		float headYaw = 0.0f;
		float headPitch = 0.0f;
#else
		float diffX = (float)(centerX - xm);
		float diffY = (float)((centerY - 35) - ym);
		float headYaw = diffX * 0.5f;
		float headPitch = -diffY * 0.5f;
		if (headYaw > 45.0f) headYaw = 45.0f;
		if (headYaw < -45.0f) headYaw = -45.0f;
		if (headPitch > 45.0f) headPitch = 45.0f;
		if (headPitch < -45.0f) headPitch = -45.0f;
#endif

		glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);
		bool isSlim = (minecraft && minecraft->options.getStringValue(OPTIONS_SKIN_MODEL) == "slim");
		HumanoidModel skinModel(0.0f, 0.0f, skinW, skinH, isSlim);
		skinModel.render(nullptr, 0, 0, 0, headYaw, headPitch, 0.0625f);
		glPopMatrix();
		glDisable2(GL_DEPTH_TEST);
		GuiShader::bind();
	}
}

bool StartMenuScreen::handleBackEvent( bool isDown ) {
	minecraft->quit();
	return true;
}
