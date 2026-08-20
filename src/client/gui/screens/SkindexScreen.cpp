#include "SkindexScreen.h"
#include "RenameSkinScreen.h"
#include "NewPackScreen.h"
#include <algorithm>
#include "../../Minecraft.h"
#include "../../Options.h"
#include "../../player/LocalPlayer.h"
#include "../../renderer/entity/EntityRenderDispatcher.h"
#include "../../renderer/Textures.h"
#include "../../model/HumanoidModel.h"
#include "../../renderer/GuiShader.h"
#include "../../renderer/Tesselator.h"
#include "../../../locale/I18n.h"
#include "../../../util/Mth.h"
#include "../../../platform/input/Mouse.h"
#include "../../sound/SoundEngine.h"
#include "world/level/storage/FolderMethods.h"
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#else
#include <dirent.h>
#endif

#if defined(ANDROID)
extern "C" void pickImage_JNI();
#endif

SkindexScreen::SkindexScreen()
:	btnConfirm(0, 0, 0, 100, 22, I18n::get("gui.confirm")),
	btnCancel(1, 0, 0, 75, 22, "<- " + I18n::get("gui.back")),
	btnRename(7, 0, 0, 50, 18, I18n::get("gui.rename")),
	btnDelete(8, 0, 0, 50, 18, I18n::get("gui.delete")),
	btnNewPack(9, 0, 0, 80, 18, I18n::get("gui.newPack")),
	btnModel(10, 0, 0, 90, 18, I18n::get("skindex.model") + ": " + I18n::get("skindex.model.normal")),
	btnCloseHeader(11, 0, 0, 20, 18, "X"),
	btnCardViewMode(12, 0, 0, 95, 18, I18n::get("skindex.view") + ": " + I18n::get("skindex.view.body")),
	btnAutoRotate(13, 0, 0, 95, 18, I18n::get("skindex.autoRotate")),
	currentPackIndex(0),
	currentSkinIndex(0),
	isSlimModel(false),
	showFullBodyCards(true),
	autoRotate(true),
	playerRot(0.0f),
	isDraggingRot(false),
	lastMouseX(0)
{
}

SkindexScreen::~SkindexScreen() {
	clearCardButtons();
}

void SkindexScreen::clearCardButtons() {
	for (Button* btn : cardButtons) {
		delete btn;
	}
	cardButtons.clear();
}

void SkindexScreen::updateModelButtonText() {
	btnModel.msg = I18n::get("skindex.model") + ": " + (isSlimModel ? I18n::get("skindex.model.slim") : I18n::get("skindex.model.normal"));
}

void SkindexScreen::updateDefaultModelForSkin() {
	if (skinPacks.empty()) return;
	if (currentPackIndex < 0 || currentPackIndex >= (int)skinPacks.size()) currentPackIndex = 0;
	
	SkinPack& pack = skinPacks[currentPackIndex];
	if (pack.skins.empty()) return;
	if (currentSkinIndex < 0 || currentSkinIndex >= (int)pack.skins.size()) currentSkinIndex = 0;

	std::string currentSkin = pack.skins[currentSkinIndex];
	if (currentSkin.find("cesar.png") != std::string::npos || currentSkin.find("cesar malo.png") != std::string::npos) {
		isSlimModel = true;
	} else if (currentSkin.find("steve.png") != std::string::npos) {
		isSlimModel = false;
	} else if (minecraft) {
		isSlimModel = (minecraft->options.getStringValue(OPTIONS_SKIN_MODEL) == "slim");
	}
	updateModelButtonText();
}

void SkindexScreen::ensureSkinsDir() {
	createFolderIfNotExists("games");
	createFolderIfNotExists("games/com.mojang");
	createFolderIfNotExists("games/com.mojang/skins");
	createFolderIfNotExists("games/com.mojang/skins/Default");
#ifdef _WIN32
	RemoveDirectoryA("games\\com.mojang\\skins\\Personalizados");
#endif
}

