#ifndef NET_MINECRAFT_CLIENT_RENDERER__BlockAtlasStitcher_H__
#define NET_MINECRAFT_CLIENT_RENDERER__BlockAtlasStitcher_H__

#include <string>
#include <map>
#include <vector>
#include "TextureData.h"

class AppPlatform;

enum TextureCategory {
    CAT_OPAQUE,
    CAT_CUTOUT,
    CAT_TRANSLUCENT
};

class BlockAtlasStitcher {
public:
    struct TextureMapping {
        std::string filename;
        int index; // 0..255
        std::string atlasName; // "terrain.png" or "terrain2.png"
    };

    static TextureCategory getTextureCategory(const std::string& filename);
    static void downsampleTile(const unsigned char* src, int srcW, int srcH, unsigned char* dst, TextureCategory cat);

    // Stitches all separate block PNGs into the provided atlas in memory
    static void stitchAtlas(const std::string& atlasResourceName, TextureData& atlasData, AppPlatform* platform);

    // Stitches all separate block PNGs into multi-level atlas mipmaps (L0..L4)
    static std::vector<TextureData> stitchAtlasMultiLevel(const std::string& atlasResourceName, TextureData& atlasL0, AppPlatform* platform, int maxLevels = 4);

    // Inserts a tile into the atlas texture at the specified cell index (0..255)
    static bool insertTile(TextureData& dstAtlas, int slotIndex, const TextureData& srcTile);

    // Gets the registered mapping table
    static const std::vector<TextureMapping>& getTextureMappings();
};

#endif /* NET_MINECRAFT_CLIENT_RENDERER__BlockAtlasStitcher_H__ */
