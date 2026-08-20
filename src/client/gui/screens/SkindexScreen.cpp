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

SkindexScreen::SkindexScreen(int packIdx, int skinIdx)
:	btnConfirm(0, 0, 0, 100, 22, I18n::get("gui.confirm")),
	btnCancel(1, 0, 0, 75, 22, "<- " + I18n::get("gui.back")),
	btnRename(7, 0, 0, 50, 18, I18n::get("gui.rename")),
	btnDelete(8, 0, 0, 50, 18, I18n::get("gui.delete")),
	btnNewPack(9, 0, 0, 80, 18, I18n::get("gui.newPack")),
	btnModel(10, 0, 0, 90, 18, I18n::get("skindex.model") + ": " + I18n::get("skindex.model.normal")),
	btnCloseHeader(11, 0, 0, 20, 18, "X"),
	btnCardViewMode(12, 0, 0, 95, 18, I18n::get("skindex.view") + ": " + I18n::get("skindex.view.body")),
	btnAutoRotate(13, 0, 0, 95, 18, I18n::get("skindex.autoRotate")),
	currentPackIndex(packIdx),
	currentSkinIndex(skinIdx),
	isSlimModel(false),
	showFullBodyCards(true),
	autoRotate(true),
	playerRot(0.0f),
	isDraggingRot(false),
	isDraggingScroll(false),
	lastMouseX(0),
	lastMouseY(0),
	packScrollOffset(0),
	modelNormal(nullptr),
	modelSlim(nullptr),
	showDeleteModal(false),
	pendingDeleteType(DELETE_NONE),
	pendingDeleteIndex(-1)
{
}

SkindexScreen::~SkindexScreen() {
	clearCardButtons();
	delete modelNormal;
	modelNormal = nullptr;
	delete modelSlim;
	modelSlim = nullptr;
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

	// Check if geometry is available from skins.json
	if (currentSkinIndex < (int)pack.skinGeometries.size() && !pack.skinGeometries[currentSkinIndex].empty()) {
		std::string geometry = pack.skinGeometries[currentSkinIndex];
		std::string lowerGeom = geometry;
		std::transform(lowerGeom.begin(), lowerGeom.end(), lowerGeom.begin(), ::tolower);
		isSlimModel = (lowerGeom.find("slim") != std::string::npos || lowerGeom.find("alex") != std::string::npos);
	} else {
		// Fallback to filename-based detection for legacy packs
		std::string currentSkin = pack.skins[currentSkinIndex];
		std::string lowerSkin = currentSkin;
		std::transform(lowerSkin.begin(), lowerSkin.end(), lowerSkin.begin(), ::tolower);
		if (lowerSkin.find("cesar") != std::string::npos || lowerSkin.find("alex") != std::string::npos || lowerSkin.find("slim") != std::string::npos) {
			isSlimModel = true;
		} else if (lowerSkin.find("steve") != std::string::npos) {
			isSlimModel = false;
		} else if (minecraft) {
			isSlimModel = (minecraft->options.getStringValue(OPTIONS_SKIN_MODEL) == "slim");
		}
	}
	updateModelButtonText();
}

void SkindexScreen::ensureSkinsDir() {
	createFolderIfNotExists("games");
	createFolderIfNotExists("games/com.mojang");
	createFolderIfNotExists("games/com.mojang/skins");
}

static std::string readAssetFileContent(Minecraft* minecraft, const std::string& path) {
	std::ifstream file(path.c_str());
	if (file.good()) {
		return std::string((std::istreambuf_iterator<char>(file)),
		                      std::istreambuf_iterator<char>());
	}
	if (minecraft && minecraft->platform()) {
		BinaryBlob blob = minecraft->platform()->readAssetFile(path);
		if (blob.data != nullptr && blob.size > 0) {
			std::string res((char*)blob.data, blob.size);
			return res;
		}
	}
	return "";
}

static bool isBedrockPack(Minecraft* minecraft, const std::string& dirPath) {
	std::string skinsJsonPath = dirPath + "/skins.json";
	std::ifstream file(skinsJsonPath.c_str());
	if (file.good()) return true;
	std::string content = readAssetFileContent(minecraft, skinsJsonPath);
	return !content.empty();
}

static bool readBedrockSkins(Minecraft* minecraft, const std::string& dirPath, SkinPack& pack) {
	std::string manifestContent = readAssetFileContent(minecraft, dirPath + "/manifest.json");
	if (!manifestContent.empty()) {
		size_t headerPos = manifestContent.find("\"header\"");
		size_t namePos = manifestContent.find("\"name\"", headerPos != std::string::npos ? headerPos : 0);
		if (namePos != std::string::npos) {
			size_t colon = manifestContent.find(":", namePos);
			if (colon != std::string::npos) {
				size_t q1 = manifestContent.find("\"", colon);
				if (q1 != std::string::npos) {
					size_t q2 = manifestContent.find("\"", q1 + 1);
					if (q2 != std::string::npos) {
						pack.displayName = manifestContent.substr(q1 + 1, q2 - q1 - 1);
					}
				}
			}
		}
	}
	if (pack.displayName.empty()) {
		pack.displayName = pack.name;
	}

	std::string content = readAssetFileContent(minecraft, dirPath + "/skins.json");
	if (content.empty()) return false;

	size_t skinsArrayPos = content.find("\"skins\"");
	if (skinsArrayPos == std::string::npos) return false;

	if (pack.displayName == pack.name) {
		auto getTopJsonProp = [&](const std::string& prop) -> std::string {
			size_t p = content.find("\"" + prop + "\"");
			if (p != std::string::npos && p < skinsArrayPos) {
				size_t colon = content.find(":", p);
				if (colon != std::string::npos && colon < skinsArrayPos) {
					size_t q1 = content.find("\"", colon);
					if (q1 != std::string::npos && q1 < skinsArrayPos) {
						size_t q2 = content.find("\"", q1 + 1);
						if (q2 != std::string::npos && q2 < skinsArrayPos) {
							return content.substr(q1 + 1, q2 - q1 - 1);
						}
					}
				}
			}
			return "";
		};
		std::string topName = getTopJsonProp("localization_name");
		if (topName.empty()) topName = getTopJsonProp("serialize_name");
		if (!topName.empty()) {
			std::replace(topName.begin(), topName.end(), '_', ' ');
			pack.displayName = topName;
		}
	}

	auto getJsonProp = [](const std::string& json, const std::string& prop) -> std::string {
		size_t p = json.find("\"" + prop + "\"");
		if (p == std::string::npos) return "";
		size_t colon = json.find(":", p);
		if (colon == std::string::npos) return "";
		size_t q1 = json.find("\"", colon);
		if (q1 == std::string::npos) return "";
		size_t q2 = json.find("\"", q1 + 1);
		if (q2 == std::string::npos) return "";
		return json.substr(q1 + 1, q2 - q1 - 1);
	};

	size_t pos = skinsArrayPos;
	while ((pos = content.find('{', pos)) != std::string::npos) {
		size_t endObj = content.find('}', pos);
		if (endObj == std::string::npos) break;

		std::string objStr = content.substr(pos, endObj - pos + 1);

		std::string texture = getJsonProp(objStr, "texture");
		std::string locName = getJsonProp(objStr, "localization_name");
		std::string geometry = getJsonProp(objStr, "geometry");

		if (!texture.empty()) {
			pack.skins.push_back(dirPath + "/" + texture);

			std::string dispName = locName;
			if (dispName.empty()) {
				dispName = texture;
				size_t dot = dispName.rfind('.');
				if (dot != std::string::npos) dispName = dispName.substr(0, dot);
			} else {
				std::replace(dispName.begin(), dispName.end(), '_', ' ');
			}
			pack.skinDisplayNames.push_back(dispName);
			pack.skinGeometries.push_back(geometry);
		}

		pos = endObj + 1;
	}

	std::string langContent = readAssetFileContent(minecraft, dirPath + "/texts/en_US.lang");
	if (!langContent.empty()) {
		for (size_t i = 0; i < pack.skinDisplayNames.size(); ++i) {
			std::string key = pack.skinDisplayNames[i];
			key.erase(std::remove(key.begin(), key.end(), ' '), key.end());
			
			size_t keyPos = langContent.find(key + "=");
			if (keyPos != std::string::npos) {
				size_t valStart = keyPos + key.length() + 1;
				size_t valEnd = langContent.find_first_of("\r\n", valStart);
				if (valEnd != std::string::npos) {
					pack.skinDisplayNames[i] = langContent.substr(valStart, valEnd - valStart);
				}
			}
		}
	}

	return !pack.skins.empty();
}