void SkindexScreen::scanSkins() {
	skinPacks.clear();
	ensureSkinsDir();

	// Initialize default skins in games/com.mojang/skins/Default
#ifdef _WIN32
	CopyFileA("data/images/skins/steve.png", "games/com.mojang/skins/Default/steve.png", FALSE);
	CopyFileA("data/images/skins/cesar.png", "games/com.mojang/skins/Default/cesar.png", FALSE);
	CopyFileA("data/images/skins/cesar malo.png", "games/com.mojang/skins/Default/cesar malo.png", FALSE);
#elif !defined(ANDROID)
	auto copyFile = [](const std::string& src, const std::string& dest) {
		std::ifstream source(src, std::ios::binary);
		std::ofstream destination(dest, std::ios::binary);
		destination << source.rdbuf();
	};
	copyFile("data/images/skins/steve.png", "games/com.mojang/skins/Default/steve.png");
	copyFile("data/images/skins/cesar.png", "games/com.mojang/skins/Default/cesar.png");
	copyFile("data/images/skins/cesar malo.png", "games/com.mojang/skins/Default/cesar malo.png");
#endif

#ifdef _WIN32
	WIN32_FIND_DATAA findDirData;
	HANDLE hFindDir = FindFirstFileA("games\\com.mojang\\skins\\*", &findDirData);
	if (hFindDir != INVALID_HANDLE_VALUE) {
		do {
			if (findDirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				std::string dirName = findDirData.cFileName;
				if (dirName != "." && dirName != ".." && dirName != "Personalizados") {
					SkinPack pack;
					pack.name = dirName;
					
					std::string searchPath = "games\\com.mojang\\skins\\" + dirName + "\\*.png";
					WIN32_FIND_DATAA findFileData;
					HANDLE hFindFile = FindFirstFileA(searchPath.c_str(), &findFileData);
					if (hFindFile != INVALID_HANDLE_VALUE) {
						do {
							if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
								pack.skins.push_back("games/com.mojang/skins/" + dirName + "/" + findFileData.cFileName);
							}
						} while (FindNextFileA(hFindFile, &findFileData) != 0);
						FindClose(hFindFile);
					}
					skinPacks.push_back(pack);
				}
			}
		} while (FindNextFileA(hFindDir, &findDirData) != 0);
		FindClose(hFindDir);
	}
#elif !defined(ANDROID)
	DIR* dir = opendir("games/com.mojang/skins");
	if (dir != NULL) {
		struct dirent* ent;
		while ((ent = readdir(dir)) != NULL) {
			std::string dirName = ent->d_name;
			if (dirName != "." && dirName != ".." && dirName != "Personalizados") {
				std::string fullDirPath = "games/com.mojang/skins/" + dirName;
				DIR* subDir = opendir(fullDirPath.c_str());
				if (subDir != NULL) {
					SkinPack pack;
					pack.name = dirName;
					struct dirent* subEnt;
					while ((subEnt = readdir(subDir)) != NULL) {
						std::string fileName = subEnt->d_name;
						if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == ".png") {
							pack.skins.push_back(fullDirPath + "/" + fileName);
						}
					}
					closedir(subDir);
					skinPacks.push_back(pack);
				}
			}
		}
		closedir(dir);
	}
#endif

	// Sort packs: "Default" first, others alphabetically
	std::sort(skinPacks.begin(), skinPacks.end(), [](const SkinPack& a, const SkinPack& b) {
		if (a.name == "Default") return true;
		if (b.name == "Default") return false;
		return a.name < b.name;
	});

	if (skinPacks.empty()) {
		SkinPack pack;
		pack.name = "Default";
#ifdef ANDROID
		pack.skins.push_back("images/skins/steve.png");
		pack.skins.push_back("images/skins/cesar.png");
		pack.skins.push_back("images/skins/cesar malo.png");
#else
		pack.skins.push_back("games/com.mojang/skins/Default/steve.png");
#endif
		skinPacks.push_back(pack);
	}
}

void SkindexScreen::init() {
	scanSkins();

	std::string currentSkin = minecraft->options.getStringValue(OPTIONS_SKIN);
#ifdef ANDROID
	if (currentSkin == "" || currentSkin == "Default") currentSkin = "images/skins/steve.png";
#else
	if (currentSkin == "" || currentSkin == "Default") currentSkin = "games/com.mojang/skins/Default/steve.png";
#endif

	currentPackIndex = 0;
	currentSkinIndex = 0;

	bool found = false;
	for (int p = 0; p < (int)skinPacks.size(); ++p) {
		for (int s = 0; s < (int)skinPacks[p].skins.size(); ++s) {
			if (skinPacks[p].skins[s] == currentSkin) {
				currentPackIndex = p;
				currentSkinIndex = s;
				found = true;
				break;
			}
		}
		if (found) break;
	}

	buttons.push_back(&btnConfirm);
	buttons.push_back(&btnCardViewMode);
	buttons.push_back(&btnAutoRotate);
	buttons.push_back(&btnRename);
	buttons.push_back(&btnDelete);
	buttons.push_back(&btnNewPack);
	buttons.push_back(&btnModel);
	buttons.push_back(&btnCloseHeader);

	setupPositions();
	updateDefaultModelForSkin();
}

