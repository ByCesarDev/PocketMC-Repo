#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ProgressScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ProgressScreen_H__

#include "../Screen.h"
#include <string>

class ProgressScreen: public Screen
{
public:
	ProgressScreen();
	ProgressScreen(const std::string& title, const std::string& status = "", const std::string& bgTexture = "");

	void setTitle(const std::string& title) { customTitle = title; }
	void setStatus(const std::string& status) { customStatus = status; }
	void setBackgroundTexture(const std::string& bgTexture) { customBgTexture = bgTexture; }

	void render(int xm, int ym, float a);
	bool isInGameScreen();
	virtual bool renderGameBehind() override { return false; }

	virtual void keyPressed(int eventKey) {}

	void tick();
private:
	int ticks;
	std::string customTitle;
	std::string customStatus;
	std::string customBgTexture;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ProgressScreen_H__*/
