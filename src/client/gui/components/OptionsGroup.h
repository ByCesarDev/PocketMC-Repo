#ifndef NET_MINECRAFT_CLIENT_GUI_COMPONENTS__OptionsGroup_H__
#define NET_MINECRAFT_CLIENT_GUI_COMPONENTS__OptionsGroup_H__

//package net.minecraft.client.gui;

#include <string>
#include "GuiElementContainer.h"
#include "ScrollingPane.h"
#include "../../Options.h"

class Font;
class Minecraft;

class OptionsGroup: public GuiElementContainer {
	typedef GuiElementContainer super;
public:
	OptionsGroup(std::string labelID);
	virtual void setupPositions();
	virtual void render(Minecraft* minecraft, int xm, int ym);
	OptionsGroup& addOptionItem(OptionId optId, Minecraft* minecraft);
	void setTitle(const std::string& newLabel) { label = newLabel; }
    virtual void mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum);
    virtual void mouseReleased(Minecraft* minecraft, int x, int y, int buttonNum);
    virtual void mouseWheel(int dx, int dy, int xm, int ym);
	int getScrollOffset() const { return scrollOffset; }
protected:

	void createToggle(OptionId optId, Minecraft* minecraft);
	void createProgressSlider(OptionId optId, Minecraft* minecraft);
	void createStepSlider(OptionId optId, Minecraft* minecraft);
	void createTextbox(OptionId optId, Minecraft* minecraft);
	void createKey(OptionId optId, Minecraft* minecraft);

	std::string label;
	int scrollOffset;
	int contentHeight;
	bool isDragging;
	int lastDragY;
	float dragInertia;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_COMPONENTS__OptionsGroup_H__*/