void SkindexScreen::setupPositions() {
	// Dynamic i18n text updates
	btnConfirm.msg = I18n::get("gui.confirm");
	btnRename.msg = I18n::get("gui.rename");
	btnDelete.msg = I18n::get("gui.delete");
	btnNewPack.msg = I18n::get("gui.newPack");
	updateModelButtonText();
	btnCardViewMode.msg = I18n::get("skindex.view") + ": " + (showFullBodyCards ? I18n::get("skindex.view.body") : I18n::get("skindex.view.head"));
	btnAutoRotate.msg = autoRotate ? I18n::get("skindex.autoRotate") : I18n::get("skindex.manualRotate");

	// Top Header Bar
	btnNewPack.width = (std::max)(75, font->width(btnNewPack.msg) + 12);
	btnNewPack.x = 6;
	btnNewPack.y = 4;
	btnNewPack.height = 18;

	btnCardViewMode.width = (std::max)(85, font->width(btnCardViewMode.msg) + 12);
	btnCardViewMode.x = btnNewPack.x + btnNewPack.width + 4;
	btnCardViewMode.y = 4;
	btnCardViewMode.height = 18;

	btnCloseHeader.width = 20;
	btnCloseHeader.height = 18;
	btnCloseHeader.x = width - 24;
	btnCloseHeader.y = 4;

	btnAutoRotate.width = (std::max)(85, font->width(btnAutoRotate.msg) + 12);
	btnAutoRotate.x = btnCloseHeader.x - btnAutoRotate.width - 4;
	btnAutoRotate.y = 4;
	btnAutoRotate.height = 18;

	// Split Panel Dimensions
	int topY = 26;
	int leftBottomY = height - 6;
	int leftAvailableH = leftBottomY - topY;

	int rightBottomY = height - 28;
	int rightAvailableH = rightBottomY - topY;

	int leftX = 6;
	int leftW = (int)(width * 0.54f);
	int rightX = leftX + leftW + 6;
	int rightW = width - rightX - 6;

	// Right Panel Action Buttons
	int actionY = topY + rightAvailableH - 22;
	int buttonW = (rightW - 16) / 3;
	btnModel.x = rightX + 4;
	btnModel.y = actionY;
	btnModel.width = buttonW;
	btnModel.height = 18;

	btnRename.x = btnModel.x + buttonW + 4;
	btnRename.y = actionY;
	btnRename.width = buttonW;
	btnRename.height = 18;

	btnDelete.x = btnRename.x + buttonW + 4;
	btnDelete.y = actionY;
	btnDelete.width = buttonW;
	btnDelete.height = 18;

	btnConfirm.x = rightX;
	btnConfirm.y = height - 24;
	btnConfirm.width = rightW;
	btnConfirm.height = 20;

	// --- RE-POPULATE CARD BUTTONS FOR ALL PACKS ---
	clearCardButtons();

	int cardW = 46;
	int cardH = showFullBodyCards ? 54 : 46;
	int packRowH = showFullBodyCards ? 76 : 68;

	for (int p = 0; p < (int)skinPacks.size(); ++p) {
		int renderY = topY + 6 + p * packRowH;
		if (renderY + packRowH > topY + leftAvailableH) break;

		SkinPack& pack = skinPacks[p];
		int boxY = renderY + 12;

		for (int s = 0; s < (int)pack.skins.size(); ++s) {
			int cardX = leftX + 10 + s * (cardW + 5);
			int cardY = boxY + 2;

			if (cardX + cardW <= leftX + leftW - 8) {
				std::string skinName = pack.skins[s];
				size_t slashPos = skinName.find_last_of("\\/");
				if (slashPos != std::string::npos) skinName = skinName.substr(slashPos + 1);
				if (skinName.length() > 4 && skinName.substr(skinName.length() - 4) == ".png") {
					skinName = skinName.substr(0, skinName.length() - 4);
				}

				Button* cardBtn = new Button(1000 + p * 100 + s, cardX, cardY, cardW, cardH, skinName);
				cardButtons.push_back(cardBtn);
			}
		}
	}
}

void SkindexScreen::tick() {
	if (autoRotate && !isDraggingRot) {
		playerRot += 0.3f;
		if (playerRot >= 360.0f) playerRot -= 360.0f;
	}
}

