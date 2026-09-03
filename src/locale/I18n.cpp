#include "I18n.h"
#include <sstream>
#include "../AppPlatform.h"
#include "../util/StringUtils.h"
#include "../world/level/tile/Tile.h"
#include "../world/item/Item.h"
#include "../world/item/ItemInstance.h"
#include <ctype.h>

I18n::Map I18n::_strings;

void I18n::loadLanguage( AppPlatform* platform, const std::string& languageCode )
{
	_strings.clear();
	fillTranslations(platform, "lang/en_US.lang", true);

	if (languageCode != "en_US")
		fillTranslations(platform, "lang/" + languageCode + ".lang", true);
}

bool I18n::get( const std::string& id, std::string& out ) {
	Map::const_iterator cit = _strings.find(id);
	if (cit != _strings.end()) {
		out = cit->second;
		return true;
	}
	return false;
}

std::string I18n::get( const std::string& id )
{
	Map::const_iterator cit = _strings.find(id);
	if (cit != _strings.end())
		return cit->second;

	// Fallback robusto para nombres (soporta minúsculas del sistema de descripción)
	std::string lowId = id;
	for(auto &c : lowId) c = tolower(c);

	if (lowId == "tile.planksspruce.name") return "Spruce Wood Planks";
	if (lowId == "tile.planksbirch.name") return "Birch Wood Planks";
	if (lowId == "tile.spruceslab.name") return "Spruce Wood Slab";
	if (lowId == "tile.birchslab.name") return "Birch Wood Slab";
	if (lowId == "tile.stairsspruce.name") return "Spruce Wood Stairs";
	if (lowId == "tile.stairsbirch.name") return "Birch Wood Stairs";
	if (lowId == "tile.fencespruce.name") return "Spruce Fence";
	if (lowId == "tile.fencebirch.name") return "Birch Fence";
	if (lowId == "tile.logspruce.name") return "Spruce Log";
	if (lowId == "tile.logbirch.name") return "Birch Log";
	if (lowId == "tile.chest.name") return "Chest";

	return id + '<';//lang.getElement(id);
}

void I18n::fillTranslations( AppPlatform* platform, const std::string& filename, bool overwrite )
{
	BinaryBlob blob = platform->readAssetFile(filename);
	if (!blob.data || blob.size <= 0)
		return;

	std::string data((const char*)blob.data, blob.size);
	
	// The font texture (default8.png) uses CP437 encoding for the extended range.
	// We must convert UTF-8 input to the correct CP437 byte for each character.
	std::string cp437_data;
	cp437_data.reserve(data.length());
	for (size_t i = 0; i < data.length(); ) {
		unsigned char c = data[i];
		// Skip UTF-8 BOM (EF BB BF)
		if (i == 0 && c == 0xEF && i + 2 < data.length() &&
		    (unsigned char)data[i+1] == 0xBB && (unsigned char)data[i+2] == 0xBF) {
			i += 3;
		// U+00A0..U+00BF: encoded as 0xC2 XX
		} else if (c == 0xC2 && i + 1 < data.length()) {
			unsigned char c2 = data[i+1];
			if      (c2 == 0xBF) cp437_data += (char)0xA8; // ¿
			else if (c2 == 0xA1) cp437_data += (char)0xAD; // ¡
			else                 cp437_data += '?';
			i += 2;
		// U+00C0..U+00FF: encoded as 0xC3 XX
		} else if (c == 0xC3 && i + 1 < data.length()) {
			unsigned char c2 = data[i+1];
			if      (c2 == 0xA1) cp437_data += (char)0xA0; // á  CP437=0xA0
			else if (c2 == 0xA9) cp437_data += (char)0x82; // é  CP437=0x82
			else if (c2 == 0xAD) cp437_data += (char)0xA1; // í  CP437=0xA1
			else if (c2 == 0xB3) cp437_data += (char)0xA2; // ó  CP437=0xA2
			else if (c2 == 0xBA) cp437_data += (char)0xA3; // ú  CP437=0xA3
			else if (c2 == 0xB1) cp437_data += (char)0xA4; // ñ  CP437=0xA4
			else if (c2 == 0x91) cp437_data += (char)0xA5; // Ñ  CP437=0xA5
			else if (c2 == 0x89) cp437_data += (char)0x90; // É  CP437=0x90
			else if (c2 == 0x81) cp437_data += (char)0xB5; // Á  CP437=0xB5
			else if (c2 == 0x8D) cp437_data += (char)0xD6; // Í  CP437=0xD6
			else if (c2 == 0x93) cp437_data += (char)0xE0; // Ó  CP437=0xE0
			else if (c2 == 0x9A) cp437_data += (char)0xE9; // Ú  CP437=0xE9
			else if (c2 == 0xBC) cp437_data += (char)0x81; // ü  CP437=0x81
			else if (c2 == 0x9C) cp437_data += (char)0x9A; // Ü  CP437=0x9A
			else                 cp437_data += '?';
			i += 2;
		// 3-byte and 4-byte UTF-8 (outside Latin-1 range, unlikely in game text)
		} else if ((c & 0xF0) == 0xE0 && i + 2 < data.length()) {
			cp437_data += '?'; i += 3;
		} else if ((c & 0xF8) == 0xF0 && i + 3 < data.length()) {
			cp437_data += '?'; i += 4;
		} else {
			cp437_data += c;
			i++;
		}
	}
	
	std::stringstream fin(cp437_data, std::ios_base::in);

	std::string line;
	while( std::getline(fin, line) ) {
		int spos = line.find('=');
		if (spos == std::string::npos)
			continue;

		std::string key   = Util::stringTrim(line.substr(0, spos));
		Map::const_iterator cit = _strings.find(key);
		if (!overwrite && cit != _strings.end())
			continue;

		std::string value = Util::stringTrim(line.substr(spos + 1));
		if (overwrite)
			_strings[key] = value;                          // always overwrite (e.g. es_ES over en_US)
		else if (_strings.find(key) == _strings.end())
			_strings[key] = value;                          // only insert if key doesn't exist yet
	}

	delete[] blob.data;
}

