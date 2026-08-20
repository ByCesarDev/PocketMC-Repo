#pragma once
#include "../Screen.h"
#include "../components/Button.h"
#include "../components/OptionsGroup.h"
class ControlsScreen : public Screen {
public:
    ControlsScreen();
    virtual ~ControlsScreen();
    virtual void init();
    virtual void setupPositions();
    virtual void buttonClicked(Button* button);
    virtual void render(int xm, int ym, float a);
    virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);
    virtual void mouseWheel(int dx, int dy, int xm, int ym);
    virtual void renderBackground();
    virtual void keyPressed(int eventKey);
	virtual void tick();
private:
    Button* btnDone;
    Button* btnMouse;
    Button* btnKeyboard;
    OptionsGroup* group;
};