void SkindexScreen::importSkinToPack(int packIndex) {
	if (skinPacks.empty() || packIndex < 0 || packIndex >= (int)skinPacks.size()) return;
	std::string targetPackName = skinPacks[packIndex].name;
	std::string targetDir = "games/com.mojang/skins/" + targetPackName;
	createFolderIfNotExists(targetDir.c_str());

#ifdef _WIN32
	OPENFILENAMEA ofn;
	char szFile[260] = {0};
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "PNG Images\0*.PNG\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE) {
		std::string src = szFile;
		std::string filename = src;
		size_t pos = filename.find_last_of("\\/");
		if (pos != std::string::npos) {
			filename = filename.substr(pos + 1);
		}

		std::string dest = "games\\com.mojang\\skins\\" + targetPackName + "\\" + filename;
		if (CopyFileA(src.c_str(), dest.c_str(), FALSE) || GetLastError() == ERROR_ALREADY_EXISTS) {
			scanSkins();
			setupPositions();
			for (int p = 0; p < (int)skinPacks.size(); ++p) {
				if (skinPacks[p].name == targetPackName) {
					currentPackIndex = p;
					for (int s = 0; s < (int)skinPacks[p].skins.size(); ++s) {
						if (skinPacks[p].skins[s].find(filename) != std::string::npos) {
							currentSkinIndex = s;
							break;
						}
					}
					break;
				}
			}
			updateDefaultModelForSkin();
		}
	}
#elif defined(ANDROID)
	pickImage_JNI();
#endif
}

void SkindexScreen::mouseClicked(int x, int y, int buttonNum) {
	Screen::mouseClicked(x, y, buttonNum);

	int topY = 26;
	int leftBottomY = height - 6;
	int leftAvailableH = leftBottomY - topY;
	int rightBottomY = height - 28;
	int rightAvailableH = rightBottomY - topY;

	int leftX = 6;
	int leftW = (int)(width * 0.54f);
	int rightX = leftX + leftW + 6;
	int rightW = width - rightX - 6;

	// Accept MouseAction::ACTION_LEFT (1) or any button press
	if (buttonNum == MouseAction::ACTION_LEFT || buttonNum == 1 || buttonNum == 0) {
		// 1. Direct card click check
		int cardW = 46;
		int cardH = showFullBodyCards ? 54 : 46;
		int packRowH = showFullBodyCards ? 76 : 68;

		for (int p = 0; p < (int)skinPacks.size(); ++p) {
			int renderY = topY + 6 + p * packRowH;
			if (renderY + packRowH > topY + leftAvailableH) break;

			SkinPack& pack = skinPacks[p];
			int boxY = renderY + 12;

			for (int s = 0; s < (int)pack.skins.size(); ++s) {
				int cardX = leftX + 10 + s * (cardW + 5);
				int cardY = boxY + 2;

				if (x >= cardX && x <= (cardX + cardW) && y >= cardY && y <= (cardY + cardH)) {
					currentPackIndex = p;
					currentSkinIndex = s;
					updateDefaultModelForSkin();
					if (minecraft && minecraft->soundEngine) {
						minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
					}
					return;
				}
			}

			// 2. Check [ Importar ] per-pack button click for custom packs
			if (pack.name != "Default") {
				int impBtnX = leftX + leftW - 68;
				int impBtnY = renderY - 2;
				int impBtnW = 60;
				int impBtnH = 14;

				if (x >= impBtnX && x <= impBtnX + impBtnW && y >= impBtnY && y <= impBtnY + impBtnH) {
					importSkinToPack(p);
					return;
				}
			}
		}

		// 3. Rotation Drag Area (Right Panel 3D area)
		if (x >= rightX && x <= rightX + rightW && y >= topY && y <= topY + rightAvailableH - 26) {
			isDraggingRot = true;
			lastMouseX = x;
		}
	}
}

void SkindexScreen::mouseReleased(int x, int y, int buttonNum) {
	Screen::mouseReleased(x, y, buttonNum);
	if (buttonNum == MouseAction::ACTION_LEFT || buttonNum == 1 || buttonNum == 0) {
		isDraggingRot = false;

		// Check skin card selection on mouse release for 100% click responsiveness
		int topY = 26;
		int leftBottomY = height - 6;
		int leftAvailableH = leftBottomY - topY;
		int leftX = 6;
		int leftW = (int)(width * 0.54f);

		int cardW = 46;
		int cardH = showFullBodyCards ? 54 : 46;
		int packRowH = showFullBodyCards ? 76 : 68;

		for (int p = 0; p < (int)skinPacks.size(); ++p) {
			int renderY = topY + 6 + p * packRowH;
			if (renderY + packRowH > topY + leftAvailableH) break;

			SkinPack& pack = skinPacks[p];
			int boxY = renderY + 12;

			for (int s = 0; s < (int)pack.skins.size(); ++s) {
				int cardX = leftX + 10 + s * (cardW + 5);
				int cardY = boxY + 2;

				if (x >= cardX && x <= (cardX + cardW) && y >= cardY && y <= (cardY + cardH)) {
					currentPackIndex = p;
					currentSkinIndex = s;
					updateDefaultModelForSkin();
					return;
				}
			}
		}
	}
}

