#include "BlockAtlasStitcher.h"
#include "../../AppPlatform.h"
#include "../../world/level/tile/Tile.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

static const std::vector<BlockAtlasStitcher::TextureMapping> s_mappings = {
    // Terrain.png - Bloques Simples (UNIFORME)
    { "stone.png", 1, "terrain.png" },
    { "dirt.png", 2, "terrain.png" },
    { "stone_brick.png", 16, "terrain.png" },
    { "planks_oak.png", 4, "terrain.png" },
    { "sand.png", 18, "terrain.png" },
    { "gravel.png", 19, "terrain.png" },
    { "gold_ore.png", 32, "terrain.png" },
    { "iron_ore.png", 33, "terrain.png" },
    { "coal_ore.png", 34, "terrain.png" },
    { "glass.png", 49, "terrain.png" },
    { "glass_pane.png", 49, "terrain.png" },
    { "lapis_ore.png", 160, "terrain.png" },
    { "lapis_block.png", 144, "terrain.png" },
    { "mossy_cobblestone.png", 36, "terrain.png" },
    { "obsidian.png", 37, "terrain.png" },
    { "brick.png", 7, "terrain.png" },
    { "tnt.png", 8, "terrain.png" },
    { "gold_block.png", 23, "terrain.png" },
    { "iron_block.png", 22, "terrain.png" },
    { "diamond_ore.png", 50, "terrain.png" },
    { "diamond_block.png", 24, "terrain.png" },
    { "netherrack.png", 103, "terrain.png" },
    { "glowstone.png", 105, "terrain.png" },
    { "nether_brick.png", 224, "terrain.png" },
    { "glowing_obsidian.png", 218, "terrain.png" },
    { "cobweb.png", 11, "terrain.png" },
    { "cactus_side.png", 70, "terrain.png" },
    { "clay.png", 72, "terrain.png" },
    { "sugar_cane.png", 73, "terrain.png" },
    { "ice.png", 67, "terrain.png" },
    { "snow.png", 66, "terrain.png" },
    { "snow_layer.png", 66, "terrain.png" },
    { "redstone_ore.png", 51, "terrain.png" },
    { "redstone_ore_lit.png", 51, "terrain.png" },
    { "sapling.png", 15, "terrain.png" },
    { "sapling_oak.png", 15, "terrain.png" },
    { "sapling_spruce.png", 63, "terrain.png" },
    { "sapling_birch.png", 79, "terrain.png" },
    { "tall_grass.png", 39, "terrain.png" },
    { "mushroom_red.png", 29, "terrain.png" },
    { "mushroom_brown.png", 28, "terrain.png" },
    { "flower_dandelion.png", 13, "terrain.png" },
    { "flower_cornflower.png", 12, "terrain.png" },
    { "fire.png", 31, "terrain.png" },
    { "fire_0.png", 31, "terrain.png" },
    { "fire_1.png", 47, "terrain.png" },
    { "bedrock.png", 17, "terrain.png" },
    { "sponge.png", 48, "terrain.png" },
    { "trapdoor.png", 84, "terrain.png" },
    { "farmland_dry.png", 87, "terrain.png" },
    { "farmland_wet.png", 86, "terrain.png" },
    { "pumpkin_top.png", 102, "terrain.png" },
    { "pumpkin_side.png", 118, "terrain.png" },
    { "pumpkin_face.png", 119, "terrain.png" },
    { "pumpkin_face_lit.png", 120, "terrain.png" },
    { "stem_straight.png", 111, "terrain.png" },
    { "stem_bent.png", 127, "terrain.png" },

    // Beds
    { "bed_head_top.png", 134, "terrain.png" },
    { "bed_foot_top.png", 135, "terrain.png" },
    { "bed_head_side.png", 150, "terrain.png" },
    { "bed_foot_side.png", 151, "terrain.png" },
    { "bed_head_end.png", 166, "terrain.png" },
    { "bed_foot_end.png", 167, "terrain.png" },

    // Wool / Cloth (16 colors)
    { "wool_white.png", 64, "terrain.png" },
    { "wool_silver.png", 113, "terrain.png" },
    { "wool_gray.png", 114, "terrain.png" },
    { "wool_cyan.png", 129, "terrain.png" },
    { "wool_pink.png", 130, "terrain.png" },
    { "wool_purple.png", 145, "terrain.png" },
    { "wool_lime.png", 146, "terrain.png" },
    { "wool_blue.png", 161, "terrain.png" },
    { "wool_yellow.png", 162, "terrain.png" },
    { "wool_brown.png", 177, "terrain.png" },
    { "wool_light_blue.png", 178, "terrain.png" },
    { "wool_green.png", 193, "terrain.png" },
    { "wool_magenta.png", 194, "terrain.png" },
    { "wool_red.png", 209, "terrain.png" },
    { "wool_orange.png", 210, "terrain.png" },
    { "wool_black.png", 225, "terrain.png" },

    { "fence_oak.png", 4, "terrain.png" },
    { "wheat.png", 88, "terrain.png" },
    { "wheat_stage_0.png", 88, "terrain.png" },
    { "wheat_stage_1.png", 89, "terrain.png" },
    { "wheat_stage_2.png", 90, "terrain.png" },
    { "wheat_stage_3.png", 91, "terrain.png" },
    { "wheat_stage_4.png", 92, "terrain.png" },
    { "wheat_stage_5.png", 93, "terrain.png" },
    { "wheat_stage_6.png", 94, "terrain.png" },
    { "wheat_stage_7.png", 95, "terrain.png" },

    // Destroy stages (animaciones de rotura)
    { "destroy_stage_0.png", 240, "terrain.png" },
    { "destroy_stage_1.png", 241, "terrain.png" },
    { "destroy_stage_2.png", 242, "terrain.png" },
    { "destroy_stage_3.png", 243, "terrain.png" },
    { "destroy_stage_4.png", 244, "terrain.png" },
    { "destroy_stage_5.png", 245, "terrain.png" },
    { "destroy_stage_6.png", 246, "terrain.png" },
    { "destroy_stage_7.png", 247, "terrain.png" },
    { "destroy_stage_8.png", 248, "terrain.png" },
    { "destroy_stage_9.png", 249, "terrain.png" },

    // Terrain.png - Bloques Multicara
    { "grass_top.png", 0, "terrain.png" },
    { "grass_side.png", 3, "terrain.png" },
    { "grass_side_snow.png", 68, "terrain.png" },
    { "grass_side_overlay.png", 38, "terrain.png" },
    { "dirt_grass.png", 236, "terrain.png" },
    { "log_oak_top.png", 21, "terrain.png" },
    { "log_oak_side.png", 20, "terrain.png" },
    { "bookshelf_side.png", 35, "terrain.png" },
    { "melon_top.png", 137, "terrain.png" },
    { "melon_side.png", 136, "terrain.png" },
    { "furnace_top.png", 62, "terrain.png" },
    { "furnace_bottom.png", 62, "terrain.png" },
    { "furnace_front.png", 44, "terrain.png" },
    { "furnace_front_lit.png", 61, "terrain.png" },
    { "furnace_side.png", 45, "terrain.png" },
    { "chest_top.png", 25, "terrain.png" },
    { "chest_front.png", 27, "terrain.png" },
    { "chest_side.png", 26, "terrain.png" },
    { "crafting_table_top.png", 43, "terrain.png" },
    { "crafting_table_front.png", 60, "terrain.png" },
    { "crafting_table_side.png", 60, "terrain.png" },
    { "crafting_table_back.png", 59, "terrain.png" },
    { "stonecutter_top.png", 169, "terrain.png" },
    { "stonecutter_bottom.png", 62, "terrain.png" },
    { "stonecutter_front.png", 168, "terrain.png" },
    { "stonecutter_side.png", 45, "terrain.png" },
    { "sandstone_top.png", 176, "terrain.png" },
    { "sandstone_bottom.png", 208, "terrain.png" },
    { "sandstone_normal.png", 192, "terrain.png" },
    { "sandstone_chiseled.png", 229, "terrain.png" },
    { "sandstone_smooth.png", 230, "terrain.png" },
    { "stonebrick_normal.png", 54, "terrain.png" },
    { "stonebrick_mossy.png", 100, "terrain.png" },
    { "stonebrick_cracked.png", 101, "terrain.png" },
    { "quartz_top.png", 196, "terrain.png" },
    { "quartz_bottom.png", 211, "terrain.png" },
    { "quartz_side.png", 212, "terrain.png" },
    { "quartz_chiseled_top.png", 198, "terrain.png" },
    { "quartz_chiseled.png", 214, "terrain.png" },
    { "quartz_lines_top.png", 197, "terrain.png" },
    { "quartz_lines.png", 213, "terrain.png" },
    { "nether_reactor_core.png", 234, "terrain.png" },
    { "nether_reactor_active.png", 234, "terrain.png" },
    { "nether_reactor_used.png", 234, "terrain.png" },
    { "door_wood_top_bottom.png", 97, "terrain.png" },
    { "door_wood_upper.png", 81, "terrain.png" },
    { "door_wood_lower.png", 97, "terrain.png" },
    { "door_iron_top_bottom.png", 98, "terrain.png" },
    { "door_iron_upper.png", 82, "terrain.png" },
    { "door_iron_lower.png", 98, "terrain.png" },
    { "leaves_oak.png", 52, "terrain.png" },
    { "leaves_oak_opaque.png", 53, "terrain.png" },
    { "leaves_spruce.png", 132, "terrain.png" },
    { "leaves_spruce_opaque.png", 133, "terrain.png" },
    { "leaves_carried.png", 235, "terrain.png" },
    { "leaves_oak_carried.png", 235, "terrain.png" },

    // Terrain2.png - Atlas Secundario
    { "log_spruce_side.png", 0, "terrain2.png" },
    { "log_spruce_top.png", 1, "terrain2.png" },
    { "log_birch_side.png", 2, "terrain2.png" },
    { "log_birch_top.png", 3, "terrain2.png" },
    { "planks_spruce.png", 4, "terrain2.png" },
    { "planks_birch.png", 5, "terrain2.png" },
    { "deepslate.png", 6, "terrain2.png" },
    { "cobbled_deepslate.png", 7, "terrain2.png" },
    { "deepslate_coal_ore.png", 8, "terrain2.png" },
    { "deepslate_diamond_ore.png", 9, "terrain2.png" },
    { "deepslate_gold_ore.png", 10, "terrain2.png" },
    { "deepslate_iron_ore.png", 11, "terrain2.png" },
    { "deepslate_lapis_ore.png", 12, "terrain2.png" },
    { "deepslate_redstone_ore.png", 13, "terrain2.png" },
    { "deepslate_redstone_ore_lit.png", 13, "terrain2.png" },
    { "polished_deepslate.png", 18, "terrain2.png" },
    { "deepslate_tiles.png", 19, "terrain2.png" },
    { "deepslate_bricks.png", 20, "terrain2.png" },
    { "nether_quartz_ore.png", 23, "terrain2.png" },
    { "end_stone.png", 24, "terrain2.png" },
    { "soul_sand.png", 25, "terrain2.png" },
    { "fence_spruce.png", 4, "terrain2.png" },
    { "fence_birch.png", 5, "terrain2.png" },
    { "nether_wart_stage_0.png", 26, "terrain2.png" },
    { "nether_wart_stage_1.png", 27, "terrain2.png" },
    { "nether_wart_stage_2.png", 28, "terrain2.png" },
    { "nether_wart_block.png", 29, "terrain2.png" },
    { "ancient_debris_side.png", 30, "terrain2.png" },
    { "ancient_debris_top.png", 31, "terrain2.png" },

    // Wood, Planks, Saplings & Leaves Extensions
    { "log_jungle.png", 32, "terrain2.png" },
    { "log_jungle_top.png", 33, "terrain2.png" },
    { "log_acacia.png", 34, "terrain2.png" },
    { "log_acacia_top.png", 35, "terrain2.png" },
    { "planks_jungle.png", 36, "terrain2.png" },
    { "planks_acacia.png", 37, "terrain2.png" },
    { "planks_big_oak.png", 38, "terrain2.png" },
    { "sapling_jungle.png", 39, "terrain2.png" },
    { "sapling_acacia.png", 40, "terrain2.png" },
    { "leaves_birch.png", 41, "terrain2.png" },
    { "leaves_birch_opaque.png", 42, "terrain2.png" },
    { "leaves_jungle.png", 43, "terrain2.png" },
    { "leaves_jungle_opaque.png", 44, "terrain2.png" },
    { "leaves_acacia.png", 45, "terrain2.png" },
    { "leaves_acacia_opaque.png", 46, "terrain2.png" },
    { "leaves_big_oak.png", 47, "terrain2.png" },
    { "leaves_big_oak_opaque.png", 48, "terrain2.png" },
    { "leaves_spruce_carried.png", 49, "terrain2.png" },
    { "leaves_birch_carried.png", 50, "terrain2.png" },
    { "leaves_jungle_carried.png", 51, "terrain2.png" },
    { "leaves_acacia_carried.png", 52, "terrain2.png" },
    { "leaves_big_oak_carried.png", 53, "terrain2.png" },
    { "sapling_big_oak.png", 54, "terrain2.png" }
};

