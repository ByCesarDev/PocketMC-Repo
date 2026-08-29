#pragma once
#include "../Screen.h"
#include "../components/Button.h"

class SafeAreaScreen : public Screen {
public:
    SafeAreaScreen();
    virtual ~SafeAreaScreen();

    virtual void init();
    virtual void setupPositions();
    virtual void buttonClicked(Button* button);
    virtual void render(int xm, int ym, float a);
    virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);
    virtual void keyPressed(int eventKey);
    virtual void removed();

private:
    void drawCornerGuides(int marginX, int marginY);

    Button* btnConfirm;
    bool isDraggingSlider;

    // Modal dialog geometry
    int dialogX;
    int dialogY;
    int dialogW;
    int dialogH;

    // Slider geometry
    int sliderX;
    int sliderY;
    int sliderW;
    int sliderH;
};