static const std::vector<std::string> g_internalPacks = {
	"Default",
	"Backroom_Entity_4_Skins",
	"Best_MEME_Skin_+67",
	"Dark_Neeko_5_Skins",
	"Fashion_PVP_Skins_10_Skins",
	"GroxSkinpack",
	"Jujutsu Anime V2",
	"Kakashi",
	"Neon_Pulse_13_Skins",
	"Star vs the forces of evil",
	"The Corrputed V1",
	"Trendy Fashion Teens - v1.3",
	"Ultimate-SpiderMan-Skinpack"
};

void SkindexScreen::scanSkins() {
	skinPacks.clear();
	ensureSkinsDir();

	// Helper function to check if a pack exists in data/images/skins (internal)
	auto packExistsInData = [](const std::string& packName) -> bool {
#ifdef _WIN32
		std::string dataPath = "data\\images\\skins\\" + packName;
		DWORD attrib = GetFileAttributesA(dataPath.c_str());
		return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
#else
		for (const auto& p : g_internalPacks) {
			if (p == packName) return true;
		}
		std::string dataPath = "data/images/skins/" + packName;
		DIR* dir = opendir(dataPath.c_str());
		if (dir) {
			closedir(dir);
			return true;
		}
		return false;
#endif
	};

	// 1. Scan Internal Packs from data/images/skins
#ifdef _WIN32
	WIN32_FIND_DATAA findDirData;
	HANDLE hFindDir = FindFirstFileA("data\\images\\skins\\*", &findDirData);
	if (hFindDir != INVALID_HANDLE_VALUE) {
		do {
			if (findDirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				std::string dirName = findDirData.cFileName;
				if (dirName != "." && dirName != ".." && dirName != "Personalizados") {
					SkinPack pack;
					pack.name = dirName;
					pack.isInternal = true;
					
					std::string fullDirPath = "data/images/skins/" + dirName;
					
					if (isBedrockPack(minecraft, fullDirPath)) {
						readBedrockSkins(minecraft, fullDirPath, pack);
					} else {
						std::string searchPath = "data\\images\\skins\\" + dirName + "\\*.png";
						WIN32_FIND_DATAA findFileData;
						HANDLE hFindFile = FindFirstFileA(searchPath.c_str(), &findFileData);
						if (hFindFile != INVALID_HANDLE_VALUE) {
							do {
								if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
									pack.skins.push_back("data/images/skins/" + dirName + "/" + findFileData.cFileName);
								}
							} while (FindNextFileA(hFindFile, &findFileData) != 0);
							FindClose(hFindFile);
						}
					}
					
					if (!pack.skins.empty()) {
						skinPacks.push_back(pack);
					}
				}
			}
		} while (FindNextFileA(hFindDir, &findDirData) != 0);
		FindClose(hFindDir);
	}
#else
	DIR* dirData = opendir("data/images/skins");
	if (dirData != NULL) {
		struct dirent* ent;
		while ((ent = readdir(dirData)) != NULL) {
			std::string dirName = ent->d_name;
			if (dirName != "." && dirName != ".." && dirName != "Personalizados") {
				std::string fullDirPath = "data/images/skins/" + dirName;
				SkinPack pack;
				pack.name = dirName;
				pack.isInternal = true;
				
				if (isBedrockPack(minecraft, fullDirPath)) {
					readBedrockSkins(minecraft, fullDirPath, pack);
				} else {
					DIR* subDir = opendir(fullDirPath.c_str());
					if (subDir != NULL) {
						struct dirent* subEnt;
						while ((subEnt = readdir(subDir)) != NULL) {
							std::string fileName = subEnt->d_name;
							if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == ".png") {
								pack.skins.push_back(fullDirPath + "/" + fileName);
							}
						}
						closedir(subDir);
					}
				}
				if (!pack.skins.empty()) {
					skinPacks.push_back(pack);
				}
			}
		}
		closedir(dirData);
	} else {
		// Fallback for Android APK assets
		for (const auto& dirName : g_internalPacks) {
			std::string fullDirPath = "data/images/skins/" + dirName;
			SkinPack pack;
			pack.name = dirName;
			pack.isInternal = true;
			if (isBedrockPack(minecraft, fullDirPath)) {
				readBedrockSkins(minecraft, fullDirPath, pack);
			}
			if (!pack.skins.empty()) {
				skinPacks.push_back(pack);
			}
		}
	}
#endif

	// 2. Scan Custom User Packs from games/com.mojang/skins
#ifdef _WIN32
	hFindDir = FindFirstFileA("games\\com.mojang\\skins\\*", &findDirData);
	if (hFindDir != INVALID_HANDLE_VALUE) {
		do {
			if (findDirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				std::string dirName = findDirData.cFileName;
				if (dirName != "." && dirName != ".." && dirName != "Personalizados") {
					if (packExistsInData(dirName)) continue;

					SkinPack pack;
					pack.name = dirName;
					pack.isInternal = false;
					
					std::string fullDirPath = "games/com.mojang/skins/" + dirName;
					
					if (isBedrockPack(minecraft, fullDirPath)) {
						readBedrockSkins(minecraft, fullDirPath, pack);
					} else {
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
					}
					
					skinPacks.push_back(pack);
				}
			}
		} while (FindNextFileA(hFindDir, &findDirData) != 0);
		FindClose(hFindDir);
	}
