#include "OptionsGroup.h"
#include "../../Minecraft.h"
#include "ImageButton.h"
#include "OptionsItem.h"
#include "Slider.h"
#include "../../../locale/I18n.h"
#include "TextOption.h"
#include "KeyOption.h"
#include "../../../platform/input/Mouse.h"

OptionsGroup::OptionsGroup( std::string labelID )  {
	label = labelID;
	scrollOffset = 0;
	contentHeight = 0;
	isDragging = false;
	lastDragY = 0;
	dragInertia = 0.0f;
}

void OptionsGroup::setupPositions() {
	// First we write the header and then we add the items
	int curY = y + 18;
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->width = width - 15;
		
		(*it)->y = curY;
		(*it)->x = x + 5;
		(*it)->setupPositions();
		curY += (*it)->height + 3;
	}
	// total content height (including header area)
	contentHeight = curY - y;
}

void OptionsGroup::render( Minecraft* minecraft, int xm, int ym ) {
	// Touch / mouse drag scrolling
	if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
		if (isDragging) {
			int dy = ym - lastDragY;
			if (dy != 0) {
				scrollOffset -= dy;
				dragInertia = (float)dy;
				lastDragY = ym;
			}
		} else if (xm >= x && xm <= x + width && ym >= y && ym <= y + height) {
			bool clickedChild = false;
			for (auto child : children) {
				if (child->pointInside(xm, ym)) {
					clickedChild = true;
					break;
				}
			}
			if (!clickedChild) {
				isDragging = true;
				lastDragY = ym;
				dragInertia = 0.0f;
			}
		}
	} else {
		if (isDragging) {
			isDragging = false;
		}
		if (std::abs(dragInertia) > 0.5f) {
			scrollOffset -= (int)dragInertia;
			dragInertia *= 0.85f;
		} else {
			dragInertia = 0.0f;
		}
	}

	// Clamp scrollOffset
	int maxOffset = contentHeight - height;
	if (maxOffset < 0) maxOffset = 0;
	if (scrollOffset < 0) scrollOffset = 0;
	if (scrollOffset > maxOffset) scrollOffset = maxOffset;

	float padX = 10.0f;
	float padY = 5.0f;
	
	std::string headerText = I18n::get(label);
	if (!headerText.empty() && headerText.back() == '<') headerText = label;
	minecraft->font->draw(headerText, (float)x + padX, (float)y + padY, 0xffffffff, false);

	// Render children with vertical scroll offset and simple clipping
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		GuiElement* child = *it;
		int oldY = child->y;
		int drawY = oldY - scrollOffset;

		// cull items outside visible area
		if (drawY + child->height < y || drawY > y + height) continue;

		child->y = drawY;
		child->render(minecraft, xm, ym);
		child->y = oldY;
	}
}

OptionsGroup& OptionsGroup::addOptionItem(OptionId optId, Minecraft* minecraft ) {
	auto option = minecraft->options.getOpt(optId);

	if (option == nullptr) return *this;

	// TODO: do a options key class to check it faster via dynamic_cast
	if (option->getStringId().find("options.key") != std::string::npos) createKey(optId, minecraft);
	else if (dynamic_cast<OptionBool*>(option)) createToggle(optId, minecraft);
	else if (dynamic_cast<OptionFloat*>(option)) createProgressSlider(optId, minecraft);
	else if (dynamic_cast<OptionInt*>(option)) createStepSlider(optId, minecraft);
	else if (dynamic_cast<OptionString*>(option)) createTextbox(optId, minecraft);

	return *this;
}

// TODO: wrap this copypaste shit into templates

void OptionsGroup::createToggle(OptionId optId, Minecraft* minecraft ) {
	ImageDef def;

	def.setSrc(IntRectangle(160, 206, 39, 20));
	def.name = "gui/touchgui.png";
	def.width = 39 * 0.7f;
	def.height = 20 * 0.7f;
	
	OptionButton* element = new OptionButton(optId);
	element->setImageDef(def, true);
	element->updateImage(&minecraft->options);
	
	std::string itemLabel = minecraft->options.getOpt(optId)->getStringId();
	
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	
	addChild(item);
	setupPositions();
}

void OptionsGroup::createProgressSlider(OptionId optId, Minecraft* minecraft ) {
	Slider* element = new SliderFloat(minecraft, optId);
	element->width = 100;
	element->height = 20;

	std::string itemLabel = minecraft->options.getOpt(optId)->getStringId();
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	addChild(item);
	setupPositions();
}

void OptionsGroup::createStepSlider(OptionId optId, Minecraft* minecraft ) {
	Slider* element = new SliderInt(minecraft, optId);
	element->width = 100;
	element->height = 20;

	std::string itemLabel = minecraft->options.getOpt(optId)->getStringId();
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	addChild(item);
	setupPositions();
}

void OptionsGroup::createTextbox(OptionId optId, Minecraft* minecraft) {
	TextBox* element = new TextOption(minecraft, optId);
	element->width = 100;
	element->height = 20;

	std::string itemLabel = minecraft->options.getOpt(optId)->getStringId();
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	addChild(item);
	setupPositions();
}

void OptionsGroup::createKey(OptionId optId, Minecraft* minecraft) {
	KeyOption* element = new KeyOption(minecraft, optId);
	element->width = 50;
	element->height = 20;

	std::string itemLabel = minecraft->options.getOpt(optId)->getStringId();
	OptionsItem* item = new OptionsItem(optId, itemLabel, element);
	addChild(item);
	setupPositions();
}

void OptionsGroup::mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) {
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->mouseClicked(minecraft, x, y, buttonNum);
	}
}

void OptionsGroup::mouseReleased(Minecraft* minecraft, int x, int y, int buttonNum) {
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->mouseReleased(minecraft, x, y, buttonNum);
	}
}

void OptionsGroup::mouseWheel(int dx, int dy, int xm, int ym) {
	// dy is wheel delta (positive scrolls down). Tune speed.
	int step = dy * 12; // pixels per wheel step
	scrollOffset -= step; // invert so wheel up moves content up
	if (scrollOffset < 0) scrollOffset = 0;
	int maxOffset = contentHeight - height;
	if (maxOffset < 0) maxOffset = 0;
	if (scrollOffset > maxOffset) scrollOffset = maxOffset;
}