std::string I18n::getDescriptionString( const ItemInstance& item )
{
	// Convert to lower. Normally std::transform would be used, but tolower might be
	// implemented with a macro in certain C-implementations -> messing stuff up
	const std::string desc = item.getDescriptionId();

	std::string s = desc;
	std::string trans;

	// Handle special cases
	if (item.id == Tile::cloth->id)
		return get(item.getAuxValue()? "desc.wool" : "desc.woolstring");
	else if (item.id == Tile::fenceGate->id || item.id == Tile::fenceGateBirch->id || item.id == Tile::fenceGateSpruce->id || item.id == Tile::fenceGateJungle->id || item.id == Tile::fenceGateAcacia->id || item.id == Tile::fenceGateDarkOak->id)
		return I18n::get("desc.fence");
	else if (item.id == Tile::stoneSlabHalf->id || item.id == Tile::spruceSlabHalf->id || item.id == Tile::birchSlabHalf->id || item.id == Tile::jungleSlabHalf->id || item.id == Tile::acaciaSlabHalf->id || item.id == Tile::darkOakSlabHalf->id)
		return I18n::get("desc.slab");
	else if ((item.id >= 170 && item.id <= 178) || (item.id >= 181 && item.id <= 183))
		return I18n::get("desc.stonebricksmooth");

	for (unsigned int i = 0; i < s.length(); ++i)
		s[i] = ::tolower(s[i]);

	// Replace item./tile. with desc., hopefully it's enough
	if (s[0] == 't') s = Util::stringReplace(s, "tile.", "desc.");
	if (s[0] == 'i') s = Util::stringReplace(s, "item.", "desc.");
	if (I18n::get(s, trans))
		return trans;

	// Remove all materials from the identifier, since swordWood should
	// be read as just sword
	const char* materials[] = {
		"wood",
		"iron",
		"stone",
		"diamond",
		"gold",
		"brick",
		"emerald",
		"lapis",
		"birch",
		"spruce",
		"cloth"
	};

	Util::removeAll(s, materials, sizeof(materials) / sizeof(const char*));
	if (I18n::get(s, trans))
		return trans;

	std::string mapping[] = {
		"tile.workbench",	"craftingtable",
	};
	const char numMappings = sizeof(mapping) / sizeof(std::string);
	for (int i = 0; i < numMappings; i += 2) {
		if (desc == mapping[i]) {
			if (I18n::get("desc." + mapping[i+1], trans))
				return trans;
		}
	}

	return desc + " : couldn't find desc";
}
