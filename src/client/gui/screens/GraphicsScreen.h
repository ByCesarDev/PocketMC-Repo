#pragma once
#include "../Screen.h"
#include "../components/Button.h"
#include "../components/OptionsGroup.h"
class GraphicsScreen : public Screen {
public:
    GraphicsScreen();
    virtual ~GraphicsScreen();
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
	virtual void removed();
private:
    Button* btnDone;
    Button* btnPanorama;
    OptionsGroup* group;
};
