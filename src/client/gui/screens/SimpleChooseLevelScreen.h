#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__

#include "ChooseLevelScreen.h"
#include "../components/TextBox.h"
#include "../components/Button.h"    // for Touch::THeader
class Button;
class ImageButton;

class SimpleChooseLevelScreen: public ChooseLevelScreen
{
public:
	SimpleChooseLevelScreen(const std::string& levelName);

	virtual ~SimpleChooseLevelScreen();

	void init();
	void setupPositions();
	void tick();

	void render(int xm, int ym, float a);

	void buttonClicked(Button* button);
	bool handleBackEvent(bool isDown);
	virtual void keyPressed(int eventKey);
	virtual void mouseClicked(int x, int y, int buttonNum);

private:
	Touch::THeader* bHeader;
	Button* bGamemode;
	Button* bCheats;
	Button* bWorldType;
	Button* bExperimental;
	Button* bWoodland; // <-- Woodland mod button
	ImageButton* bBack;
	Button* bCreate;
	bool hasChosen;

	std::string levelName;
	int gamemode;
	bool cheatsEnabled;
	int worldType;
	bool experimental;
	bool woodlandEnabled; // <-- Woodland state

	TextBox tLevelName;
	TextBox tSeed;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__*/