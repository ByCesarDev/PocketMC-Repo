#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ProfileScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ProfileScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include <string>
#include <vector>

class ProfileScreen : public Screen {
private:
    Button _btnBack;
    Button _btnClose;
    Button _btnAction;              // "Vincular Cuenta PocketMC >" or "Administrar cuenta >"
    Button _btnSignOut;             // "Cerrar sesion"
    Button _btnCopyId;              // "Copiar ID"
    Button _btnEditProfileWeb;      // "Editar perfil en la web >"
    Button _btnChangePasswordWeb;   // "Cambiar contrasena >"
    Button _btnEditLocalName;       // "Editar nombre local >"

    int _selectedTab;               // 0 = Resumen, 1 = Cuenta, 2 = Identidad, 3 = Seguridad
    int _copiedFeedbackTicks;       // Visual countdown when ID copied

    // Panel bounds
    int _leftPanelX, _leftPanelY, _leftPanelW, _leftPanelH;
    int _rightPanelX, _rightPanelY, _rightPanelW, _rightPanelH;

    // Helpers
    void drawPanel(int x, int y, int w, int h, unsigned int bg, unsigned int border);
    void drawNavTab(int x, int y, int w, int h, int index, const std::string& iconPath, const std::string& title, const std::string& subtitle, bool isSelected, int xm, int ym);
    void drawSkinBust(int x, int y, int size);
    void drawSkinFace(int x, int y, int size);
    void drawIconTexture(const std::string& path, int x, int y, int w, int h, unsigned int color = 0xffffffff);
    void drawInfoCard(int x, int y, int w, int h, const std::string& iconPath, const std::string& label, const std::string& val, int valColor);
    void drawActionCard(int x, int y, int w, int h, const std::string& iconPath, const std::string& title, const std::string& subtitle, int xm, int ym);
    void copyToClipboard(const std::string& text);
    std::string maskEmail(const std::string& email);

    // Tab Renderers
    void renderTabResumen(int xm, int ym);
    void renderTabCuenta(int xm, int ym);
    void renderTabIdentidad(int xm, int ym);
    void renderTabSeguridad(int xm, int ym);

public:
    ProfileScreen();
    virtual ~ProfileScreen();

    virtual void init() override;
    virtual void setupPositions();
    virtual void render(int xm, int ym, float a) override;
    virtual void tick() override;
    virtual void buttonClicked(Button* button) override;
    virtual void mouseClicked(int x, int y, int buttonNum) override;
    virtual void keyPressed(int eventKey) override;
    virtual bool handleBackEvent(bool isDown) override;
};

#endif // NET_MINECRAFT_CLIENT_GUI_SCREENS__ProfileScreen_H__
