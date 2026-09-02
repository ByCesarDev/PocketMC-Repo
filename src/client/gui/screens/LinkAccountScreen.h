#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__LinkAccountScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__LinkAccountScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "client/gui/components/TextBox.h"
#include <string>

class LinkAccountScreen : public Screen {
private:
    Button _btnBack;
    Button _btnSignIn;
    
    TextBox tUsername;
    TextBox tPassword;

    bool _rememberMe;
    bool _isAuthenticating;
    std::string _statusMsg;
    bool _isError;

    // Layout coordinates
    int _panelX, _panelY, _panelW, _panelH;
    int _eyeX, _eyeY, _eyeW, _eyeH;
    int _cbX, _cbY, _cbSize;
    int _fpX, _fpY, _fpW, _fpH;
    int _createBtnX, _createBtnY, _createBtnW, _createBtnH;

    // Helpers
    void drawPanel(int x, int y, int w, int h, unsigned int bg, unsigned int border);
    void drawScaledString(const std::string& str, float x, float y, int color, float scale = 0.8f);
    void drawIconTexture(const std::string& path, int x, int y, int w, int h, unsigned int color = 0xffffffff);

public:
    LinkAccountScreen();
    virtual ~LinkAccountScreen();

    virtual void init() override;
    virtual void tick() override;
    virtual void setupPositions() override;
    virtual void render(int xm, int ym, float a) override;
    virtual void buttonClicked(Button* button) override;
    virtual void mouseClicked(int x, int y, int buttonNum) override;
    virtual void keyPressed(int eventKey) override;
    virtual bool handleBackEvent(bool isDown) override;
};

#endif // NET_MINECRAFT_CLIENT_GUI_SCREENS__LinkAccountScreen_H__
