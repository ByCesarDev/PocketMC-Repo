#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_RENAMESKINSCREEN_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_RENAMESKINSCREEN_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "client/gui/components/TextBox.h"
#include <string>

class RenameSkinScreen : public Screen
{
	typedef Screen super;
public:
	RenameSkinScreen(const std::string& skinPath, int packIndex = -1, int skinIndex = -1);
	virtual ~RenameSkinScreen();
	
	virtual void init() override;
	virtual void setupPositions() override;
	virtual void render(int xm, int ym, float a) override;
	virtual void tick() override;
	virtual void keyPressed(int eventKey) override;
	virtual void removed() override;

protected:
	virtual void buttonClicked(Button* button) override;

private:
	Button _btnDone;
	Button _btnCancel;
	TextBox tName;
	std::string _skinPath;
	int _packIndex;
	int _skinIndex;
};

#endif
