#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_SKINDEXSCREEN_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_SKINDEXSCREEN_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../../renderer/Textures.h"
#include <vector>
#include <string>

struct SkinPack {
	std::string name;
	std::string displayName; // Localized name from lang file
	std::vector<std::string> skins;
	std::vector<std::string> skinDisplayNames; // Localized skin names
	std::vector<std::string> skinGeometries; // Geometry type for each skin
	bool isInternal; // true if from data/images/skins, false if user-created
	int pageOffset = 0; // Current horizontal page offset for skins
};

class HumanoidModel;

class SkindexScreen : public Screen
{
public:
	SkindexScreen(int packIdx = -1, int skinIdx = -1);
	virtual ~SkindexScreen();
	
	virtual void init();
	virtual void setupPositions();
	
	virtual void render(int xm, int ym, float a);
	virtual void tick();

	virtual void buttonClicked(Button* button);
	virtual bool handleBackEvent(bool isDown);
	
	virtual bool isEscScreen() { return true; }

	virtual void mouseClicked(int x, int y, int buttonNum);
	virtual void mouseReleased(int x, int y, int buttonNum);
	virtual void mouseWheel(int dx, int dy, int xm, int ym);

private:
	void scanSkins();
	void ensureSkinsDir();
	
	Button btnConfirm;
	Button btnCancel;

	// Advanced skin management
	Button btnRename;
	Button btnDelete;
	Button btnNewPack;
	Button btnModel;
	Button btnCloseHeader;
	Button btnCardViewMode;
	Button btnAutoRotate;

	std::vector<SkinPack> skinPacks;
	std::vector<Button*> cardButtons;
	int currentPackIndex;
	int currentSkinIndex;
	bool isSlimModel;
	bool showFullBodyCards;
	bool autoRotate;
	float playerRot;
	bool isDraggingRot;
	bool isDraggingScroll;
	int lastMouseX;
	int lastMouseY;
	int packScrollOffset;

	void updateModelButtonText();
	void updateDefaultModelForSkin();
	void importSkinToPack(int packIndex);
	void deletePack(int packIndex);
	void clearCardButtons();
	void drawSkinCard(int x, int y, int w, int h, const std::string& skinPath, bool isSelected, const std::string& label, bool isSlim = false);
	void drawSkinBody2D(float x, float y, float w, float h, TextureId tid);
	void drawSkinBody3D(float x, float y, float w, float h, TextureId tid, bool isSlim);

	HumanoidModel* modelNormal;
	HumanoidModel* modelSlim;

	enum DeleteType { DELETE_NONE, DELETE_SKIN, DELETE_PACK };
	bool showDeleteModal;
	DeleteType pendingDeleteType;
	int pendingDeleteIndex;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS_SKINDEXSCREEN_H__*/
