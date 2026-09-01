#ifndef NET_MINECRAFT_CLIENT_RENDERER__BlockAtlasStitcher_H__
#define NET_MINECRAFT_CLIENT_RENDERER__BlockAtlasStitcher_H__

#include <string>
#include <map>
#include <vector>
#include "TextureData.h"

class AppPlatform;

class BlockAtlasStitcher {
public:
    struct TextureMapping {
        std::string filename;
        int index; // 0..255
        std::string atlasName; // "terrain.png" or "terrain2.png"
    };

    // Stitches all separate block PNGs into the provided atlas in memory
    static void stitchAtlas(const std::string& atlasResourceName, TextureData& atlasData, AppPlatform* platform);

    // Inserts a tile into the atlas texture at the specified cell index (0..255)
    static bool insertTile(TextureData& dstAtlas, int slotIndex, const TextureData& srcTile);

    // Gets the registered mapping table
    static const std::vector<TextureMapping>& getTextureMappings();
};

#endif /* NET_MINECRAFT_CLIENT_RENDERER__BlockAtlasStitcher_H__ */
