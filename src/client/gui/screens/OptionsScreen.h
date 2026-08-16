#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"

class OptionsScreen : public Screen
{
public:
	OptionsScreen();
	virtual ~OptionsScreen();

	virtual void init();
	virtual void setupPositions();
	virtual void buttonClicked(Button* button);
	virtual void render(int xm, int ym, float a);
	virtual void tick();
	virtual void keyPressed(int eventKey);
	virtual void removed();
    
    virtual void renderBackground();

private:
	Button* btnDone;
	Button* btnSkin;
	Button* btnAudio;
	Button* btnGraphics;
	Button* btnControls;
	Button* btnLanguage;
	Button* btnChat;
	Button* btnResourcePacks;
	Button* btnAccessibility;
};

#endif