const std::vector<BlockAtlasStitcher::TextureMapping>& BlockAtlasStitcher::getTextureMappings() {
    return s_mappings;
}

bool BlockAtlasStitcher::insertTile(TextureData& dstAtlas, int slotIndex, const TextureData& srcTile) {
    if (!dstAtlas.data || !srcTile.data || dstAtlas.w <= 0 || dstAtlas.h <= 0 || srcTile.w <= 0 || srcTile.h <= 0) {
        return false;
    }

    int tileX = slotIndex & 0xf;
    int tileY = (slotIndex >> 4) & 0xf;
    int cellW = dstAtlas.w / 16;
    int cellH = dstAtlas.h / 16;
    int startX = tileX * cellW;
    int startY = tileY * cellH;

    for (int y = 0; y < cellH && (startY + y) < dstAtlas.h; ++y) {
        for (int x = 0; x < cellW && (startX + x) < dstAtlas.w; ++x) {
            int srcX = (x * srcTile.w) / cellW;
            int srcY = (y * srcTile.h) / cellH;
            int srcIdx = (srcY * srcTile.w + srcX) * 4;
            int dstIdx = ((startY + y) * dstAtlas.w + (startX + x)) * 4;

            dstAtlas.data[dstIdx + 0] = srcTile.data[srcIdx + 0];
            dstAtlas.data[dstIdx + 1] = srcTile.data[srcIdx + 1];
            dstAtlas.data[dstIdx + 2] = srcTile.data[srcIdx + 2];
            dstAtlas.data[dstIdx + 3] = srcTile.data[srcIdx + 3];
        }
    }
    return true;
}

