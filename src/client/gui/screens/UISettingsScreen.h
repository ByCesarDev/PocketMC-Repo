#pragma once
#include "../Screen.h"
#include "../components/Button.h"

class UISettingsScreen : public Screen {
public:
    UISettingsScreen();
    virtual ~UISettingsScreen();
    virtual void init();
    virtual void setupPositions();
    virtual void buttonClicked(Button* button);
    virtual void render(int xm, int ym, float a);
    virtual void keyPressed(int eventKey);
    virtual void removed();

private:
    void updateProfileButtonText();

    Button* btnUIProfile;
    Button* btnSafeArea;
    Button* btnDone;
};
