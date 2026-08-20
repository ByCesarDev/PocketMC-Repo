#include "LanguageScreen.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "../../renderer/Tesselator.h"
#include "../components/ScrolledSelectionList.h"
#include "locale/I18n.h"

struct LanguageInfo {
    std::string code;
    std::string name;
};

class LanguageSelectionList : public ScrolledSelectionList {
private:
    LanguageScreen* parent;
    std::vector<LanguageInfo> languages;

public:
    LanguageSelectionList(LanguageScreen* parent, Minecraft* mc, int width, int height, int y0, int y1, int itemHeight)
        : ScrolledSelectionList(mc, width, height, y0, y1, itemHeight), parent(parent) {
        
        // Available language codes — names are loaded dynamically from their lang files
        std::vector<std::string> codes = {"en_US", "es_ES"};
        
        // Save current lang index so we can restore after probing
        OptionInt* langOpt = dynamic_cast<OptionInt*>(mc->options.getOpt(OPTIONS_LANGUAGE));
        int savedLangIdx = langOpt ? langOpt->get() : 0;
        
        for (const std::string& code : codes) {
            // Temporarily load this language to read its name
            I18n::loadLanguage(mc->platform(), code);
            std::string langName = I18n::get("language.name");
            std::string langRegion = I18n::get("language.region");
            std::string displayName = langName;
            if (!langRegion.empty() && langRegion.find('<') == std::string::npos)
                displayName += " (" + langRegion + ")";
            languages.push_back({code, displayName});
        }
        
        // Restore the original language
        if (savedLangIdx >= 0 && savedLangIdx < (int)codes.size())
            I18n::loadLanguage(mc->platform(), codes[savedLangIdx]);
        else
            I18n::loadLanguage(mc->platform(), "en_US");
    }

protected:
    virtual int getNumberOfItems() {
        return languages.size();
    }

    virtual void selectItem(int item, bool doubleClick) {
        minecraft->options.set(OPTIONS_LANGUAGE, item); // Sets the option
        I18n::loadLanguage(minecraft->platform(), languages[item].code); // Reloads strings
    }

    virtual bool isSelectedItem(int item) {
        OptionInt* opt = dynamic_cast<OptionInt*>(minecraft->options.getOpt(OPTIONS_LANGUAGE));
        return opt && opt->get() == item;
    }

    virtual int getMaxPosition() {
        return getNumberOfItems() * itemHeight;
    }

    virtual void renderItem(int i, int x, int y, int h, Tesselator& t) {
        std::string text = languages[i].name;
        int textWidth = minecraft->font->width(text);
        
        int color = 0xFFFFFF;
        if (isSelectedItem(i)) {
            color = 0x55FF55; // Selected color (greenish)
            
            // Draw selection box outline like PC
            fill(x - 2, y - 2, x + 216 + 2, y + h + 2, 0x80FFFFFF);
            fill(x - 1, y - 1, x + 216 + 1, y + h + 1, 0xFF000000);
        }
        
        parent->drawCenteredString(minecraft->font, text, x + 108, y + 2, color);
    }

    virtual void renderBackground() {
        // Use the parent screen's background (panorama)
        parent->renderBackground();
    }
};

LanguageScreen::LanguageScreen() : btnDone(nullptr), btnFont(nullptr), languageList(nullptr) {}
LanguageScreen::~LanguageScreen() { 
    delete languageList; 
}

void LanguageScreen::init() {
    for (auto b : buttons) delete b;
    buttons.clear();

    btnDone = new Button(0, 0, 0, 150, 20, I18n::get("gui.done"));
    btnFont = new Button(1, 0, 0, 150, 20, I18n::get("options.fontSettings"));
    btnFont->active = false;

    buttons.push_back(btnFont);
    buttons.push_back(btnDone);

    if (languageList) { delete languageList; languageList = nullptr; }
    languageList = new LanguageSelectionList(this, minecraft, width, height, 32, height - 50, 18);

    setupPositions();
}

void LanguageScreen::setupPositions() {
    int subW = std::min((width - 30) / 2, 150);
    if (subW < 100) subW = 100;

    if (btnFont) {
        btnFont->width = subW;
        btnFont->x = width / 2 - subW - 5;
        btnFont->y = height - 30;
    }
    if (btnDone) {
        btnDone->width = subW;
        btnDone->x = width / 2 + 5;
        btnDone->y = height - 30;
    }
    if (languageList) {
        languageList->setDimensions(width, height, 32, height - 50);
    }
}

void LanguageScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->popScreen();
    }
}

void LanguageScreen::render(int xm, int ym, float a) {
    if (languageList) languageList->render(xm, ym, a);
    
    std::string titleStr = I18n::get("options.language");
    font->draw(titleStr, (float)(width / 2 - font->width(titleStr) / 2), 10.0f, 0xffffff, false);

    std::string warnStr = "(" + I18n::get("options.languageWarning") + ")";
    font->draw(warnStr, (float)(width / 2 - font->width(warnStr) / 2), (float)(height - 45), 0x808080, false);
    
    Screen::render(xm, ym, a);
}

void LanguageScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
}

void LanguageScreen::mouseReleased(int x, int y, int buttonNum) {
    Screen::mouseReleased(x, y, buttonNum);
}

void LanguageScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    Screen::mouseWheel(dx, dy, xm, ym);
}

void LanguageScreen::renderBackground() {
    Screen::renderBackground();
}

void LanguageScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->popScreen();
    }
}
