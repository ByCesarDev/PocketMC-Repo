#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ProfileScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ProfileScreen_H__

#include "../Screen.h"
#include "../components/Button.h"

class ProfileScreen : public Screen {
private:
    Button _btnAction;
    Button _btnSecondary;
    Button _btnBack;

public:
    ProfileScreen();
    virtual ~ProfileScreen();

    virtual void init() override;
    virtual void setupPositions();
    virtual void render(int xm, int ym, float a) override;
    virtual void buttonClicked(Button* button) override;
    virtual void keyPressed(int eventKey) override;
};

#endif // NET_MINECRAFT_CLIENT_GUI_SCREENS__ProfileScreen_H__
