#include "OptionsItem.h"
#include "../../Minecraft.h"
#include "../../../locale/I18n.h"
#include "../../../util/Mth.h"
OptionsItem::OptionsItem( OptionId optionId, std::string label, GuiElement* element )
: GuiElementContainer(false, true, 0, 0, 24, 12),
  m_optionId(optionId),
  m_labelKey(label) {
	  addChild(element);
}

void OptionsItem::setupPositions() {
	int currentHeight = 0;
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->x = x + width - (*it)->width - 15;
		(*it)->y = y + currentHeight;
		currentHeight += (*it)->height;
	}
	height = currentHeight;
}

bool OptionsItem::pointInside(int xm, int ym) {
	for (auto child : children) {
		if (child->pointInside(xm, ym)) return true;
	}
	return false;
}

void OptionsItem::render( Minecraft* minecraft, int xm, int ym ) {
	std::vector<int> oldYs;
	for(auto child : children) oldYs.push_back(child->y);
	setupPositions();

	int yOffset = (height - 8) / 2;
	// Read label live from I18n so it updates instantly when language changes
	std::string text = I18n::get(m_labelKey);
	// If key not found (I18n appends '<'), fall back to the raw key string
	if (!text.empty() && text.back() == '<') text = m_labelKey;
	
	if (m_optionId == OPTIONS_GUI_SCALE) {
		int value = minecraft->options.getIntValue(OPTIONS_GUI_SCALE);
		std::string scaleText;
		switch (value) {
		case 0: scaleText = I18n::get("options.guiScale.auto"); break;
		case 1: scaleText = I18n::get("options.guiScale.small"); break;
		case 2: scaleText = I18n::get("options.guiScale.medium"); break;
		case 3: scaleText = I18n::get("options.guiScale.large"); break;
		case 4: scaleText = I18n::get("options.guiScale.larger"); break;
		case 5: scaleText = I18n::get("options.guiScale.largest"); break;
		default: scaleText = I18n::get("options.guiScale.auto"); break;
		}
		text += ": " + scaleText;
	} else if (m_optionId == OPTIONS_FOV) {
		float val = minecraft->options.getProgressValue(OPTIONS_FOV);
		std::string valText;
		if (val <= 70.0f + 0.01f) {
			valText = I18n::get("options.fov.min");
		} else if (val >= 110.0f - 0.01f) {
			valText = I18n::get("options.fov.max");
		} else {
			valText = std::to_string((int)std::round(val));
		}
		text += ": " + valText;
	} else if (m_optionId == OPTIONS_BRIGHTNESS) {
		float val = minecraft->options.getProgressValue(OPTIONS_BRIGHTNESS);
		std::string valText;
		if (val <= 0.0f + 0.01f) {
			valText = I18n::get("options.gamma.min");
		} else if (val >= 1.0f - 0.01f) {
			valText = I18n::get("options.gamma.max");
		} else {
			valText = "+" + std::to_string((int)std::round(val * 100.0f)) + "%";
		}
		text += ": " + valText;
	} else if (m_optionId == OPTIONS_VIEW_DISTANCE) {
		int val = minecraft->options.getIntValue(OPTIONS_VIEW_DISTANCE);
		std::string valText = std::to_string(val);
		static const char* defaultNames[] = {"Lejano", "Normal", "Corto", "Mínimo"};
		std::string name = I18n::get("options.renderDistance." + std::to_string(val));
		if (name.empty() || name.back() == '<') {
			if (val >= 0 && val <= 3) {
				name = defaultNames[val];
			}
		}
		if (!name.empty()) {
			valText += " (" + name + ")";
		}
		text += ": " + valText;
	} else if (m_optionId == OPTIONS_MUSIC_VOLUME) {
		float val = minecraft->options.getProgressValue(OPTIONS_MUSIC_VOLUME);
		std::string valText;
		if (val <= 0.0f + 0.01f) {
			valText = I18n::get("options.off");
		} else {
			valText = std::to_string((int)std::round(val * 100.0f)) + "%";
		}
		text += ": " + valText;
	} else if (m_optionId == OPTIONS_SOUND_VOLUME) {
		float val = minecraft->options.getProgressValue(OPTIONS_SOUND_VOLUME);
		std::string valText;
		if (val <= 0.0f + 0.01f) {
			valText = I18n::get("options.off");
		} else {
			valText = std::to_string((int)std::round(val * 100.0f)) + "%";
		}
		text += ": " + valText;
	}

	minecraft->font->draw(text, (float)x, (float)y + yOffset, 0x909090, false);
	super::render(minecraft, xm, ym);

	for(size_t i = 0; i < children.size(); ++i) {
		children[i]->y = oldYs[i];
	}
}