void SkindexScreen::drawSkinBody2D(float x, float y, float w, float h, TextureId tid) {
	if (tid <= 0) return;
	minecraft->textures->bind(tid);
	const TextureData* tdata = minecraft->textures->getTemporaryTextureData(tid);
	float texW = 64.0f, texH = 64.0f;
	if (tdata && tdata->w > 0 && tdata->h > 0) {
		texW = (float)tdata->w;
		texH = (float)tdata->h;
	}
	float us = 1.0f / texW;
	float vs = 1.0f / texH;

	float bx = (float)(x + (w - 16) / 2);
	float by = (float)(y + 2);

	glEnable2(GL_TEXTURE_2D);
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);

	Tesselator& t = Tesselator::instance;

	auto drawPart = [&](float px, float py, float pw, float ph, float u, float v, float uw, float vh) {
		t.begin();
		t.vertexUV(px,      py + ph, 0.0f, u * us,         (v + vh) * vs);
		t.vertexUV(px + pw, py + ph, 0.0f, (u + uw) * us, (v + vh) * vs);
		t.vertexUV(px + pw, py,      0.0f, (u + uw) * us, v * vs);
		t.vertexUV(px,      py,      0.0f, u * us,         v * vs);
		t.draw();
	};

	// 1. Head Face (8x8 at offset 4,0)
	drawPart(bx + 4, by, 8, 8, 8, 8, 8, 8);
	// Head Overlay / Hat (40,8)
	float pad = 1.0f;
	drawPart(bx + 4 - pad, by - pad, 8 + pad * 2, 8 + pad * 2, 40, 8, 8, 8);

	// 2. Torso Body (8x12 at offset 4,8)
	drawPart(bx + 4, by + 8, 8, 12, 20, 20, 8, 12);
	if (texH >= 64) drawPart(bx + 4, by + 8, 8, 12, 20, 36, 8, 12);

	// 3. Right Arm (4x12 with slight outward pose matching 3D model)
	float armGap = 1.5f;
	drawPart(bx - armGap, by + 8, 4, 12, 44, 20, 4, 12);
	if (texH >= 64) drawPart(bx - armGap, by + 8, 4, 12, 44, 36, 4, 12);

	// 4. Left Arm (4x12 with slight outward pose matching 3D model)
	if (texH >= 64) {
		drawPart(bx + 12 + armGap, by + 8, 4, 12, 36, 52, 4, 12);
		drawPart(bx + 12 + armGap, by + 8, 4, 12, 52, 52, 4, 12);
	} else {
		drawPart(bx + 12 + armGap, by + 8, 4, 12, 44, 20, 4, 12);
	}

	// 5. Right Leg (4x12 at offset 4,20)
	drawPart(bx + 4, by + 20, 4, 12, 4, 20, 4, 12);
	if (texH >= 64) drawPart(bx + 4, by + 20, 4, 12, 4, 36, 4, 12);

	// 6. Left Leg (4x12 at offset 8,20)
	if (texH >= 64) {
		drawPart(bx + 8, by + 20, 4, 12, 20, 52, 4, 12);
		drawPart(bx + 8, by + 20, 4, 12, 4, 52, 4, 12);
	} else {
		drawPart(bx + 8, by + 20, 4, 12, 4, 20, 4, 12);
	}
}

void SkindexScreen::drawSkinBody3D(float x, float y, float w, float h, TextureId tid, bool isSlim) {
	if (tid <= 0) return;
	minecraft->textures->bind(tid);

	int skinW = 64, skinH = 64;
	const TextureData* tdata = minecraft->textures->getTemporaryTextureData(tid);
	if (tdata && tdata->w > 0 && tdata->h > 0) {
		skinW = tdata->w;
		skinH = tdata->h;
	}

	glEnable2(GL_DEPTH_TEST);
	GuiShader::unbind();
	glPushMatrix();

	float centerX = x + w / 2.0f;
	float centerY = y + h / 2.0f - 12.0f;
	glTranslatef(centerX, centerY, -200.0f);
	float ss = 14.5f;
	glScalef(-ss, ss, ss);
	glRotatef(180.0f, 0, 1, 0);
	glRotatef(10.0f, 1, 0, 0);
	glRotatef(15.0f, 0, 1, 0); // Facing slightly to the left, matching original reference!

	glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);
	HumanoidModel model(0.0f, 0.0f, skinW, skinH, isSlim);
	model.render(nullptr, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f);

	glPopMatrix();
	glDisable2(GL_DEPTH_TEST);
	GuiShader::bind();
}

