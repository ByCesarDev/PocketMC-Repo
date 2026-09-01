#include "PauseScreen.h"
#include "StartMenuScreen.h"
#include "../components/ImageButton.h"
#include "../../Minecraft.h"
#include "../../../util/Mth.h"
#include "../../../network/RakNetInstance.h"
#include "../../../network/ServerSideNetworkHandler.h"
#include "client/Options.h"
#include "client/player/AccountManager.h"
#include "client/gui/components/Button.h"
#include "client/gui/screens/OptionsScreen.h"
#include "client/gui/screens/SkindexScreen.h"
#include "client/model/HumanoidModel.h"
#include "client/renderer/TextureData.h"
#include "client/renderer/Textures.h"
#include "client/renderer/GuiShader.h"
#include "../../../locale/I18n.h"

PauseScreen::PauseScreen(bool wasBackPaused)
:	saveStep(0),
	visibleTime(0),
	bContinue(0),
	bQuit(0),
	bOptions(0),
	bDressingRoom(0),
	bQuitAndSaveLocally(0),
	bServerVisibility(0),
//	bThirdPerson(0),
	wasBackPaused(wasBackPaused),
	// bSound(OPTIONS_SOUND_VOLUME, 1, 0),
	bThirdPerson(OPTIONS_THIRD_PERSON_VIEW),
    bHideGui(OPTIONS_HIDEGUI)
{
	ImageDef def;
	def.setSrc(IntRectangle(160, 144, 39, 31));
	def.name = "gui/touchgui.png";
	IntRectangle& defSrc = *def.getSrc();

	def.width = defSrc.w * 0.666667f;
	def.height = defSrc.h * 0.666667f;

	// bSound.setImageDef(def, true);
	defSrc.y += defSrc.h;
	bThirdPerson.setImageDef(def, true);
    bHideGui.setImageDef(def, true);
	//void setImageDef(ImageDef& imageDef, bool setButtonSize);
}

PauseScreen::~PauseScreen() {
	delete bContinue;
	delete bQuit;
	delete bQuitAndSaveLocally;
	delete bServerVisibility;
	delete bOptions;
	delete bDressingRoom;
//	delete bThirdPerson;
}

void PauseScreen::init() {
	bContinue = new Button(1, I18n::get("menu.returnToGame"));
	bOptions = new Button(5, I18n::get("menu.options"));
	bQuit = new Button(2, I18n::get("menu.returnToMenu"));
	bQuitAndSaveLocally = new Button(3, I18n::get("menu.quitAndSaveLocally"));
	bServerVisibility = new Button(4, I18n::get("menu.serverVisible"));
	bDressingRoom = new Button(6, I18n::get("menu.skindex"));

	buttons.push_back(bContinue);
	buttons.push_back(bOptions);
	buttons.push_back(bQuit);
	buttons.push_back(bServerVisibility);
	buttons.push_back(bDressingRoom);
	// bSound.updateImage(&minecraft->options);
	bThirdPerson.updateImage(&minecraft->options);
	bHideGui.updateImage(&minecraft->options);
	// buttons.push_back(&bSound);
	buttons.push_back(&bThirdPerson);
    //buttons.push_back(&bHideGui);

	// If Back wasn't pressed, set up additional items (more than Quit to menu
	// and Back to game) here
    
    #if !defined(APPLE_DEMO_PROMOTION) && !defined(RPI)
	if (true || !wasBackPaused) {
		if (minecraft->raknetInstance) {
			if (minecraft->raknetInstance->isServer()) {
				updateServerVisibilityText();
				bServerVisibility->active = true;
			}
			else {
                #if !defined(DEMO_MODE)
                bServerVisibility->active = false;
				#endif
			}
		}
	}
    #endif
//	buttons.push_back(bThirdPerson);

	for (unsigned int i = 0; i < buttons.size(); ++i) {
		// if (buttons[i] == &bSound) continue;
		if (buttons[i] == &bThirdPerson) continue;
		if (buttons[i] == &bHideGui) continue;
		tabButtons.push_back(buttons[i]);
	}
}

void PauseScreen::setupPositions() {
    saveStep = 0;
	int buttonW = (std::min)(220, (std::max)(140, width / 3));
	int buttonH = 26;
	int gap = 6;
	// Only 4 buttons in the left column (no Dressing Room)
	int totalH = buttonH * 4 + gap * 3;
	int yBase = (height - totalH) / 2;
	if (yBase < 12) yBase = 12;
	int leftX = width / 4 - buttonW / 2;
	if (leftX < 12) leftX = 12;

	bContinue->width = bOptions->width = bQuit->width = buttonW;
	bServerVisibility->width = buttonW;
	bContinue->height = bOptions->height = bQuit->height = buttonH;
	bServerVisibility->height = buttonH;

	bContinue->x = leftX;
	bContinue->y = yBase;

	bOptions->x = leftX;
	bOptions->y = bContinue->y + buttonH + gap;

	bQuit->x = leftX;
	bQuit->y = bOptions->y + buttonH + gap;

#if APPLE_DEMO_PROMOTION
    bQuit->y += 16;
#endif

	bServerVisibility->x = leftX;
	bServerVisibility->y = bQuit->y + buttonH + gap;

	bQuitAndSaveLocally->x = bQuit->x;
	bQuitAndSaveLocally->y = bQuit->y;

	// Dressing Room: placed below the skin model in the right half
	int dressingW = (std::min)(160, buttonW);
	int centerX = (width * 3) / 4;
	// Skin model center Y: centered vertically in right half
	float ss = (std::min)(34.0f, (float)height / 6.0f);
	int modelHalfH = (int)(ss * 2.5f); // approx model height in screen pixels
	int modelTopY  = height / 2 - modelHalfH;
	int modelBotY  = height / 2 + modelHalfH;

	bDressingRoom->width  = dressingW;
	bDressingRoom->height = buttonH;
	bDressingRoom->x = centerX - dressingW / 2;
	bDressingRoom->y = modelBotY + 8;
	if (bDressingRoom->y + buttonH > height - 4)
		bDressingRoom->y = height - buttonH - 4;
}