void BlockAtlasStitcher::stitchAtlas(const std::string& atlasResourceName, TextureData& atlasData, AppPlatform* platform) {
    if (!platform) {
        return;
    }

    // Si el atlas en memoria no existe o no tiene datos (por ejemplo, si se borró terrain.png del disco),
    // creamos automáticamente un lienzo limpio en memoria de 256x256 RGBA
    if (!atlasData.data || atlasData.w <= 0 || atlasData.h <= 0) {
        atlasData.w = 256;
        atlasData.h = 256;
        atlasData.format = TEXF_UNCOMPRESSED_8888;
        atlasData.memoryHandledExternally = false;
        atlasData.data = new unsigned char[atlasData.w * atlasData.h * 4];
        std::memset(atlasData.data, 0, atlasData.w * atlasData.h * 4);
    }

    std::string cleanAtlasName = atlasResourceName;
    size_t lastSlash = cleanAtlasName.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        cleanAtlasName = cleanAtlasName.substr(lastSlash + 1);
    }

    int stitchedCount = 0;

    // 1. Stitch textures from static mapping table
    for (const auto& entry : s_mappings) {
        if (entry.atlasName == cleanAtlasName) {
            std::string blockFilePath = "data/images/blocks/" + entry.filename;
            TextureData tileData = platform->loadTexture(blockFilePath, false);
            if (tileData.data && tileData.w > 0 && tileData.h > 0) {
                if (insertTile(atlasData, entry.index, tileData)) {
                    stitchedCount++;
                }
                if (tileData.data && !tileData.memoryHandledExternally) {
                    delete[] tileData.data;
                    tileData.data = nullptr;
                }
            }
        }
    }

    // 2. Stitch textures configured dynamically on Tile instances
    for (int i = 0; i < 256; i++) {
        Tile* tile = Tile::tiles[i];
        if (!tile || !tile->hasMaterialInstances()) continue;

        for (int f = 0; f < 6; f++) {
            Tile::BlockFace bf = (Tile::BlockFace)f;
            const Tile::MaterialInstance* matInst = tile->getMaterialInstance(bf);
            if (!matInst || !matInst->usesSeparateTexture()) continue;

            int atlasIdx = matInst->textureIndex;
            bool isAlt = (atlasIdx & Tile::TEXTURE_ALT_FLAG) != 0;
            std::string targetAtlas = isAlt ? "terrain2.png" : "terrain.png";

            if (targetAtlas == cleanAtlasName) {
                int cleanIdx = atlasIdx & ~Tile::TEXTURE_ALT_FLAG;
                std::string blockFilePath = "data/images/blocks/" + matInst->textureName + ".png";
                TextureData tileData = platform->loadTexture(blockFilePath, false);
                if (tileData.data && tileData.w > 0 && tileData.h > 0) {
                    if (insertTile(atlasData, cleanIdx, tileData)) {
                        stitchedCount++;
                    }
                    if (tileData.data && !tileData.memoryHandledExternally) {
                        delete[] tileData.data;
                        tileData.data = nullptr;
                    }
                }
            }
        }
    }
}
