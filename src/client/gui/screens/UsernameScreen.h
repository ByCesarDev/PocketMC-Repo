#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__UsernameScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__UsernameScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "client/gui/components/TextBox.h"
#include <string>

class UsernameScreen : public Screen
{
    typedef Screen super;
public:
    UsernameScreen();
    virtual ~UsernameScreen();

    virtual void init() override;
    virtual void setupPositions() override;
    virtual void render(int xm, int ym, float a) override;
    virtual void tick() override;

    virtual bool isPauseScreen() override { return false; }

    virtual void keyPressed(int eventKey) override;
    virtual void mouseClicked(int x, int y, int buttonNum) override;
    virtual bool handleBackEvent(bool isDown) override;
    virtual void removed() override;

protected:
    virtual void buttonClicked(Button* button) override;

private:
    Button _btnBack;
    Button _btnContinue;
    TextBox tUsername;

    bool _hasExistingName;

    // Layout coordinates
    int _panelX, _panelY, _panelW, _panelH;

    // Helpers
    void drawPanel(int x, int y, int w, int h, unsigned int bg, unsigned int border);
    void drawScaledString(const std::string& str, float x, float y, int color, float scale = 0.8f);
    void drawIconTexture(const std::string& path, int x, int y, int w, int h, unsigned int color = 0xffffffff);
    void drawRuleItem(const std::string& text, int x, int y, bool valid);

    // Validation
    bool isLengthValid() const;
    bool areCharsValid() const;
    bool isStartLetterValid() const;
    bool isAllValid() const;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__UsernameScreen_H__*/