#else
	DIR* dirCustom = opendir("games/com.mojang/skins");
	if (dirCustom != NULL) {
		struct dirent* ent;
		while ((ent = readdir(dirCustom)) != NULL) {
			std::string dirName = ent->d_name;
			if (dirName != "." && dirName != ".." && dirName != "Personalizados") {
				if (packExistsInData(dirName)) continue;

				std::string fullDirPath = "games/com.mojang/skins/" + dirName;
				SkinPack pack;
				pack.name = dirName;
				pack.isInternal = false;
				
				if (isBedrockPack(minecraft, fullDirPath)) {
					readBedrockSkins(minecraft, fullDirPath, pack);
				} else {
					DIR* subDir = opendir(fullDirPath.c_str());
					if (subDir != NULL) {
						struct dirent* subEnt;
						while ((subEnt = readdir(subDir)) != NULL) {
							std::string fileName = subEnt->d_name;
							if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == ".png") {
								pack.skins.push_back(fullDirPath + "/" + fileName);
							}
						}
						closedir(subDir);
					}
				}
				skinPacks.push_back(pack);
			}
		}
		closedir(dirCustom);
	}
#endif

	// Sort packs: "Default" first, then custom packs, then internal packs alphabetically
	std::sort(skinPacks.begin(), skinPacks.end(), [](const SkinPack& a, const SkinPack& b) {
		if (a.name == "Default" && b.name != "Default") return true;
		if (b.name == "Default" && a.name != "Default") return false;

		if (!a.isInternal && b.isInternal) return true;
		if (a.isInternal && !b.isInternal) return false;
		
		return a.name < b.name;
	});

	if (skinPacks.empty()) {
		SkinPack pack;
		pack.name = "Default";
		pack.isInternal = true;
		pack.skins.push_back("data/images/skins/Default/steve.png");
		pack.skins.push_back("data/images/skins/Default/cesar.png");
		pack.skins.push_back("data/images/skins/Default/cesarWhite.png");
		skinPacks.push_back(pack);
	}
}

void SkindexScreen::init() {
	if (!modelNormal) modelNormal = new HumanoidModel(0.0f, 0.0f, 64, 64, false);
	if (!modelSlim) modelSlim = new HumanoidModel(0.0f, 0.0f, 64, 64, true);

	scanSkins();

	if (currentPackIndex >= 0 && currentSkinIndex >= 0) {
		if (currentPackIndex >= (int)skinPacks.size()) currentPackIndex = 0;
		if (!skinPacks.empty()) {
			if (currentSkinIndex >= (int)skinPacks[currentPackIndex].skins.size()) {
				currentSkinIndex = 0;
			}
			int cardsPerPage = ((int)(width * 0.54f) - 24) / 51;
			if (cardsPerPage > 0) {
				skinPacks[currentPackIndex].pageOffset = currentSkinIndex / cardsPerPage;
			}
		}
	} else {
		std::string currentSkin = minecraft->options.getStringValue(OPTIONS_SKIN);
#ifdef ANDROID
		if (currentSkin == "" || currentSkin == "Default") currentSkin = "images/skins/steve.png";
#else
		if (currentSkin == "" || currentSkin == "Default" || currentSkin == "games/com.mojang/skins/Default/steve.png") currentSkin = "data/images/skins/Default/steve.png";
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

void SkindexScreen::mouseWheel(int dx, int dy, int xm, int ym) {
	Screen::mouseWheel(dx, dy, xm, ym);
	
	// Only allow scroll if mouse is in left panel (packs section)
	int topY = 26;
	int leftX = 6;
	int leftW = (int)(width * 0.54f);
	int leftBottomY = height - 6;
	
	if (xm < leftX || xm > leftX + leftW || ym < topY || ym > leftBottomY) {
		return;
	}
	
	int leftAvailableH = leftBottomY - topY;
	int packRowH = showFullBodyCards ? 76 : 68;
	
	// Calculate max scroll offset
	int maxVisiblePacks = (leftAvailableH - 6) / packRowH;
	int maxScrollOffset = (int)skinPacks.size() - maxVisiblePacks;
	if (maxScrollOffset < 0) maxScrollOffset = 0;
	
	// Adjust scroll offset
	packScrollOffset -= dy;
	
	// Clamp scroll offset
	if (packScrollOffset < 0) packScrollOffset = 0;
	if (packScrollOffset > maxScrollOffset) packScrollOffset = maxScrollOffset;
}

static void addSkinToSkinsJson(const std::string& packDir, const std::string& textureFilename) {
	std::string skinsJsonPath = packDir + "/skins.json";
	std::ifstream inFile(skinsJsonPath);
	std::string content = "";
	if (inFile.good()) {
		content = std::string((std::istreambuf_iterator<char>(inFile)),
		                      std::istreambuf_iterator<char>());
		inFile.close();
	}

	std::string skinName = textureFilename;
	size_t dot = skinName.rfind('.');
	if (dot != std::string::npos) skinName = skinName.substr(0, dot);

	std::string lowerName = skinName;
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
	std::string geometry = (lowerName.find("cesar") != std::string::npos || lowerName.find("alex") != std::string::npos || lowerName.find("slim") != std::string::npos) 
		? "geometry.humanoid.customSlim" : "geometry.humanoid.custom";

	std::string newSkinEntry = "    {\n"
	                           "      \"localization_name\": \"" + skinName + "\",\n"
	                           "      \"geometry\": \"" + geometry + "\",\n"
	                           "      \"texture\": \"" + textureFilename + "\",\n"
	                           "      \"type\": \"free\"\n"
	                           "    }";

	size_t arrayPos = content.find("\"skins\"");
	if (arrayPos == std::string::npos) {
		size_t slash = packDir.find_last_of("\\/");
		std::string packName = (slash != std::string::npos) ? packDir.substr(slash + 1) : "CustomPack";
		std::ofstream outFile(skinsJsonPath);
		if (outFile.is_open()) {
			outFile << "{\n"
			        << "  \"serialize_name\": \"" << packName << "\",\n"
			        << "  \"localization_name\": \"" << packName << "\",\n"
			        << "  \"skins\": [\n"
			        << newSkinEntry << "\n"
			        << "  ]\n"
			        << "}\n";
			outFile.close();
		}
	} else {
		size_t bracketPos = content.find("[", arrayPos);
		if (bracketPos != std::string::npos) {
			size_t closeBracket = content.find("]", bracketPos);
			if (closeBracket != std::string::npos) {
				std::string skinsInside = content.substr(bracketPos + 1, closeBracket - bracketPos - 1);
				size_t firstChar = skinsInside.find_first_not_of(" \t\n\r");
				std::string updatedSkins;
				if (firstChar == std::string::npos) {
					updatedSkins = "\n" + newSkinEntry + "\n  ";
				} else {
					updatedSkins = skinsInside;
					size_t lastChar = updatedSkins.find_last_not_of(" \t\n\r");
					if (lastChar != std::string::npos) updatedSkins = updatedSkins.substr(0, lastChar + 1);
					updatedSkins += ",\n" + newSkinEntry + "\n  ";
				}
				content.replace(bracketPos + 1, closeBracket - bracketPos - 1, updatedSkins);
				std::ofstream outFile(skinsJsonPath);
				if (outFile.is_open()) {
					outFile << content;
					outFile.close();
				}
			}
		}
	}
}

static void removeSkinFromSkinsJson(const std::string& packDir, const std::string& textureFilename) {
	std::string skinsJsonPath = packDir + "/skins.json";
	std::ifstream inFile(skinsJsonPath);
	if (!inFile.good()) return;

	std::string content((std::istreambuf_iterator<char>(inFile)),
	                    std::istreambuf_iterator<char>());
	inFile.close();

	size_t arrayPos = content.find("\"skins\"");
	if (arrayPos == std::string::npos) return;

	size_t bracketPos = content.find("[", arrayPos);
	if (bracketPos == std::string::npos) return;

	size_t closeBracket = content.find("]", bracketPos);
	if (closeBracket == std::string::npos) return;

	std::string skinsArrayStr = content.substr(bracketPos + 1, closeBracket - bracketPos - 1);

	size_t texPos = skinsArrayStr.find("\"" + textureFilename + "\"");
	if (texPos == std::string::npos) {
		std::string nameNoExt = textureFilename;
		size_t dot = nameNoExt.rfind('.');
		if (dot != std::string::npos) nameNoExt = nameNoExt.substr(0, dot);
		texPos = skinsArrayStr.find("\"" + nameNoExt + "\"");
	}

	if (texPos != std::string::npos) {
		size_t startObj = skinsArrayStr.rfind('{', texPos);
		size_t endObj = skinsArrayStr.find('}', texPos);
		if (startObj != std::string::npos && endObj != std::string::npos) {
			size_t eraseStart = startObj;
			size_t eraseEnd = endObj + 1;

			size_t commaBefore = skinsArrayStr.rfind(',', startObj);
			if (commaBefore != std::string::npos) {
				bool onlySpaces = true;
				for (size_t k = commaBefore + 1; k < startObj; ++k) {
					if (!isspace((unsigned char)skinsArrayStr[k])) { onlySpaces = false; break; }
				}
				if (onlySpaces) eraseStart = commaBefore;
			} else {
				size_t commaAfter = skinsArrayStr.find(',', endObj);
				if (commaAfter != std::string::npos) {
					bool onlySpaces = true;
					for (size_t k = endObj + 1; k < commaAfter; ++k) {
						if (!isspace((unsigned char)skinsArrayStr[k])) { onlySpaces = false; break; }
					}
					if (onlySpaces) eraseEnd = commaAfter + 1;
				}
			}

			skinsArrayStr.erase(eraseStart, eraseEnd - eraseStart);
			content.replace(bracketPos + 1, closeBracket - bracketPos - 1, skinsArrayStr);

			std::ofstream outFile(skinsJsonPath);
			if (outFile.is_open()) {
				outFile << content;
				outFile.close();
			}
		}
	}
}

static void updateGeometryInSkinsJson(const std::string& packDir, const std::string& textureFilename, bool isSlim) {
	std::string skinsJsonPath = packDir + "/skins.json";
	std::ifstream inFile(skinsJsonPath);
	if (!inFile.good()) return;

	std::string content((std::istreambuf_iterator<char>(inFile)),
	                    std::istreambuf_iterator<char>());
	inFile.close();

	std::string newGeometry = isSlim ? "geometry.humanoid.customSlim" : "geometry.humanoid.custom";

	size_t texPos = content.find("\"" + textureFilename + "\"");
	if (texPos != std::string::npos) {
		size_t startObj = content.rfind('{', texPos);
		size_t endObj = content.find('}', texPos);
		if (startObj != std::string::npos && endObj != std::string::npos && startObj < endObj) {
			std::string objStr = content.substr(startObj, endObj - startObj + 1);
			size_t geomPos = objStr.find("\"geometry\"");
			if (geomPos != std::string::npos) {
				size_t colon = objStr.find(":", geomPos);
				if (colon != std::string::npos) {
					size_t q1 = objStr.find("\"", colon);
					if (q1 != std::string::npos) {
						size_t q2 = objStr.find("\"", q1 + 1);
						if (q2 != std::string::npos) {
							std::string updatedObj = objStr;
							updatedObj.replace(q1 + 1, q2 - q1 - 1, newGeometry);
							content.replace(startObj, endObj - startObj + 1, updatedObj);
						}
					}
				}
			} else {
				std::string updatedObj = objStr;
				size_t firstBrace = updatedObj.find('{');
				if (firstBrace != std::string::npos) {
					std::string insertStr = "\n      \"geometry\": \"" + newGeometry + "\",";
					updatedObj.insert(firstBrace + 1, insertStr);
					content.replace(startObj, endObj - startObj + 1, updatedObj);
				}
			}

			std::ofstream outFile(skinsJsonPath);
			if (outFile.is_open()) {
				outFile << content;
				outFile.close();
			}
		}
	}
}

static void deletePackDirectory(const std::string& packDir) {
#ifdef _WIN32
	std::string searchPath = packDir + "\\*";
	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			std::string name = findData.cFileName;
			if (name != "." && name != "..") {
				std::string fullPath = packDir + "\\" + name;
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					deletePackDirectory(fullPath);
				} else {
					DeleteFileA(fullPath.c_str());
				}
			}
		} while (FindNextFileA(hFind, &findData) != 0);
		FindClose(hFind);
	}
	RemoveDirectoryA(packDir.c_str());