void SkindexScreen::drawSkinCard(int x, int y, int w, int h, const std::string& skinPath, bool isSelected, const std::string& label) {
	// Card Background: Mid-grey slate fill so dark/black skins pop out with high contrast!
	fill(x, y, x + w, y + h, isSelected ? 0xf0707276 : 0xf0525458);
	
	// Card Border Highlight
	int borderColor = isSelected ? 0xffffd700 : 0x70808080;
	fill(x, y, x + w, y + 1, borderColor);
	fill(x, y + h - 1, x + w, y + h, borderColor);
	fill(x, y, x + 1, y + h, borderColor);
	fill(x + w - 1, y, x + w, y + h, borderColor);

	if (!skinPath.empty()) {
		TextureId tid = minecraft->textures->loadTexture(skinPath, false);
		if (tid > 0) {
			if (showFullBodyCards) {
				bool isSlim = (skinPath.find("cesar.png") != std::string::npos || skinPath.find("cesar malo.png") != std::string::npos);
				drawSkinBody3D((float)x, (float)y, (float)w, (float)h, tid, isSlim);
			} else {
				minecraft->textures->bind(tid);
				const TextureData* tdata = minecraft->textures->getTemporaryTextureData(tid);
				float texW = 64.0f, texH = 64.0f;
				if (tdata && tdata->w > 0 && tdata->h > 0) {
					texW = (float)tdata->w;
					texH = (float)tdata->h;
				}
				float us = 1.0f / texW;
				float vs = 1.0f / texH;

				float headSize = 22.0f;
				float hx = (float)(x + (w - (int)headSize) / 2);
				float hy = (float)(y + 4);

				glEnable2(GL_TEXTURE_2D);
				glEnable2(GL_BLEND);
				glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);

				Tesselator& t = Tesselator::instance;

				// Base Head Face (UV 8,8 to 16,16)
				t.begin();
				t.vertexUV(hx,            hy + headSize, 0.0f, 8.0f * us,  16.0f * vs);
				t.vertexUV(hx + headSize, hy + headSize, 0.0f, 16.0f * us, 16.0f * vs);
				t.vertexUV(hx + headSize, hy,            0.0f, 16.0f * us, 8.0f * vs);
				t.vertexUV(hx,            hy,            0.0f, 8.0f * us,  8.0f * vs);
				t.draw();

				// Hat Layer (UV 40,8 to 48,16)
				float pad = headSize * 0.125f;
				t.begin();
				t.vertexUV(hx - pad,            hy + headSize + pad, 0.0f, 40.0f * us, 16.0f * vs);
				t.vertexUV(hx + headSize + pad, hy + headSize + pad, 0.0f, 48.0f * us, 16.0f * vs);
				t.vertexUV(hx + headSize + pad, hy - pad,            0.0f, 48.0f * us, 8.0f * vs);
				t.vertexUV(hx - pad,            hy - pad,            0.0f, 40.0f * us, 8.0f * vs);
				t.draw();
			}
		}
	}

	// Label below card
	if (!label.empty()) {
		std::string shortLabel = label;
		if (shortLabel.length() > 7) {
			shortLabel = shortLabel.substr(0, 6) + "..";
		}
		int labelW = font->width(shortLabel);
		int labelX = x + (w - labelW) / 2;
		drawString(font, shortLabel, labelX, y + h - 11, isSelected ? 0xffff00 : 0xffffffff);
	}
}

