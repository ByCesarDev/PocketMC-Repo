#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_SKINDEXSCREEN_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_SKINDEXSCREEN_H__

#include "../Screen.h"
#include "../components/Button.h"
#include <vector>
#include <string>

struct SkinPack {
	std::string name;
	std::vector<std::string> skins;
};

class SkindexScreen : public Screen
{
public:
	SkindexScreen();
	virtual ~SkindexScreen();
	
	virtual void init();
	virtual void setupPositions();
	
	virtual void render(int xm, int ym, float a);
	virtual void tick();

	virtual void buttonClicked(Button* button);
	virtual bool handleBackEvent(bool isDown);
	
	virtual bool isEscScreen() { return true; }

private:
	void scanSkins();
	void ensureSkinsDir();
	
	Button btnConfirm;
	Button btnCancel;
	Button btnImport;
	Button btnNext;
	Button btnPrev;

	// Advanced skin management
	Button btnPackPrev;
	Button btnPackNext;
	Button btnRename;
	Button btnDelete;
	Button btnNewPack;
	Button btnModel;

	std::vector<SkinPack> skinPacks;
	int currentPackIndex;
	int currentSkinIndex;
	bool isSlimModel;
	float playerRot;
	void updateModelButtonText();
	void updateDefaultModelForSkin();
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS_SKINDEXSCREEN_H__*/
