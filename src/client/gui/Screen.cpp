#include "Screen.h"
#include "components/Button.h"
#include "components/TextBox.h"
#include "components/NinePatch.h"
#include "../Minecraft.h"
#include "../renderer/Tesselator.h"
#include "../renderer/gles.h"
#include "../renderer/GuiShader.h"
#include "../sound/SoundEngine.h"
#include "../../platform/input/Keyboard.h"
#include "../../platform/input/Mouse.h"
#include "../renderer/Textures.h"

Screen::Screen()
:   passEvents(false),
	clickedButton(NULL),
	tabButtonIndex(0),
	width(1),
	height(1),
	minecraft(NULL),
	font(NULL)
{
}

Screen::~Screen()
{
}

void Screen::render( int xm, int ym, float a )
{
	for (unsigned int i = 0; i < buttons.size(); i++) {
		Button* button = buttons[i];
		button->render(minecraft, xm, ym);
	}

	// render any text boxes after buttons
	for (unsigned int i = 0; i < textBoxes.size(); i++) {
		TextBox* textbox = textBoxes[i];
		textbox->render(minecraft, xm, ym);
	}
}

void Screen::renderTooltip(const std::string& text, int xm, int ym) {
	if (text.empty()) return;

	Font* font = minecraft->font;
	int textWidth = font->width(text);
	int w = textWidth + 12;
	int h = 18;

	int tx = xm + 12;
	int ty = ym - 12;

	if (tx + w > width) {
		tx = xm - w - 8;
	}
	if (ty < 4) {
		ty = ym + 14;
	}
	if (ty + h > height) {
		ty = height - h - 4;
	}

	glDisable2(GL_DEPTH_TEST);
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Render effect_background.png as a 9-patch to preserve exact transparent rounded corners without distortion
	static NinePatchLayer* s_tooltipPatch = NULL;
	static Textures* s_lastTextures = NULL;
	if (!s_tooltipPatch || s_lastTextures != minecraft->textures) {
		delete s_tooltipPatch;
		s_lastTextures = minecraft->textures;
		NinePatchFactory builder(minecraft->textures, "gui/effect_background.png");
		s_tooltipPatch = builder.createSymmetrical(IntRectangle(0, 0, 120, 32), 6, 6);
	}

	if (s_tooltipPatch) {
		Tesselator& t = Tesselator::instance;
		glColor4f2(1, 1, 1, 1);
		t.colorABGR(0xFFFFFFFF);
		s_tooltipPatch->setSize(static_cast<float>(w), static_cast<float>(h));
		s_tooltipPatch->draw(t, static_cast<float>(tx), static_cast<float>(ty));
	}

	// Draw text with shadow centered vertically inside the 9-patch frame
	drawString(font, text, tx + 6, ty + (h - 8) / 2, 0xFFFFFFFF);

	glDisable2(GL_BLEND);
	glEnable2(GL_DEPTH_TEST);
}

void Screen::init( Minecraft* minecraft, int width, int height )
{
	//particles = /*new*/ GuiParticles(minecraft);
	this->minecraft = minecraft;
	this->font = minecraft->font;
	this->width = width;
	this->height = height;
	init();
	setupPositions();
	updateTabButtonSelection();
}

void Screen::init()
{
}

void Screen::setSize( int width, int height )
{
	this->width = width;
	this->height = height;
	setupPositions();
}

bool Screen::handleBackEvent( bool isDown )
{
	return false;
}

void Screen::updateEvents()
{
	if (passEvents)
		return;

	while (Mouse::next())
		mouseEvent();

	while (Keyboard::next())
		keyboardEvent();
	while (Keyboard::nextTextChar())
		keyboardTextEvent();
}

void Screen::mouseEvent()
{
	const MouseAction& e = Mouse::getEvent();
	int xm = e.x * width / minecraft->width;
	int ym = e.y * height / minecraft->height - 1;

	// forward wheel events to subclasses
	if (e.action == MouseAction::ACTION_WHEEL) {
		mouseWheel(e.dx, e.dy, xm, ym);
		return;
	}

	if (e.action == MouseAction::ACTION_MOVE) {
		int dx = e.dx * width / minecraft->width;
		int dy = e.dy * height / minecraft->height;
		mouseMoved(xm, ym, dx, dy);
		return;
	}

	if (!e.isButton())
		return;

	if (Mouse::getEventButtonState()) {
		mouseClicked(xm, ym, Mouse::getEventButton());
	} else {
		mouseReleased(xm, ym, Mouse::getEventButton());
	}
}

void Screen::keyboardEvent()
{
	if (Keyboard::getEventKeyState()) {
		//if (Keyboard.getEventKey() == Keyboard.KEY_F11) {
		//    minecraft->toggleFullScreen();
		//    return;
		//}
		keyPressed(Keyboard::getEventKey());
	}
}
void Screen::keyboardTextEvent()
{
	charPressed(Keyboard::getChar());
}
void Screen::renderBackground()
{
	renderBackground(0);
}

void Screen::renderBackground( int vo )
{
	if (minecraft->isLevelGenerated()) {
		fillGradient(0, 0, width, height, 0xc0101010, 0xd0101010);
	} else {
		renderDirtBackground(vo);
	}
}

#include <chrono>

float Screen::getPanoramaTime() {
    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    return duration / 50.0f;
}

void Screen::renderDirtBackground( int vo )
{
	renderPanorama((int)getPanoramaTime(), getPanoramaTime() - (int)getPanoramaTime());

	// Dark semi-transparent overlay
	glDisable2(GL_DEPTH_TEST);
	glDisable2(GL_ALPHA_TEST);
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	fillGradient(0, 0, width, height, 0xB0000000, 0xB0000000);
	
	glDisable2(GL_BLEND);
}

