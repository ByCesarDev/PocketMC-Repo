#pragma once
#include "../Screen.h"
#include "../components/Button.h"
#include "../components/ScrolledSelectionList.h"
#include "../panoramas/PanoramaRegistry.h"
#include <vector>

class PanoramaSelectionList;

class PanoramaSelectScreen : public Screen {
public:
    PanoramaSelectScreen();
    virtual ~PanoramaSelectScreen();

    virtual void init();
    virtual void setupPositions();
    virtual void buttonClicked(Button* button);
    virtual void render(int xm, int ym, float a);
    virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);
    virtual void mouseWheel(int dx, int dy, int xm, int ym);
    virtual void keyPressed(int eventKey);
    virtual void removed();

    void setSelectedPanorama(int index);

    friend class PanoramaSelectionList;

private:
    Button* btnDone;
    Button* btnCancel;
    Button* btnDefault;

    std::vector<PanoramaInfo> panoramaList;
    int selectedIndex;
    int activeIndex;

    PanoramaSelectionList* scrolledList;

    // Responsive Layout Bounds
    int leftX, leftY, leftW, leftH;
    int rightX, rightY, rightW, rightH;
};