void SkindexScreen::render(int xm, int ym, float a) {
	renderDirtBackground(0);

	// Update mouse drag rotation (REVERSED dx so dragging right turns character right)
	if (isDraggingRot) {
		int dx = xm - lastMouseX;
		playerRot -= dx * 1.5f;
		if (playerRot >= 360.0f) playerRot -= 360.0f;
		if (playerRot < 0.0f) playerRot += 360.0f;
		lastMouseX = xm;
	}

	// Header Title Bar
	drawCenteredString(font, I18n::get("skindex.title"), width / 2, 6, 0xffffff);

	int topY = 26;
	int leftBottomY = height - 6;
	int leftAvailableH = leftBottomY - topY;

	int rightBottomY = height - 28;
	int rightAvailableH = rightBottomY - topY;

	int leftX = 6;
	int leftW = (int)(width * 0.54f);
	int rightX = leftX + leftW + 6;
	int rightW = width - rightX - 6;

	// --- LEFT PANEL (SKIN PACKS LISTED VERTICALLY) ---
	fill(leftX, topY, leftX + leftW, topY + leftAvailableH, 0xf0404245);
	fill(leftX, topY, leftX + leftW, topY + 1, 0x60ffffff);
	fill(leftX, topY, leftX + 1, topY + leftAvailableH, 0x60ffffff);

	// Render Skin Packs one below the other
	int cardW = 46;
	int cardH = showFullBodyCards ? 54 : 46;
	int packRowH = showFullBodyCards ? 76 : 68;

	for (int p = 0; p < (int)skinPacks.size(); ++p) {
		int renderY = topY + 6 + p * packRowH;
		if (renderY + packRowH > topY + leftAvailableH) break;

		SkinPack& pack = skinPacks[p];
		bool isPackActive = (currentPackIndex == p);

		// Pack Header
		drawString(font, pack.name, leftX + 8, renderY, isPackActive ? 0xffff00 : 0xe0e0e0);

		// Import button for custom packs (not Default)
		if (pack.name != "Default") {
			int impBtnX = leftX + leftW - 68;
			int impBtnY = renderY - 2;
			int impBtnW = 60;
			int impBtnH = 14;

			bool isHover = (xm >= impBtnX && xm <= impBtnX + impBtnW && ym >= impBtnY && ym <= impBtnY + impBtnH);
			fill(impBtnX, impBtnY, impBtnX + impBtnW, impBtnY + impBtnH, isHover ? 0x90606060 : 0x70404040);
			fill(impBtnX, impBtnY, impBtnX + impBtnW, impBtnY + 1, 0x60ffffff);
			fill(impBtnX, impBtnY + impBtnH - 1, impBtnX + impBtnW, impBtnY + impBtnH, 0x60ffffff);
			fill(impBtnX, impBtnY, impBtnX + 1, impBtnY + impBtnH, 0x60ffffff);
			fill(impBtnX + impBtnW - 1, impBtnY, impBtnX + impBtnW, impBtnY + impBtnH, 0x60ffffff);

			drawCenteredString(font, I18n::get("gui.import"), impBtnX + impBtnW / 2, impBtnY + 3, isHover ? 0xffff00 : 0xffffff);
		}

		// Container box for pack skins
		int boxY = renderY + 12;
		int boxH = showFullBodyCards ? 58 : 50;
		fill(leftX + 6, boxY, leftX + leftW - 6, boxY + boxH, 0xf0303235);

		if (pack.skins.empty()) {
			drawString(font, I18n::get("skindex.emptyPack"), leftX + 12, boxY + boxH / 2 - 4, 0x888888);
		} else {
			for (int s = 0; s < (int)pack.skins.size(); ++s) {
				int cardX = leftX + 10 + s * (cardW + 5);
				int cardY = boxY + 2;

				if (cardX + cardW <= leftX + leftW - 8) {
					bool isSelected = (currentPackIndex == p && currentSkinIndex == s);
					
					std::string skinName = pack.skins[s];
					size_t slashPos = skinName.find_last_of("\\/");
					if (slashPos != std::string::npos) skinName = skinName.substr(slashPos + 1);
					if (skinName.length() > 4 && skinName.substr(skinName.length() - 4) == ".png") {
						skinName = skinName.substr(0, skinName.length() - 4);
					}

					drawSkinCard(cardX, cardY, cardW, cardH, pack.skins[s], isSelected, skinName);
				}
			}
		}
	}

	// --- RIGHT PANEL (PLAYER 3D PREVIEW & ACTIONS) ---
	fill(rightX, topY, rightX + rightW, topY + rightAvailableH, 0xf054565a);
	fill(rightX, topY, rightX + rightW, topY + 1, 0x60ffffff);
	fill(rightX + rightW - 1, topY, rightX + rightW, topY + rightAvailableH, 0x60ffffff);

	SkinPack& activePack = skinPacks[currentPackIndex];
	std::string currentSkin = activePack.skins.empty() ? "mob/char.png" : activePack.skins[currentSkinIndex];

	// Header inside Right Panel
	std::string currentSkinName = currentSkin;
	size_t sPos = currentSkinName.find_last_of("\\/");
	if (sPos != std::string::npos) currentSkinName = currentSkinName.substr(sPos + 1);
	if (currentSkinName.length() > 4 && currentSkinName.substr(currentSkinName.length() - 4) == ".png") {
		currentSkinName = currentSkinName.substr(0, currentSkinName.length() - 4);
	}
	drawCenteredString(font, currentSkinName, rightX + rightW / 2, topY + 6, 0xffff00);

	// Controls validation
	bool isBuiltin = false;
	if (!activePack.skins.empty()) {
		std::string fname = activePack.skins[currentSkinIndex];
		size_t pos = fname.find_last_of("\\/");
		if (pos != std::string::npos) fname = fname.substr(pos + 1);
		if (fname == "steve.png" || fname == "cesar.png" || fname == "cesar malo.png" || fname == "char.png") {
			isBuiltin = true;
		}
	} else {
		isBuiltin = true;
	}
	btnRename.active = !isBuiltin;
	btnDelete.active = !isBuiltin;
	btnConfirm.active = !activePack.skins.empty();

	// Render Buttons
	Screen::render(xm, ym, a);

	// 3D Player Model Render inside Right Panel
	if (!activePack.skins.empty()) {
		std::string newTexture = currentSkin;
		TextureId textureId = minecraft->textures->loadTexture(newTexture, false);
		minecraft->textures->bind(textureId);

		int skinW = 64, skinH = 64;
		const TextureData* tdata = minecraft->textures->getTemporaryTextureData(textureId);
		if (tdata) {
			skinW = tdata->w;
			skinH = tdata->h;
		}

		glEnable2(GL_DEPTH_TEST);
		GuiShader::unbind();
		glPushMatrix();
		
		int renderCenterY = topY + (rightAvailableH - 30) / 2 + 10;
		glTranslatef((float)(rightX + rightW / 2), (float)renderCenterY, -200);
		float ss = 55.0f;
		glScalef(-ss, ss, ss);
		glRotatef(180.0f, 0, 1, 0);
		glRotatef(15.0f, 1, 0, 0);
		glRotatef(playerRot, 0, 1, 0);

		glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);
		HumanoidModel model(0.0f, 0.0f, skinW, skinH, isSlimModel);
		model.render(nullptr, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f);

		glPopMatrix();
		glDisable2(GL_DEPTH_TEST);
		GuiShader::bind();

		// Rotation Indicator Slider Graphic
		int rotIndicatorY = topY + rightAvailableH - 36;
		drawCenteredString(font, "<------ ( o ) ------>", rightX + rightW / 2, rotIndicatorY, 0xaaaaaa);
	}
}

