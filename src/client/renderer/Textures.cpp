#include "Textures.h"

#include "TextureData.h"
#include "BlockAtlasStitcher.h"
#include "ptexture/DynamicTexture.h"
#include "../Options.h"
#include "../../AppPlatform.h"
#include "../../util/StringUtils.h"

/*static*/ int  Textures::textureChanges = 0;
/*static*/ bool Textures::MIPMAP = false;
/*static*/ int  Textures::MIPMAP_LEVELS = 0;
/*static*/ const TextureId Textures::InvalidId = -1;

static bool _isSkyBodyTexture(const std::string& resourceName)
{
	return resourceName == "environment/sun.png" || resourceName == "environment/moon_phases.png";
}

static bool _hasAnyTransparentAlpha(const TextureData& texdata)
{
	if (!texdata.data || texdata.w <= 0 || texdata.h <= 0)
		return false;

	if (texdata.format != TEXF_UNCOMPRESSED_8888)
		return false;

	const int pixelCount = texdata.w * texdata.h;
	const unsigned char* p = texdata.data;
	for (int i = 0; i < pixelCount; ++i, p += 4) {
		if (p[3] < 250)
			return true;
	}

	return false;
}

static void _applySkyBodyAlpha(const std::string& resourceName, TextureData& texdata)
{
	if (!texdata.data || texdata.w <= 0 || texdata.h <= 0)
		return;

	if (texdata.format != TEXF_UNCOMPRESSED_8888)
		return;

	// Keep authored alpha if the texture already contains transparency.
	if (_hasAnyTransparentAlpha(texdata)) {
		texdata.transparent = true;
		return;
	}

	const int pixelCount = texdata.w * texdata.h;
	unsigned char* p = texdata.data;

	if (resourceName == "environment/sun.png") {
		for (int i = 0; i < pixelCount; ++i, p += 4) {
			int m = p[0];
			if (p[1] > m) m = p[1];
			if (p[2] > m) m = p[2];

			// Build usable alpha from brightness for legacy opaque sun art.
			if (m <= 24) {
				p[3] = 0;
			}
			else if (m < 96) {
				p[3] = (unsigned char)((m - 24) * 255 / (96 - 24));
			}
			else {
				p[3] = 255;
			}
		}
	}
	else {
		for (int i = 0; i < pixelCount; ++i, p += 4) {
			int m = p[0];
			if (p[1] > m) m = p[1];
			if (p[2] > m) m = p[2];

			// Moon atlas in this pack uses dark opaque background; rebuild alpha from brightness.
			if (m <= 18) {
				p[3] = 0;
			}
			else if (m < 72) {
				p[3] = (unsigned char)((m - 18) * 255 / (72 - 18));
			}
			else {
				p[3] = 255;
			}
		}
	}

	texdata.transparent = true;
}

Textures::Textures( Options* options_, AppPlatform* platform_ )
:	clamp(false),
	blur(false),
	options(options_),
	platform(platform_),
	lastBoundTexture(Textures::InvalidId)
{
}

Textures::~Textures()
{
	clear();

	for (unsigned int i = 0; i < dynamicTextures.size(); ++i)
		delete dynamicTextures[i];
}

std::map<std::string, std::vector<TextureData>> Textures::s_atlasLevelsMap;

static unsigned char s_animL1[8 * 8 * 4];
static unsigned char s_animL2[4 * 4 * 4];
static unsigned char s_animL3[2 * 2 * 4];
static unsigned char s_animL4[1 * 1 * 4];

void Textures::clear()
{
	for (TextureMap::iterator it = idMap.begin(); it != idMap.end(); ++it) {
		if (it->second != Textures::InvalidId)
			glDeleteTextures(1, &it->second);
	}
	for (TextureImageMap::iterator it = loadedImages.begin(); it != loadedImages.end(); ++it) {
		if (!(it->second).memoryHandledExternally)
			delete[] (it->second).data;
	}
	for (auto& pair : s_atlasLevelsMap) {
		for (size_t i = 1; i < pair.second.size(); i++) {
			if (pair.second[i].data && !pair.second[i].memoryHandledExternally) {
				delete[] pair.second[i].data;
				pair.second[i].data = nullptr;
			}
		}
	}
	s_atlasLevelsMap.clear();
	idMap.clear();
	loadedImages.clear();

	lastBoundTexture = Textures::InvalidId;
}

TextureId Textures::loadAndBindTexture( const std::string& resourceName )
{
	//static Stopwatch t;

	TextureId id = loadTexture(resourceName);
	if (id != Textures::InvalidId)
		bind(id);

	return id;
}