std::string Screen::s_overridePanoramaPath = "";

void Screen::setOverridePanoramaPath(const std::string& path) {
	s_overridePanoramaPath = path;
}

void Screen::renderPanorama(int ticks, float a)
{
	GuiShader::unbind();

	Tesselator& t = Tesselator::instance;

	glDisable2(GL_DEPTH_TEST);
	glDisable2(GL_FOG);
	glDisable2(GL_ALPHA_TEST);
	glDisable2(GL_CULL_FACE);
	glDisable2(GL_BLEND);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(90.0f, (float)width / (float)height, 0.05f, 10.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);
	
	// Rotations
	glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
	glRotatef((ticks + a) * 0.1f, 0.0f, 1.0f, 0.0f);

	std::string panPath = s_overridePanoramaPath;
	if (panPath.empty() && minecraft) {
		panPath = minecraft->options.getStringValue(OPTIONS_PANORAMA_PATH);
	}
	if (panPath.empty()) {
		panPath = "gui/panorama/";
	}
	if (panPath.back() != '/' && panPath.back() != '\\') {
		panPath += "/";
	}

	for (int i = 0; i < 6; i++) {
		minecraft->textures->loadAndBindTexture(panPath + "panorama_" + std::to_string(i) + ".png");

		glPushMatrix();
		if (i == 1) glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		if (i == 2) glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		if (i == 3) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		if (i == 4) glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		if (i == 5) glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

		t.begin();
		t.vertexUV(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f);
		t.vertexUV( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f);
		t.vertexUV( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f);
		t.vertexUV(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f);
		t.draw();

		glPopMatrix();
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable2(GL_DEPTH_TEST);
	glEnable2(GL_CULL_FACE);
	glEnable2(GL_ALPHA_TEST);

	GuiShader::bind();
}

bool Screen::isPauseScreen()
{
	return false;
}

bool Screen::isErrorScreen()
{
	return false;
}

bool Screen::isInGameScreen()
{
	return true;
}

bool Screen::closeOnPlayerHurt() {
    return false;
}

void Screen::keyPressed( int eventKey )
{
	if (eventKey == Keyboard::KEY_ESCAPE) {
		minecraft->setScreen(NULL);
		//minecraft->grabMouse();
	}

	// pass key events to any text boxes first
	for (auto& textbox : textBoxes) {
		textbox->keyPressed(minecraft, eventKey);
	}

#ifdef TABBING
	if (minecraft->useTouchscreen())
		return;


	// "Tabbing" the buttons (walking with keys)
	const int tabButtonCount = tabButtons.size();
	if (!tabButtonCount)
		return;

	Options& o = minecraft->options;
	if (eventKey == o.getIntValue(OPTIONS_KEY_MENU_NEXT))
		if (++tabButtonIndex == tabButtonCount) tabButtonIndex = 0;
	if (eventKey == o.getIntValue(OPTIONS_KEY_MENU_PREV))
		if (--tabButtonIndex == -1) tabButtonIndex = tabButtonCount-1;
	if (eventKey == o.getIntValue(OPTIONS_KEY_MENU_OK)) {
		Button* button = tabButtons[tabButtonIndex];
		if (button->active) {
			minecraft->soundEngine->playUI("random.click", 1, 1);
			buttonClicked(button);
		}
	}

	updateTabButtonSelection();
#endif
}

void Screen::charPressed(char inputChar) {
	for (auto& textbox : textBoxes) {
		textbox->charPressed(minecraft, inputChar);
	}
}

void Screen::updateTabButtonSelection()
{
#ifdef TABBING
	if (minecraft->useTouchscreen())
		return;

	for (unsigned int i = 0; i < tabButtons.size(); ++i)
		tabButtons[i]->selected = (i == tabButtonIndex);
#endif
}

void Screen::mouseClicked( int x, int y, int buttonNum )
{
	if (buttonNum == MouseAction::ACTION_LEFT) {
		for (unsigned int i = 0; i < buttons.size(); ++i) {
			Button* button = buttons[i];
            //LOGI("Hit-testing button: %p\n", button);
			if (button->clicked(minecraft, x, y)) {
                button->setPressed();

                //LOGI("Hit-test successful: %p\n", button);
				clickedButton = button;
/*
#if !defined(ANDROID) && !defined(__APPLE__) //if (!minecraft->isTouchscreen()) {
					minecraft->soundEngine->playUI("random.click", 1, 1);
					buttonClicked(button);
#endif }
*/
			}
		}
	}

	// let textboxes see the click regardless
	for (auto& textbox : textBoxes) {
		textbox->mouseClicked(minecraft, x, y, buttonNum);
	}
}

void Screen::mouseReleased( int x, int y, int buttonNum )
{
	if (!clickedButton || buttonNum != MouseAction::ACTION_LEFT) return;

	Button* button = clickedButton;
	clickedButton = NULL;
	button->released(x, y);

	if (button->clicked(minecraft, x, y)) {
		minecraft->soundEngine->playUI("random.click", 1, 1);
		buttonClicked(button);
	}
}

bool Screen::renderGameBehind() {
	return true;
}

bool Screen::hasClippingArea( IntRectangle& out )
{
	return false;
}

void Screen::lostFocus() {
	for(std::vector<TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it) {
		TextBox* tb = *it;
		tb->loseFocus(minecraft);
	}
}

void Screen::toGUICoordinate( int& x, int& y ) {
	x = x * width / minecraft->width;
	y = y * height / minecraft->height - 1;
}