void SkindexScreen::buttonClicked(Button* button) {
	if (button->id >= 1000) {
		int p = (button->id - 1000) / 100;
		int s = (button->id - 1000) % 100;
		if (p >= 0 && p < (int)skinPacks.size()) {
			if (s >= 0 && s < (int)skinPacks[p].skins.size()) {
				currentPackIndex = p;
				currentSkinIndex = s;
				updateDefaultModelForSkin();
			}
		}
		return;
	}

	if (button->id == btnCloseHeader.id) {
		minecraft->setScreen(nullptr);
	} else if (button->id == btnConfirm.id) {
		SkinPack& activePack = skinPacks[currentPackIndex];
		if (!activePack.skins.empty()) {
			minecraft->options.set(OPTIONS_SKIN, activePack.skins[currentSkinIndex]);
			minecraft->options.set(OPTIONS_SKIN_MODEL, isSlimModel ? "slim" : "normal");
			minecraft->options.save();
			
			if (minecraft->player) {
				minecraft->player->textureName = activePack.skins[currentSkinIndex];
			}
		}
		minecraft->setScreen(nullptr);
	} else if (button->id == btnModel.id) {
		isSlimModel = !isSlimModel;
		updateModelButtonText();
	} else if (button->id == btnCardViewMode.id) {
		showFullBodyCards = !showFullBodyCards;
		setupPositions();
	} else if (button->id == btnAutoRotate.id) {
		autoRotate = !autoRotate;
		setupPositions();
	} else if (button->id == btnRename.id) {
		SkinPack& activePack = skinPacks[currentPackIndex];
		if (!activePack.skins.empty()) {
			minecraft->setScreen(new RenameSkinScreen(activePack.skins[currentSkinIndex]));
		}
	} else if (button->id == btnNewPack.id) {
		minecraft->setScreen(new NewPackScreen());
	} else if (button->id == btnDelete.id) {
		SkinPack& activePack = skinPacks[currentPackIndex];
		if (!activePack.skins.empty()) {
			std::string currentSkin = activePack.skins[currentSkinIndex];
			if (std::remove(currentSkin.c_str()) == 0) {
				scanSkins();
				currentSkinIndex = 0;
				setupPositions();
				updateDefaultModelForSkin();
			}
		}
	}
}

bool SkindexScreen::handleBackEvent(bool isDown) {
	if (isDown) return true;
	minecraft->setScreen(nullptr);
	return true;
}