TextureId Textures::loadTexture( const std::string& resourceName, bool inTextureFolder /* = true */ )
{
	TextureMap::iterator it = idMap.find(resourceName);
	if (it != idMap.end()) {
		return it->second;
	}

	bool isUrl = Util::startsWith(resourceName, "http://") || Util::startsWith(resourceName, "https://");
	
	// Detectar si es ruta personalizada (empieza con "blocks/" o "data/")
	bool isCustomPath = resourceName.find("blocks/") == 0 || resourceName.find("data/") == 0;
	bool useTextureFolder = inTextureFolder && !isCustomPath;

	TextureData texdata = platform->loadTexture(resourceName, isUrl ? false : useTextureFolder);
	if (resourceName.find("terrain.png") != std::string::npos || resourceName.find("terrain2.png") != std::string::npos) {
		std::vector<TextureData> atlasLevels = BlockAtlasStitcher::stitchAtlasMultiLevel(resourceName, texdata, platform, Textures::MIPMAP_LEVELS);
		s_atlasLevelsMap[resourceName] = atlasLevels;
	}
	if (_isSkyBodyTexture(resourceName)) {
		_applySkyBodyAlpha(resourceName, texdata);
	}
	if (texdata.data) {
		TextureId newId = assignTexture(resourceName, texdata);
		return newId;
	}
	else if (texdata.identifier != InvalidId) {
		idMap.insert(std::make_pair(resourceName, texdata.identifier));
		return texdata.identifier;
	}
	else {
		idMap.insert(std::make_pair(resourceName, Textures::InvalidId));
	}
	return Textures::InvalidId;
}

