#include "ProgressScreen.h"
#include "DisconnectionScreen.h"
#include "../Gui.h"
#include "../Font.h"
#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../../SharedConstants.h"
#include "../../renderer/Textures.h"
#include "../../../platform/log.h"
#include "../../../locale/I18n.h"

ProgressScreen::ProgressScreen()
:	ticks(0),
	customTitle(""),
	customStatus(""),
	customBgTexture("")
{
}

ProgressScreen::ProgressScreen(const std::string& title, const std::string& status, const std::string& bgTexture)
:	ticks(0),
	customTitle(title),
	customStatus(status),
	customBgTexture(bgTexture)
{
}

void ProgressScreen::render( int xm, int ym, float a )
{
	LOGI("[ProgressScreen::render] Called. isLevelGenerated=%d, width=%d, height=%d, progressStagePercentage=%d\n", 
		minecraft ? minecraft->isLevelGenerated() : -1, width, height, minecraft ? minecraft->progressStagePercentage : -2);
	if (minecraft->isLevelGenerated()) {
		LOGI("[ProgressScreen::render] Level is generated. Setting screen to NULL.\n");
		minecraft->setScreen(NULL);
		return;
	}

	Tesselator& t = Tesselator::instance;

	if (!customBgTexture.empty()) {
		glDisable2(GL_DEPTH_TEST);
		glDisable2(GL_ALPHA_TEST);
		glEnable2(GL_TEXTURE_2D);

		minecraft->textures->loadAndBindTexture(customBgTexture);

		const float s = 32.0f;
		t.begin();
		t.color(0xffffff);
		t.vertexUV(0.0f, (float)height, 0.0f, 0.0f, height / s);
		t.vertexUV((float)width, (float)height, 0.0f, width / s, height / s);
		t.vertexUV((float)width, 0.0f, 0.0f, width / s, 0.0f);
		t.vertexUV(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		t.draw();
	} else {
		renderBackground();
	}

	int i = minecraft->progressStagePercentage;

	if (i >= 0) {
		int w = 100;
		int h = 2;
		int x = width / 2 - w / 2;
		int y = height / 2 + 16;

		//printf("%d, %d - %d, %d\n", x, y, x + w, y + h);

		glDisable2(GL_TEXTURE_2D);
		t.begin();
		t.color(0x808080);
		t.vertex((float)x, (float)y, 0);
		t.vertex((float)x, (float)(y + h), 0);
		t.vertex((float)(x + w), (float)(y + h), 0);
		t.vertex((float)(x + w), (float)y, 0);

		t.color(0x80ff80);
		t.vertex((float)x, (float)y, 0);
		t.vertex((float)x, (float)(y + h), 0);
		t.vertex((float)(x + i), (float)(y + h), 0);
		t.vertex((float)(x + i), (float)y, 0);
		t.draw();
		glEnable2(GL_TEXTURE_2D);
	}

    glEnable2(GL_BLEND);

	std::string displayTitle = customTitle;
	if (displayTitle.empty()) {
		std::string tr;
		if (I18n::get("progressScreen.title", tr)) {
			displayTitle = tr;
		} else {
			displayTitle = "Generating world";
		}
	}

	std::string displayStatus = customStatus;
	if (displayStatus.empty()) {
		int statusId = minecraft->getProgressStatusId();
		std::string key = "progressScreen.message.";
		if (statusId == 0) key += "locating";
		else if (statusId == 1) key += "building";
		else if (statusId == 2) key += "preparing";
		else if (statusId == 3) key += "saving";
		else key += "generic";

		std::string tr;
		if (I18n::get(key, tr)) {
			displayStatus = tr;
		} else {
			displayStatus = minecraft->getProgressMessage();
		}
	}

	minecraft->font->drawShadow(displayTitle, (float)((width - minecraft->font->width(displayTitle)) / 2), (float)(height / 2 - 4 - 16), 0xffffff);

	const int progressWidth = minecraft->font->width(displayStatus);
	const int progressLeft  = (width - progressWidth) / 2;
	const int progressY = height / 2 - 4 + 8;
	minecraft->font->drawShadow(displayStatus, (float)progressLeft, (float)progressY, 0xffffff);

#if APPLE_DEMO_PROMOTION
	drawCenteredString(minecraft->font, "This demonstration version", width/2, progressY + 36, 0xffffff);
    drawCenteredString(minecraft->font, "does not allow saving games", width/2, progressY + 46, 0xffffff);
#endif
    
	// If we're locating the server, show our famous spinner!
	bool isLocating = (minecraft->getProgressStatusId() == 0);
	if (isLocating) {
		const int spinnerX = progressLeft + progressWidth + 6;
		static const char* spinnerTexts[] = {"-", "\\", "|", "/"};
		int n = ((int)(5.5f * getTimeS()) % 4);
		drawCenteredString(minecraft->font, spinnerTexts[n], spinnerX, progressY, 0xffffffff);
	}

    glDisable2(GL_BLEND);
	sleepMs(50);
}

bool ProgressScreen::isInGameScreen() { return false; }

void ProgressScreen::tick() {
	// After 10 seconds of not connecting -> write an error message and go back
	if (++ticks == 10 * SharedConstants::TicksPerSecond && minecraft->getProgressStatusId() == 0) {
		minecraft->setScreen( new DisconnectionScreen("Could not connect to server. Try again.") );
	}
}
