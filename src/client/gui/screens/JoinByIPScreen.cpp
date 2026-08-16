#include "JoinByIPScreen.h"

#include "JoinGameScreen.h"
#include "StartMenuScreen.h"
#include "ProgressScreen.h"
#include "../Font.h"
#include "../../../network/RakNetInstance.h"
#include "client/Options.h"
#include "client/gui/Screen.h"
#include "client/gui/components/TextBox.h"
#include "network/ClientSideNetworkHandler.h"
#include "../../../locale/I18n.h"

JoinByIPScreen::JoinByIPScreen() :
    tIP(0, ""),
    bHeader(1, I18n::get("selectServer.direct")),
	bJoin(  2, I18n::get("selectServer.select")),
	bBack(  3, I18n::get("gui.cancel"))
{
	bJoin.active = false;
}

JoinByIPScreen::~JoinByIPScreen()
{
}

void JoinByIPScreen::buttonClicked(Button* button)
{
	if (button->id == bJoin.id)
	{            
        minecraft->isLookingForMultiplayer = true;
	    minecraft->netCallback = new ClientSideNetworkHandler(minecraft, minecraft->raknetInstance);

        minecraft->joinMultiplayerFromString(tIP.text);
        {
			minecraft->options.set(OPTIONS_LAST_IP, tIP.text);
            bJoin.active = false;
            bBack.active = false;
            minecraft->setScreen(new ProgressScreen());
        }
	}
	if (button->id == bBack.id)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
}

bool JoinByIPScreen::handleBackEvent(bool isDown)
{
	if (!isDown)
	{
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
	return true;
}

void JoinByIPScreen::tick()
{
	Screen::tick();
	bJoin.active = !tIP.text.empty();
}

void JoinByIPScreen::init()
{
	buttons.push_back(&bJoin);
	buttons.push_back(&bBack);
    
    textBoxes.push_back(&tIP);
#ifdef ANDROID
	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bBack);
#endif

	tIP.text = minecraft->options.getStringValue(OPTIONS_LAST_IP);
}

void JoinByIPScreen::setupPositions() {
    int yBase = height / 4 + 72 + 12;

    bJoin.width = 100;
    bJoin.height = 20;
    bJoin.x = width / 2 - 100 - 2;
    bJoin.y = yBase;

    bBack.width = 100;
    bBack.height = 20;
    bBack.x = width / 2 + 2;
    bBack.y = yBase;

    tIP.width = 200;
    tIP.height = 20;
    tIP.x = width / 2 - 100;
    tIP.y = height / 4 + 36;
}

void JoinByIPScreen::render( int xm, int ym, float a )
{
	renderBackground();
	
	drawCenteredString(minecraft->font, I18n::get("selectServer.direct"), width / 2, 17, 0xffffffff);
	drawString(minecraft->font, I18n::get("addServer.enterIp"), width / 2 - 100, height / 4 + 36 - 10, 0xa0a0a0);

	Screen::render(xm, ym, a);
}

void JoinByIPScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
        return;
    }
    // let base class handle navigation and text box keys
    Screen::keyPressed(eventKey);
}
