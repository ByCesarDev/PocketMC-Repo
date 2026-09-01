#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__LinkAccountScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__LinkAccountScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "client/gui/components/TextBox.h"
#include <string>

class LinkAccountScreen : public Screen {
private:
    Button _btnSignIn;
    Button _btnCreateAccount;
    Button _btnBack;
    
    TextBox tUsername;
    TextBox tPassword;

    std::string _statusMsg;
    bool _isError;

public:
    LinkAccountScreen();
    virtual ~LinkAccountScreen();

    virtual void init() override;
    virtual void setupPositions() override;
    virtual void render(int xm, int ym, float a) override;
    virtual void buttonClicked(Button* button) override;
    virtual void keyPressed(int eventKey) override;
};

#endif // NET_MINECRAFT_CLIENT_GUI_SCREENS__LinkAccountScreen_H__