TextureId Textures::assignTexture( const std::string& resourceName, const TextureData& img )
{
	TextureId id;
	glGenTextures(1, &id);

	bind(id);

	bool isTerrainAtlas = (resourceName.find("terrain.png") != std::string::npos || resourceName.find("terrain2.png") != std::string::npos);

	if (isTerrainAtlas && s_atlasLevelsMap.find(resourceName) != s_atlasLevelsMap.end()) {
		const auto& levels = s_atlasLevelsMap[resourceName];
		int numLevels = std::min((int)levels.size() - 1, Textures::MIPMAP_LEVELS);
		if (numLevels < 0) numLevels = 0;

		if (numLevels > 0) {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numLevels);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			for (int lvl = 0; lvl <= numLevels; lvl++) {
				glTexImage2D2(GL_TEXTURE_2D, lvl, GL_RGBA, levels[lvl].w, levels[lvl].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, levels[lvl].data);
			}
		} else {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D2(GL_TEXTURE_2D, 0, GL_RGBA, levels[0].w, levels[0].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, levels[0].data);
		}

		if (clamp) {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		} else {
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		idMap.insert(std::make_pair(resourceName, id));
		loadedImages.insert(std::make_pair(id, img));
		return id;
	}

	int levels = isTerrainAtlas ? Textures::MIPMAP_LEVELS : 0;
	if (levels > 4) levels = 4;
	if (levels < 0) levels = 0;

	if (levels > 0) {
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (blur) {
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	if (clamp) {
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else {
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

    switch (img.format)
    {
        case TEXF_COMPRESSED_PVRTC_4444:
        case TEXF_COMPRESSED_PVRTC_565:
        case TEXF_COMPRESSED_PVRTC_5551:
        {
#if defined(__APPLE__)
            int fmt = img.transparent? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, fmt, img.w, img.h, 0, img.numBytes, img.data);
#endif
            break;
        }

        default:
            const GLint mode = img.transparent? GL_RGBA : GL_RGB;

            if (img.format == TEXF_UNCOMPRESSED_565) {
                glTexImage2D2(GL_TEXTURE_2D, 0, mode, img.w, img.h, 0, mode, GL_UNSIGNED_SHORT_5_6_5, img.data);
            }
            else if (img.format == TEXF_UNCOMPRESSED_4444) {
                glTexImage2D2(GL_TEXTURE_2D, 0, mode, img.w, img.h, 0, mode, GL_UNSIGNED_SHORT_4_4_4_4, img.data);
            }
            else if (img.format == TEXF_UNCOMPRESSED_5551) {
                glTexImage2D2(GL_TEXTURE_2D, 0, mode, img.w, img.h, 0, mode, GL_UNSIGNED_SHORT_5_5_5_1, img.data);
            }
            else {
                glTexImage2D2(GL_TEXTURE_2D, 0, mode, img.w, img.h, 0, mode, GL_UNSIGNED_BYTE, img.data);
            }
            break;
    }

	idMap.insert(std::make_pair(resourceName, id));
	loadedImages.insert(std::make_pair(id, img));

	return id;
}

const TextureData* Textures::getTemporaryTextureData( TextureId id )
{
	TextureImageMap::iterator it = loadedImages.find(id);
	if (it == loadedImages.end())
		return NULL;

	return &it->second;
}

int* Textures::loadTexturePixels(TextureId texId, const std::string& resourceName) {
	const TextureData* texture = getTemporaryTextureData(texId);
	if (!texture || !texture->data)
		return NULL;

	int size = texture->w * texture->h;
	int* pixels = new int[size];
	unsigned char* raw = texture->data;
	for (int i = 0; i < size; i++) {
		int r = raw[i * 4 + 0];
		int g = raw[i * 4 + 1];
		int b = raw[i * 4 + 2];
		int a = raw[i * 4 + 3];
		pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
	}
	return pixels;
}

void Textures::tick(bool uploadToGraphicsCard)
{
	for (unsigned int i = 0; i < dynamicTextures.size(); ++i ) {
		DynamicTexture* tex = dynamicTextures[i];
		tex->tick();

		if (uploadToGraphicsCard) {
			tex->bindTexture(this);
			for (int xx = 0; xx < tex->replicate; xx++) {
				for (int yy = 0; yy < tex->replicate; yy++) {
					int tileX = (tex->tex % 16) + xx;
					int tileY = (tex->tex / 16) + yy;

					glTexSubImage2D2(GL_TEXTURE_2D, 0, tileX * 16, tileY * 16, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, tex->pixels);

					if (MIPMAP_LEVELS > 0) {
						TextureCategory cat = (dynamic_cast<WaterTexture*>(tex) || dynamic_cast<WaterSideTexture*>(tex)) ? CAT_TRANSLUCENT : CAT_OPAQUE;
						unsigned char* bufList[5] = { tex->pixels, s_animL1, s_animL2, s_animL3, s_animL4 };

						for (int lvl = 1; lvl <= MIPMAP_LEVELS && lvl <= 4; lvl++) {
							int prevS = 16 >> (lvl - 1);
							int currS = 16 >> lvl;
							BlockAtlasStitcher::downsampleTile(bufList[lvl - 1], prevS, prevS, bufList[lvl], cat);

							int subX = tileX * currS;
							int subY = tileY * currS;
							glTexSubImage2D2(GL_TEXTURE_2D, lvl, subX, subY, currS, currS, GL_RGBA, GL_UNSIGNED_BYTE, bufList[lvl]);
						}
					}
				}
			}
		}
	}
}

void Textures::addDynamicTexture( DynamicTexture* dynamicTexture )
{
	dynamicTextures.push_back(dynamicTexture);
	dynamicTexture->tick();
}

void Textures::reloadAll()
{
	clear();
	loadAndBindTexture("terrain.png");
	loadAndBindTexture("terrain2.png");
}

int Textures::smoothBlend( int c0, int c1 )
{
	int a0 = (int) (((c0 & 0xff000000) >> 24)) & 0xff;
	int a1 = (int) (((c1 & 0xff000000) >> 24)) & 0xff;
	return ((a0 + a1) >> 1 << 24) + (((c0 & 0x00fefefe) + (c1 & 0x00fefefe)) >> 1);
}

int Textures::crispBlend( int c0, int c1 )
{
	int a0 = (int) (((c0 & 0xff000000) >> 24)) & 0xff;
	int a1 = (int) (((c1 & 0xff000000) >> 24)) & 0xff;

	int a = 255;
	if (a0 + a1 == 0) {
		a0 = 1;
		a1 = 1;
		a = 0;
	}

	int r0 = ((c0 >> 16) & 0xff) * a0;
	int g0 = ((c0 >> 8) & 0xff) * a0;
	int b0 = ((c0) & 0xff) * a0;

	int r1 = ((c1 >> 16) & 0xff) * a1;
	int g1 = ((c1 >> 8) & 0xff) * a1;
	int b1 = ((c1) & 0xff) * a1;

	int r = (r0 + r1) / (a0 + a1);
	int g = (g0 + g1) / (a0 + a1);
	int b = (b0 + b1) / (a0 + a1);

	return (a << 24) | (r << 16) | (g << 8) | b;
}

///*public*/ int loadHttpTexture(std::string url, std::string backup) {
//    HttpTexture texture = httpTextures.get(url);
//    if (texture != NULL) {
//        if (texture.loadedImage != NULL && !texture.isLoaded) {
//            if (texture.id < 0) {
//                texture.id = getTexture(texture.loadedImage);
//            } else {
//                loadTexture(texture.loadedImage, texture.id);
//            }
//            texture.isLoaded = true;
//        }
//    }
//    if (texture == NULL || texture.id < 0) {
//        if (backup == NULL) return -1;
//        return loadTexture(backup);
//    }
//    return texture.id;
//}

//HttpTexture addHttpTexture(std::string url, HttpTextureProcessor processor) {
//    HttpTexture texture = httpTextures.get(url);
//    if (texture == NULL) {
//        httpTextures.put(url, /*new*/ HttpTexture(url, processor));
//    } else {
//        texture.count++;
//    }
//    return texture;
//}

//void removeHttpTexture(std::string url) {
//    HttpTexture texture = httpTextures.get(url);
//    if (texture != NULL) {
//        texture.count--;
//        if (texture.count == 0) {
//            if (texture.id >= 0) releaseTexture(texture.id);
//            httpTextures.remove(url);
//        }
//    }
//}

//void tick() {
//	for (int i = 0; i < dynamicTextures.size(); i++) {
//		DynamicTexture dynamicTexture = dynamicTextures.get(i);
//		dynamicTexture.anaglyph3d = options.anaglyph3d;
//		dynamicTexture.tick();
//
//		pixels.clear();
//		pixels.put(dynamicTexture.pixels);
//		pixels.position(0).limit(dynamicTexture.pixels.length);
//
//		dynamicTexture.bindTexture(this);
//
//		for (int xx = 0; xx < dynamicTexture.replicate; xx++)
//			for (int yy = 0; yy < dynamicTexture.replicate; yy++) {
//
//				glTexSubImage2D2(GL_TEXTURE_2D, 0, dynamicTexture.tex % 16 * 16 + xx * 16, dynamicTexture.tex / 16 * 16 + yy * 16, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
//				if (MIPMAP) {
//					for (int level = 1; level <= 4; level++) {
//						int os = 16 >> (level - 1);
//						int s = 16 >> level;
//
//						for (int x = 0; x < s; x++)
//							for (int y = 0; y < s; y++) {
//								int c0 = pixels.getInt(((x * 2 + 0) + (y * 2 + 0) * os) * 4);
//								int c1 = pixels.getInt(((x * 2 + 1) + (y * 2 + 0) * os) * 4);
//								int c2 = pixels.getInt(((x * 2 + 1) + (y * 2 + 1) * os) * 4);
//								int c3 = pixels.getInt(((x * 2 + 0) + (y * 2 + 1) * os) * 4);
//								int col = smoothBlend(smoothBlend(c0, c1), smoothBlend(c2, c3));
//								pixels.putInt((x + y * s) * 4, col);
//							}
//							glTexSubImage2D2(GL_TEXTURE_2D, level, dynamicTexture.tex % 16 * s, dynamicTexture.tex / 16 * s, s, s, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
//					}
//				}
//			}
//	}
//
//	for (int i = 0; i < dynamicTextures.size(); i++) {
//		DynamicTexture dynamicTexture = dynamicTextures.get(i);
//
//		if (dynamicTexture.copyTo > 0) {
//			pixels.clear();
//			pixels.put(dynamicTexture.pixels);
//			pixels.position(0).limit(dynamicTexture.pixels.length);
//			glBindTexture2(GL_TEXTURE_2D, dynamicTexture.copyTo);
//			glTexSubImage2D2(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
//			if (MIPMAP) {
//				for (int level = 1; level <= 4; level++) {
//					int os = 16 >> (level - 1);
//					int s = 16 >> level;
//
//					for (int x = 0; x < s; x++)
//						for (int y = 0; y < s; y++) {
//							int c0 = pixels.getInt(((x * 2 + 0) + (y * 2 + 0) * os) * 4);
//							int c1 = pixels.getInt(((x * 2 + 1) + (y * 2 + 0) * os) * 4);
//							int c2 = pixels.getInt(((x * 2 + 1) + (y * 2 + 1) * os) * 4);
//							int c3 = pixels.getInt(((x * 2 + 0) + (y * 2 + 1) * os) * 4);
//							int col = smoothBlend(smoothBlend(c0, c1), smoothBlend(c2, c3));
//							pixels.putInt((x + y * s) * 4, col);
//						}
//						glTexSubImage2D2(GL_TEXTURE_2D, level, 0, 0, s, s, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
//				}
//			}
//		}
//	}
//}
//  void releaseTexture(int id) {
//      loadedImages.erase(id);
//      glDeleteTextures(1, (const GLuint*)&id);
//  }