#else
	DIR* dir = opendir(packDir.c_str());
	if (dir != NULL) {
		struct dirent* ent;
		while ((ent = readdir(dir)) != NULL) {
			std::string name = ent->d_name;
			if (name != "." && name != "..") {
				std::string fullPath = packDir + "/" + name;
				DIR* checkSub = opendir(fullPath.c_str());
				if (checkSub != NULL) {
					closedir(checkSub);
					deletePackDirectory(fullPath);
				} else {
					std::remove(fullPath.c_str());
				}
			}
		}
		closedir(dir);
	}
	rmdir(packDir.c_str());
#endif
}

void SkindexScreen::deletePack(int packIndex) {
	if (skinPacks.empty() || packIndex < 0 || packIndex >= (int)skinPacks.size()) return;
	SkinPack& pack = skinPacks[packIndex];
	if (pack.isInternal) return;

	std::string packDir = "games/com.mojang/skins/" + pack.name;
	deletePackDirectory(packDir);

	scanSkins();
	currentPackIndex = 0;
	currentSkinIndex = 0;
	setupPositions();
	updateDefaultModelForSkin();
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
			addSkinToSkinsJson(targetDir, filename);
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

	if (showDeleteModal) {
		int modalW = 220;
		int modalH = 80;
		int modalX = (width - modalW) / 2;
		int modalY = (height - modalH) / 2;

		int btnW = 75;
		int btnH = 20;
		int btnY = modalY + modalH - 26;

		int btnYesX = modalX + 20;
		int btnNoX = modalX + modalW - 20 - btnW;

		// Yes / Confirm button
		if (x >= btnYesX && x <= btnYesX + btnW && y >= btnY && y <= btnY + btnH) {
			if (minecraft && minecraft->soundEngine) {
				minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
			}

			if (pendingDeleteType == DELETE_PACK) {
				deletePack(pendingDeleteIndex);
			} else if (pendingDeleteType == DELETE_SKIN) {
				if (!skinPacks.empty() && currentPackIndex >= 0 && currentPackIndex < (int)skinPacks.size()) {
					SkinPack& activePack = skinPacks[currentPackIndex];
					if (!activePack.skins.empty() && pendingDeleteIndex >= 0 && pendingDeleteIndex < (int)activePack.skins.size()) {
						std::string currentSkin = activePack.skins[pendingDeleteIndex];

						std::string winPath = currentSkin;
						std::replace(winPath.begin(), winPath.end(), '/', '\\');
						std::remove(winPath.c_str());
						std::remove(currentSkin.c_str());

						std::string textureFilename = currentSkin;
						size_t slashPos = textureFilename.find_last_of("\\/");
						if (slashPos != std::string::npos) {
							textureFilename = textureFilename.substr(slashPos + 1);
						}

						std::string packDir = "games/com.mojang/skins/" + activePack.name;
						removeSkinFromSkinsJson(packDir, textureFilename);

						scanSkins();
						currentSkinIndex = 0;
						setupPositions();
						updateDefaultModelForSkin();
					}
				}
			}

			showDeleteModal = false;
			pendingDeleteType = DELETE_NONE;
			pendingDeleteIndex = -1;
			return;
		}

		// No / Cancel button
		if (x >= btnNoX && x <= btnNoX + btnW && y >= btnY && y <= btnY + btnH) {
			if (minecraft && minecraft->soundEngine) {
				minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
			}
			showDeleteModal = false;
			pendingDeleteType = DELETE_NONE;
			pendingDeleteIndex = -1;
			return;
		}

		return;
	}

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

		if (y >= topY && y <= topY + leftAvailableH) {
			// Check click on Scrollbar track / thumb
			int scrollbarWidth = 4;
			int scrollbarX = leftX + leftW - 8;
			int scrollbarHeight = leftAvailableH - 4;
			int scrollbarY = topY + 2;

			if (x >= scrollbarX - 4 && x <= leftX + leftW && y >= scrollbarY && y <= scrollbarY + scrollbarHeight) {
				isDraggingScroll = true;
				lastMouseY = y;

				int maxVisiblePacks = (leftAvailableH - 6) / packRowH;
				int maxScrollOffset = (int)skinPacks.size() - maxVisiblePacks;
				if (maxScrollOffset > 0) {
					float clickRatio = (float)(y - scrollbarY) / (float)scrollbarHeight;
					if (clickRatio < 0.0f) clickRatio = 0.0f;
					if (clickRatio > 1.0f) clickRatio = 1.0f;
					packScrollOffset = (int)(clickRatio * maxScrollOffset + 0.5f);
					if (packScrollOffset < 0) packScrollOffset = 0;
					if (packScrollOffset > maxScrollOffset) packScrollOffset = maxScrollOffset;
				}
				return;
			}

			for (int p = 0; p < (int)skinPacks.size(); ++p) {
				int renderY = topY + 6 + (p - packScrollOffset) * packRowH;
				
				// Skip if pack is not visible
				if (renderY + packRowH < topY) continue;
				if (renderY > topY + leftAvailableH) break;

				SkinPack& pack = skinPacks[p];
				int boxY = renderY + 12;

				int totalSkins = (int)pack.skins.size();
				int cardsPerPage = (leftW - 24) / (cardW + 5);
				if (cardsPerPage < 1) cardsPerPage = 1;
				int maxPage = (totalSkins > 0) ? (totalSkins - 1) / cardsPerPage : 0;

				// 1. Pagination buttons click check
				if (maxPage > 0) {
					int rightOffset = !pack.isInternal ? 144 : 16;
					int nextBtnX = leftX + leftW - rightOffset - 16;
					int prevBtnX = nextBtnX - 44;
					int pageBtnY = renderY - 2;
					int pageBtnW = 14;
					int pageBtnH = 14;

					// Previous Page Button [<]
					if (x >= prevBtnX && x <= prevBtnX + pageBtnW && y >= pageBtnY && y <= pageBtnY + pageBtnH) {
						pack.pageOffset--;
						if (pack.pageOffset < 0) pack.pageOffset = maxPage;
						if (minecraft && minecraft->soundEngine) {
							minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
						}
						return;
					}

					// Next Page Button [>]
					if (x >= nextBtnX && x <= nextBtnX + pageBtnW && y >= pageBtnY && y <= pageBtnY + pageBtnH) {
						pack.pageOffset++;
						if (pack.pageOffset > maxPage) pack.pageOffset = 0;
						if (minecraft && minecraft->soundEngine) {
							minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
						}
						return;
					}
				}

				// 2. Direct card click check
				int startSkin = pack.pageOffset * cardsPerPage;
				int endSkin = std::min(totalSkins, startSkin + cardsPerPage);

				for (int s = startSkin; s < endSkin; ++s) {
					int cardIndex = s - startSkin;
					int cardX = leftX + 10 + cardIndex * (cardW + 5);
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

				// 3. Check custom pack buttons ([ Importar ] and [ Eliminar ])
				if (!pack.isInternal) {
					// [ Importar ] button
					int impBtnX = leftX + leftW - 74;
					int impBtnY = renderY - 2;
					int impBtnW = 60;
					int impBtnH = 14;

					if (x >= impBtnX && x <= impBtnX + impBtnW && y >= impBtnY && y <= impBtnY + impBtnH) {
						importSkinToPack(p);
						return;
					}

					// [ Eliminar ] button
					int delBtnX = leftX + leftW - 138;
					int delBtnY = renderY - 2;
					int delBtnW = 60;
					int delBtnH = 14;

					if (x >= delBtnX && x <= delBtnX + delBtnW && y >= delBtnY && y <= delBtnY + delBtnH) {
						showDeleteModal = true;
						pendingDeleteType = DELETE_PACK;
						pendingDeleteIndex = p;
						if (minecraft && minecraft->soundEngine) {
							minecraft->soundEngine->playUI("random.click", 1.0f, 1.0f);
						}
						return;
					}
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
		isDraggingScroll = false;

		// Check skin card selection on mouse release for 100% click responsiveness
		int topY = 26;
		int leftBottomY = height - 6;
		int leftAvailableH = leftBottomY - topY;
		int leftX = 6;
		int leftW = (int)(width * 0.54f);

		int cardW = 46;
		int cardH = showFullBodyCards ? 54 : 46;
		int packRowH = showFullBodyCards ? 76 : 68;

		if (y >= topY && y <= topY + leftAvailableH) {
			for (int p = 0; p < (int)skinPacks.size(); ++p) {
				int renderY = topY + 6 + (p - packScrollOffset) * packRowH;
				
				// Skip if pack is not visible
				if (renderY + packRowH < topY) continue;
				if (renderY > topY + leftAvailableH) break;

				SkinPack& pack = skinPacks[p];
				int boxY = renderY + 12;

				int totalSkins = (int)pack.skins.size();
				int cardsPerPage = (leftW - 24) / (cardW + 5);
				if (cardsPerPage < 1) cardsPerPage = 1;

				int startSkin = pack.pageOffset * cardsPerPage;
				int endSkin = std::min(totalSkins, startSkin + cardsPerPage);

				for (int s = startSkin; s < endSkin; ++s) {
					int cardIndex = s - startSkin;
					int cardX = leftX + 10 + cardIndex * (cardW + 5);
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
	HumanoidModel* model = isSlim ? modelSlim : modelNormal;
	if (model) {
		model->render(nullptr, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f);
	}

	glPopMatrix();
	glDisable2(GL_DEPTH_TEST);
	GuiShader::bind();
}

void SkindexScreen::drawSkinCard(int x, int y, int w, int h, const std::string& skinPath, bool isSelected, const std::string& label, bool isSlim) {
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

	int topY = 26;
	int leftBottomY = height - 6;
	int leftAvailableH = leftBottomY - topY;

	// Update mouse drag rotation (REVERSED dx so dragging right turns character right)
	if (isDraggingRot) {
		int dx = xm - lastMouseX;
		playerRot -= dx * 1.5f;
		if (playerRot >= 360.0f) playerRot -= 360.0f;
		if (playerRot < 0.0f) playerRot += 360.0f;
		lastMouseX = xm;
	}

	// Update mouse drag scrollbar
	if (isDraggingScroll) {
		int packRowH = showFullBodyCards ? 76 : 68;
		int maxVisiblePacks = (leftAvailableH - 6) / packRowH;
		int maxScrollOffset = (int)skinPacks.size() - maxVisiblePacks;
		if (maxScrollOffset > 0) {
			int scrollbarHeight = leftAvailableH - 4;
			int scrollbarY = topY + 2;
			float ratio = (float)(ym - scrollbarY) / (float)scrollbarHeight;
			if (ratio < 0.0f) ratio = 0.0f;
			if (ratio > 1.0f) ratio = 1.0f;
			packScrollOffset = (int)(ratio * maxScrollOffset + 0.5f);
			if (packScrollOffset < 0) packScrollOffset = 0;
			if (packScrollOffset > maxScrollOffset) packScrollOffset = maxScrollOffset;
		}
		lastMouseY = ym;
	}

	// Header Title Bar
	drawCenteredString(font, I18n::get("skindex.title"), width / 2, 6, 0xffffff);

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

	// Scrollbar (sleek 4px thin scrollbar with proper margins)
	int scrollbarWidth = 4;
	int scrollbarX = leftX + leftW - 8;
	int scrollbarHeight = leftAvailableH - 4;
	int scrollbarY = topY + 2;
	
	// Calculate scrollbar thumb position and size
	int maxVisiblePacks = (leftAvailableH - 6) / packRowH;
	int maxScrollOffset = (int)skinPacks.size() - maxVisiblePacks;
	if (maxScrollOffset < 0) maxScrollOffset = 0;
	
	float scrollRatio = (maxScrollOffset > 0) ? (float)packScrollOffset / maxScrollOffset : 0.0f;
	float thumbRatio = (maxScrollOffset > 0) ? (float)maxVisiblePacks / (float)skinPacks.size() : 1.0f;
	if (thumbRatio > 1.0f) thumbRatio = 1.0f;
	
	int thumbHeight = (int)(scrollbarHeight * thumbRatio);
	if (thumbHeight < 12) thumbHeight = 12;
	int thumbY = scrollbarY + (int)((scrollbarHeight - thumbHeight) * scrollRatio);
	
	// Draw scrollbar background track
	fill(scrollbarX, scrollbarY, scrollbarX + scrollbarWidth, scrollbarY + scrollbarHeight, 0xf0202225);
	
	// Draw scrollbar thumb indicator
	fill(scrollbarX, thumbY, scrollbarX + scrollbarWidth, thumbY + thumbHeight, 0xf0707070);
	fill(scrollbarX, thumbY, scrollbarX + scrollbarWidth, thumbY + 1, 0x90ffffff);
	fill(scrollbarX, thumbY + thumbHeight - 1, scrollbarX + scrollbarWidth, thumbY + thumbHeight, 0x90ffffff);

	// Enable OpenGL Scissor test to clip any pack items scrolling out of the left panel
	glEnable2(GL_SCISSOR_TEST);
	int clipX = (int)(Gui::GuiScale * leftX);
	int clipY = minecraft->height - (int)(Gui::GuiScale * (topY + leftAvailableH));
	int clipW = (int)(Gui::GuiScale * leftW);
	int clipH = (int)(Gui::GuiScale * leftAvailableH);
	glScissor(clipX, clipY, clipW, clipH);

	for (int p = 0; p < (int)skinPacks.size(); ++p) {
		int renderY = topY + 6 + (p - packScrollOffset) * packRowH;
		
		// Skip rendering if pack is completely above visible area
		if (renderY + packRowH < topY - 20) continue;
		// Stop rendering if pack is completely below visible area
		if (renderY > topY + leftAvailableH + 20) break;

		SkinPack& pack = skinPacks[p];
		bool isPackActive = (currentPackIndex == p);

		// Pack Header
		drawString(font, pack.displayName, leftX + 8, renderY, isPackActive ? 0xffff00 : 0xe0e0e0);

		int totalSkins = (int)pack.skins.size();
		int cardsPerPage = (leftW - 24) / (cardW + 5);
		if (cardsPerPage < 1) cardsPerPage = 1;
		int maxPage = (totalSkins > 0) ? (totalSkins - 1) / cardsPerPage : 0;

		// Clamp pageOffset
		if (pack.pageOffset > maxPage) pack.pageOffset = maxPage;
		if (pack.pageOffset < 0) pack.pageOffset = 0;

		// Buttons for custom packs (Importar and Eliminar)
		if (!pack.isInternal) {
			// [ Importar ] button
			int impBtnX = leftX + leftW - 74;
			int impBtnY = renderY - 2;
			int impBtnW = 60;
			int impBtnH = 14;

			bool isHoverImp = (xm >= impBtnX && xm <= impBtnX + impBtnW && ym >= impBtnY && ym <= impBtnY + impBtnH);
			fill(impBtnX, impBtnY, impBtnX + impBtnW, impBtnY + impBtnH, isHoverImp ? 0x90606060 : 0x70404040);
			fill(impBtnX, impBtnY, impBtnX + impBtnW, impBtnY + 1, 0x60ffffff);
			fill(impBtnX, impBtnY + impBtnH - 1, impBtnX + impBtnW, impBtnY + impBtnH, 0x60ffffff);
			fill(impBtnX, impBtnY, impBtnX + 1, impBtnY + impBtnH, 0x60ffffff);
			fill(impBtnX + impBtnW - 1, impBtnY, impBtnX + impBtnW, impBtnY + impBtnH, 0x60ffffff);

			drawCenteredString(font, I18n::get("gui.import"), impBtnX + impBtnW / 2, impBtnY + 3, isHoverImp ? 0xffff00 : 0xffffff);

			// [ Eliminar ] button
			int delBtnX = leftX + leftW - 138;
			int delBtnY = renderY - 2;
			int delBtnW = 60;
			int delBtnH = 14;

			bool isHoverDel = (xm >= delBtnX && xm <= delBtnX + delBtnW && ym >= delBtnY && ym <= delBtnY + delBtnH);
			fill(delBtnX, delBtnY, delBtnX + delBtnW, delBtnY + delBtnH, isHoverDel ? 0x90a03030 : 0x70702020);
			fill(delBtnX, delBtnY, delBtnX + delBtnW, delBtnY + 1, 0x60ffffff);
			fill(delBtnX, delBtnY + delBtnH - 1, delBtnX + delBtnW, delBtnY + delBtnH, 0x60ffffff);
			fill(delBtnX, delBtnY, delBtnX + 1, delBtnY + delBtnH, 0x60ffffff);
			fill(delBtnX + delBtnW - 1, delBtnY, delBtnX + delBtnW, delBtnY + delBtnH, 0x60ffffff);

			drawCenteredString(font, I18n::get("gui.delete"), delBtnX + delBtnW / 2, delBtnY + 3, isHoverDel ? 0xffff00 : 0xffffff);
		}

		// Draw Pagination controls if maxPage > 0
		if (maxPage > 0) {
			int rightOffset = !pack.isInternal ? 144 : 16;
			int nextBtnX = leftX + leftW - rightOffset - 16;
			int prevBtnX = nextBtnX - 44;
			int pageBtnY = renderY - 2;
			int pageBtnW = 14;
			int pageBtnH = 14;

			// Previous Page Button [<]
			bool isPrevHover = (xm >= prevBtnX && xm <= prevBtnX + pageBtnW && ym >= pageBtnY && ym <= pageBtnY + pageBtnH);
			fill(prevBtnX, pageBtnY, prevBtnX + pageBtnW, pageBtnY + pageBtnH, isPrevHover ? 0x90606060 : 0x70404040);
			fill(prevBtnX, pageBtnY, prevBtnX + pageBtnW, pageBtnY + 1, 0x60ffffff);
			fill(prevBtnX, pageBtnY + pageBtnH - 1, prevBtnX + pageBtnW, pageBtnY + pageBtnH, 0x60ffffff);
			fill(prevBtnX, pageBtnY, prevBtnX + 1, pageBtnY + pageBtnH, 0x60ffffff);
			fill(prevBtnX + pageBtnW - 1, pageBtnY, prevBtnX + pageBtnW, pageBtnY + pageBtnH, 0x60ffffff);
			drawCenteredString(font, "<", prevBtnX + pageBtnW / 2, pageBtnY + 3, isPrevHover ? 0xffff00 : 0xffffff);

			// Page Indicator string e.g. "1/3"
			std::string pageStr = std::to_string(pack.pageOffset + 1) + "/" + std::to_string(maxPage + 1);
			drawCenteredString(font, pageStr, prevBtnX + pageBtnW + 15, renderY, 0xaaaaaa);

			// Next Page Button [>]
			bool isNextHover = (xm >= nextBtnX && xm <= nextBtnX + pageBtnW && ym >= pageBtnY && ym <= pageBtnY + pageBtnH);
			fill(nextBtnX, pageBtnY, nextBtnX + pageBtnW, pageBtnY + pageBtnH, isNextHover ? 0x90606060 : 0x70404040);
			fill(nextBtnX, pageBtnY, nextBtnX + pageBtnW, pageBtnY + 1, 0x60ffffff);
			fill(nextBtnX, pageBtnY + pageBtnH - 1, nextBtnX + pageBtnW, pageBtnY + pageBtnH, 0x60ffffff);
			fill(nextBtnX, pageBtnY, nextBtnX + 1, pageBtnY + pageBtnH, 0x60ffffff);
			fill(nextBtnX + pageBtnW - 1, pageBtnY, nextBtnX + pageBtnW, pageBtnY + pageBtnH, 0x60ffffff);
			drawCenteredString(font, ">", nextBtnX + pageBtnW / 2, pageBtnY + 3, isNextHover ? 0xffff00 : 0xffffff);
		}

		// Container box for pack skins
		int boxY = renderY + 12;
		int boxH = showFullBodyCards ? 58 : 50;
		fill(leftX + 6, boxY, leftX + leftW - 14, boxY + boxH, 0xf0303235);

		if (pack.skins.empty()) {
			drawString(font, I18n::get("skindex.emptyPack"), leftX + 12, boxY + boxH / 2 - 4, 0x888888);
		} else {
			int startSkin = pack.pageOffset * cardsPerPage;
			int endSkin = std::min(totalSkins, startSkin + cardsPerPage);

			for (int s = startSkin; s < endSkin; ++s) {
				int cardIndex = s - startSkin;
				int cardX = leftX + 10 + cardIndex * (cardW + 5);
				int cardY = boxY + 2;

				bool isSelected = (currentPackIndex == p && currentSkinIndex == s);
				
				// Use localized name if available, otherwise fallback to filename
				std::string skinName;
				if (s < (int)pack.skinDisplayNames.size() && !pack.skinDisplayNames[s].empty()) {
					skinName = pack.skinDisplayNames[s];
				} else {
					skinName = pack.skins[s];
					size_t slashPos = skinName.find_last_of("\\/");
					if (slashPos != std::string::npos) skinName = skinName.substr(slashPos + 1);
					if (skinName.length() > 4 && skinName.substr(skinName.length() - 4) == ".png") {
						skinName = skinName.substr(0, skinName.length() - 4);
					}
				}

				// Determine if this skin should use slim model based on geometry
				bool skinIsSlim = false;
				if (s < (int)pack.skinGeometries.size() && !pack.skinGeometries[s].empty()) {
					std::string geometry = pack.skinGeometries[s];
					std::string lowerGeom = geometry;
					std::transform(lowerGeom.begin(), lowerGeom.end(), lowerGeom.begin(), ::tolower);
					skinIsSlim = (lowerGeom.find("slim") != std::string::npos || lowerGeom.find("alex") != std::string::npos);
				} else {
					// Fallback to filename-based detection
					std::string skinPath = pack.skins[s];
					std::string lowerPath = skinPath;
					std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
					skinIsSlim = (lowerPath.find("cesar") != std::string::npos || lowerPath.find("alex") != std::string::npos || lowerPath.find("slim") != std::string::npos);
				}

				drawSkinCard(cardX, cardY, cardW, cardH, pack.skins[s], isSelected, skinName, skinIsSlim);
			}
		}
	}

	glDisable2(GL_SCISSOR_TEST);

	// --- RIGHT PANEL (PLAYER 3D PREVIEW & ACTIONS) ---
	fill(rightX, topY, rightX + rightW, topY + rightAvailableH, 0xf054565a);
	fill(rightX, topY, rightX + rightW, topY + 1, 0x60ffffff);
	fill(rightX + rightW - 1, topY, rightX + rightW, topY + rightAvailableH, 0x60ffffff);

	SkinPack& activePack = skinPacks[currentPackIndex];
	std::string currentSkin = activePack.skins.empty() ? "mob/char.png" : activePack.skins[currentSkinIndex];

	// Header inside Right Panel - use localized name if available
	std::string currentSkinName;
	if (currentSkinIndex < (int)activePack.skinDisplayNames.size() && !activePack.skinDisplayNames[currentSkinIndex].empty()) {
		currentSkinName = activePack.skinDisplayNames[currentSkinIndex];
	} else {
		currentSkinName = currentSkin;
		size_t sPos = currentSkinName.find_last_of("\\/");
		if (sPos != std::string::npos) currentSkinName = currentSkinName.substr(sPos + 1);
		if (currentSkinName.length() > 4 && currentSkinName.substr(currentSkinName.length() - 4) == ".png") {
			currentSkinName = currentSkinName.substr(0, currentSkinName.length() - 4);
		}
	}
	drawCenteredString(font, currentSkinName, rightX + rightW / 2, topY + 6, 0xffff00);

	// Controls validation
	bool isBuiltin = false;
	if (!activePack.skins.empty()) {
		std::string fname = activePack.skins[currentSkinIndex];
		size_t pos = fname.find_last_of("\\/");
		if (pos != std::string::npos) fname = fname.substr(pos + 1);
		if (fname == "steve.png" || fname == "cesar.png" || fname == "cesarWhite.png" || fname == "char.png") {
			isBuiltin = true;
		}
	} else {
		isBuiltin = true;
	}
	btnRename.active = !isBuiltin && !activePack.isInternal;
	btnDelete.active = !isBuiltin && !activePack.isInternal;
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
		HumanoidModel* model = isSlimModel ? modelSlim : modelNormal;
		if (model) {
			model->render(nullptr, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f);
		}

		glPopMatrix();
		glDisable2(GL_DEPTH_TEST);
		GuiShader::bind();

		// Rotation Indicator Slider Graphic
		int rotIndicatorY = topY + rightAvailableH - 36;
		drawCenteredString(font, "<------ ( o ) ------>", rightX + rightW / 2, rotIndicatorY, 0xaaaaaa);
	}

	// Render overlay confirmation delete modal if open
	if (showDeleteModal) {
		fill(0, 0, width, height, 0xc0000000);

		int modalW = 220;
		int modalH = 80;
		int modalX = (width - modalW) / 2;
		int modalY = (height - modalH) / 2;

		fill(modalX, modalY, modalX + modalW, modalY + modalH, 0xf0282828);
		fill(modalX, modalY, modalX + modalW, modalY + 1, 0xff707070);
		fill(modalX, modalY + modalH - 1, modalX + modalW, modalY + modalH, 0xff707070);
		fill(modalX, modalY, modalX + 1, modalY + modalH, 0xff707070);
		fill(modalX + modalW - 1, modalY, modalX + modalW, modalY + modalH, 0xff707070);

		std::string titleStr = (pendingDeleteType == DELETE_PACK) 
			? I18n::get("skindex.confirmDeletePackTitle")
			: I18n::get("skindex.confirmDeleteSkinTitle");
		drawCenteredString(font, titleStr, width / 2, modalY + 10, 0xffff00);

		std::string msgStr = (pendingDeleteType == DELETE_PACK)
			? I18n::get("skindex.confirmDeletePackMsg")
			: I18n::get("skindex.confirmDeleteSkinMsg");
		drawCenteredString(font, msgStr, width / 2, modalY + 28, 0xe0e0e0);

		int btnW = 75;
		int btnH = 20;
		int btnY = modalY + modalH - 26;

		int btnYesX = modalX + 20;
		int btnNoX = modalX + modalW - 20 - btnW;

		bool hoverYes = (xm >= btnYesX && xm <= btnYesX + btnW && ym >= btnY && ym <= btnY + btnH);
		bool hoverNo = (xm >= btnNoX && xm <= btnNoX + btnW && ym >= btnY && ym <= btnY + btnH);

		fill(btnYesX, btnY, btnYesX + btnW, btnY + btnH, hoverYes ? 0x90a03030 : 0x70702020);
		fill(btnYesX, btnY, btnYesX + btnW, btnY + 1, 0x60ffffff);
		fill(btnYesX, btnY + btnH - 1, btnYesX + btnW, btnY + btnH, 0x60ffffff);
		fill(btnYesX, btnY, btnYesX + 1, btnY + btnH, 0x60ffffff);
		fill(btnYesX + btnW - 1, btnY, btnYesX + btnW, btnY + btnH, 0x60ffffff);
		drawCenteredString(font, I18n::get("gui.delete"), btnYesX + btnW / 2, btnY + 6, hoverYes ? 0xffff00 : 0xffffff);

		fill(btnNoX, btnY, btnNoX + btnW, btnY + btnH, hoverNo ? 0x90606060 : 0x70404040);
		fill(btnNoX, btnY, btnNoX + btnW, btnY + 1, 0x60ffffff);
		fill(btnNoX, btnY + btnH - 1, btnNoX + btnW, btnY + btnH, 0x60ffffff);
		fill(btnNoX, btnY, btnNoX + 1, btnNoX + btnH, 0x60ffffff);
		fill(btnNoX + btnW - 1, btnY, btnNoX + btnW, btnY + btnH, 0x60ffffff);
		drawCenteredString(font, I18n::get("gui.cancel"), btnNoX + btnW / 2, btnY + 6, hoverNo ? 0xffff00 : 0xffffff);
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

		if (!skinPacks.empty() && currentPackIndex >= 0 && currentPackIndex < (int)skinPacks.size()) {
			SkinPack& activePack = skinPacks[currentPackIndex];
			if (!activePack.skins.empty() && currentSkinIndex >= 0 && currentSkinIndex < (int)activePack.skins.size()) {
				std::string newGeometry = isSlimModel ? "geometry.humanoid.customSlim" : "geometry.humanoid.custom";
				if (currentSkinIndex < (int)activePack.skinGeometries.size()) {
					activePack.skinGeometries[currentSkinIndex] = newGeometry;
				}

				if (!activePack.isInternal) {
					std::string currentSkinPath = activePack.skins[currentSkinIndex];
					std::string textureFilename = currentSkinPath;
					size_t slashPos = textureFilename.find_last_of("\\/");
					if (slashPos != std::string::npos) {
						textureFilename = textureFilename.substr(slashPos + 1);
					}
					std::string packDir = "games/com.mojang/skins/" + activePack.name;
					updateGeometryInSkinsJson(packDir, textureFilename, isSlimModel);
				}
			}
		}
	} else if (button->id == btnCardViewMode.id) {
		showFullBodyCards = !showFullBodyCards;
		setupPositions();
	} else if (button->id == btnAutoRotate.id) {
		autoRotate = !autoRotate;
		setupPositions();
	} else if (button->id == btnRename.id) {
		SkinPack& activePack = skinPacks[currentPackIndex];
		if (!activePack.skins.empty()) {
			minecraft->setScreen(new RenameSkinScreen(activePack.skins[currentSkinIndex], currentPackIndex, currentSkinIndex));
		}
	} else if (button->id == btnNewPack.id) {
		minecraft->setScreen(new NewPackScreen());
	} else if (button->id == btnDelete.id) {
		SkinPack& activePack = skinPacks[currentPackIndex];
		if (!activePack.skins.empty() && currentSkinIndex >= 0 && currentSkinIndex < (int)activePack.skins.size()) {
			showDeleteModal = true;
			pendingDeleteType = DELETE_SKIN;
			pendingDeleteIndex = currentSkinIndex;
		}
	}
}

bool SkindexScreen::handleBackEvent(bool isDown) {
	if (showDeleteModal) {
		if (!isDown) {
			showDeleteModal = false;
			pendingDeleteType = DELETE_NONE;
			pendingDeleteIndex = -1;
		}
		return true;
	}
	if (isDown) return true;
	minecraft->setScreen(nullptr);
	return true;
}