void PauseScreen::tick() {
	super::tick();
	visibleTime++;
}

void PauseScreen::render(int xm, int ym, float a) {
	renderBackground();

	drawCenteredString(font, I18n::get("menu.gameMenu"), width / 4, 18, 0xffffff);

	super::render(xm, ym, a);

	// --- Right-side skin preview (after UI so it draws on top) ---
	std::string skinPath = minecraft->options.getStringValue(OPTIONS_SKIN);
	if (skinPath.empty() || skinPath == "Default") skinPath = "mob/char.png";

	TextureId skinTexId = minecraft->textures->loadTexture(skinPath);
	int skinW = 64, skinH = 64;
	const TextureData* tdata = minecraft->textures->getTemporaryTextureData(skinTexId);
	if (tdata) { skinW = tdata->w; skinH = tdata->h; }

	float ss    = (std::min)(34.0f, (float)height / 6.0f);
	int centerX = (width * 3) / 4;
	// Place model center at ~40% of height so there's room for username above & button below
	int modelHalfH = (int)(ss * 2.5f);
	int centerY = (height / 2) - modelHalfH / 2;
	if (centerY < modelHalfH + 16) centerY = modelHalfH + 16;

	// Username label just above the model
	std::string localUname = minecraft->options.getStringValue(OPTIONS_USERNAME);
	std::string uname = AccountManager::getDisplayName(localUname);
	if (!uname.empty()) {
		int textW = font->width(uname);
		int labelY = centerY - modelHalfH - 12;
		fill(centerX - textW / 2 - 4, labelY - 2, centerX + textW / 2 + 4, labelY + 10, 0x70000000);
		drawCenteredString(font, uname, centerX, labelY, 0xffffff);
	}

	// Clear depth so the model isn't hidden behind the game world geometry
	glClear(GL_DEPTH_BUFFER_BIT);

	minecraft->textures->bind(skinTexId);
	glEnable2(GL_DEPTH_TEST);
	GuiShader::unbind();
	glPushMatrix();
	glTranslatef((float)centerX, (float)centerY, -100);
	glScalef(-ss, ss, ss);
	glRotatef(180.0f, 0, 1, 0);
	glRotatef(10.0f, 1, 0, 0);
	glRotatef(20.0f, 0, 1, 0);
	// Head follows cursor (same as StartMenuScreen)
	float diffX     = (float)(centerX - xm);
	float diffY     = (float)(centerY  - ym);
	float headYaw   = diffX * 0.5f;
	float headPitch = -diffY * 0.5f;
	if (headYaw   >  45.0f) headYaw   =  45.0f;
	if (headYaw   < -45.0f) headYaw   = -45.0f;
	if (headPitch >  45.0f) headPitch =  45.0f;
	if (headPitch < -45.0f) headPitch = -45.0f;
	glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);
	bool isSlim = (minecraft && minecraft->options.getStringValue(OPTIONS_SKIN_MODEL) == "slim");
	HumanoidModel skinModel(0.0f, 0.0f, skinW, skinH, isSlim);
	skinModel.render(nullptr, 0, 0, 0, headYaw, headPitch, 0.0625f);
	glPopMatrix();
	glDisable2(GL_DEPTH_TEST);
	GuiShader::bind();
}

void PauseScreen::buttonClicked(Button* button) {
	if (button->id == bContinue->id) {
		minecraft->setScreen(NULL);
		//minecraft->grabMouse();
	}
    if (button->id == bQuit->id) {
		minecraft->leaveGame();
    }
	if (button->id == bQuitAndSaveLocally->id) {
		minecraft->leaveGame(true);
	}
	if (button->id == bOptions->id) {
		minecraft->setScreen(new OptionsScreen());
	}
	if (button->id == bDressingRoom->id) {
		minecraft->setScreen(new SkindexScreen());
	}
	if (button->id == bServerVisibility->id) {
		if (minecraft->raknetInstance && minecraft->netCallback && minecraft->raknetInstance->isServer()) {
			ServerSideNetworkHandler* ss = (ServerSideNetworkHandler*) minecraft->netCallback;
			bool allows = !ss->allowsIncomingConnections();
			ss->allowIncomingConnections(allows);

			updateServerVisibilityText();
		}
	}

	if (button->id == OptionButton::ButtonId) {
		((OptionButton*)button)->toggle(&minecraft->options);
	}

	//if (button->id == bThirdPerson->id) {
	//	minecraft->options.thirdPersonView = !minecraft->options.thirdPersonView;
	//}
}

void PauseScreen::updateServerVisibilityText()
{
	if (!minecraft->raknetInstance || !minecraft->raknetInstance->isServer())
		return;

	ServerSideNetworkHandler* ss = (ServerSideNetworkHandler*) minecraft->netCallback;
	bServerVisibility->msg = ss->allowsIncomingConnections()?
		I18n::get("menu.serverVisible")
	:   I18n::get("menu.serverInvisible");
}
