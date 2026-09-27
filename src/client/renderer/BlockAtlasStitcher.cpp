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
    { "wool_colored_white.png", 64, "terrain.png" },
    { "wool_white.png", 64, "terrain.png" },
    { "wool_colored_black.png", 113, "terrain.png" },
    { "wool_black.png", 113, "terrain.png" },
    { "wool_colored_gray.png", 114, "terrain.png" },
    { "wool_gray.png", 114, "terrain.png" },
    { "wool_colored_red.png", 129, "terrain.png" },
    { "wool_red.png", 129, "terrain.png" },
    { "wool_colored_pink.png", 130, "terrain.png" },
    { "wool_pink.png", 130, "terrain.png" },
    { "wool_colored_green.png", 145, "terrain.png" },
    { "wool_green.png", 145, "terrain.png" },
    { "wool_colored_lime.png", 146, "terrain.png" },
    { "wool_lime.png", 146, "terrain.png" },
    { "wool_colored_brown.png", 161, "terrain.png" },
    { "wool_brown.png", 161, "terrain.png" },
    { "wool_colored_yellow.png", 162, "terrain.png" },
    { "wool_yellow.png", 162, "terrain.png" },
    { "wool_colored_blue.png", 177, "terrain.png" },
    { "wool_blue.png", 177, "terrain.png" },
    { "wool_colored_light_blue.png", 178, "terrain.png" },
    { "wool_light_blue.png", 178, "terrain.png" },
    { "wool_colored_purple.png", 193, "terrain.png" },
    { "wool_purple.png", 193, "terrain.png" },
    { "wool_colored_magenta.png", 194, "terrain.png" },
    { "wool_magenta.png", 194, "terrain.png" },
    { "wool_colored_cyan.png", 209, "terrain.png" },
    { "wool_cyan.png", 209, "terrain.png" },
    { "wool_colored_orange.png", 210, "terrain.png" },
    { "wool_orange.png", 210, "terrain.png" },
    { "wool_colored_silver.png", 225, "terrain.png" },
    { "wool_silver.png", 225, "terrain.png" },

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
    { "sapling_big_oak.png", 54, "terrain2.png" },
    { "leaves_jungle_fruits.png", 55, "terrain2.png" },

    // Stones & Polished (56..61)
    { "stone_granite.png", 56, "terrain2.png" },
    { "stone_granite_smooth.png", 57, "terrain2.png" },
    { "stone_diorite.png", 58, "terrain2.png" },
    { "stone_diorite_smooth.png", 59, "terrain2.png" },
    { "stone_andesite.png", 60, "terrain2.png" },
    { "stone_andesite_smooth.png", 61, "terrain2.png" },

    // Soils & Organics (62..65)
    { "dirt_podzol_top.png", 62, "terrain2.png" },
    { "dirt_podzol_side.png", 63, "terrain2.png" },
    { "mycelium_top.png", 64, "terrain2.png" },
    { "mycelium_side.png", 65, "terrain2.png" },

    // Ices (66..68)
    { "ice_packed.png", 66, "terrain2.png" },
    { "blue_ice.png", 67, "terrain2.png" },
    { "frosted_ice_0.png", 68, "terrain2.png" },

    // Terracotta & 16 Stained Clays (69..85)
    { "hardened_clay.png", 69, "terrain2.png" },
    { "hardened_clay_stained_white.png", 70, "terrain2.png" },
    { "hardened_clay_stained_orange.png", 71, "terrain2.png" },
    { "hardened_clay_stained_magenta.png", 72, "terrain2.png" },
    { "hardened_clay_stained_light_blue.png", 73, "terrain2.png" },
    { "hardened_clay_stained_yellow.png", 74, "terrain2.png" },
    { "hardened_clay_stained_lime.png", 75, "terrain2.png" },
    { "hardened_clay_stained_pink.png", 76, "terrain2.png" },
    { "hardened_clay_stained_gray.png", 77, "terrain2.png" },
    { "hardened_clay_stained_silver.png", 78, "terrain2.png" },
    { "hardened_clay_stained_cyan.png", 79, "terrain2.png" },
    { "hardened_clay_stained_purple.png", 80, "terrain2.png" },
    { "hardened_clay_stained_blue.png", 81, "terrain2.png" },
    { "hardened_clay_stained_brown.png", 82, "terrain2.png" },
    { "hardened_clay_stained_green.png", 83, "terrain2.png" },
    { "hardened_clay_stained_red.png", 84, "terrain2.png" },
    { "hardened_clay_stained_black.png", 85, "terrain2.png" },

    // Huge Mushrooms (86..89)
    { "mushroom_block_skin_brown.png", 86, "terrain2.png" },
    { "mushroom_block_skin_red.png", 87, "terrain2.png" },
    { "mushroom_block_skin_stem.png", 88, "terrain2.png" },
    { "mushroom_block_inside.png", 89, "terrain2.png" },

    // Flora & Special (90..93)
    { "waterlily.png", 90, "terrain2.png" },
    { "vine.png", 91, "terrain2.png" },
    { "cocoa_stage_2.png", 92, "terrain2.png" },
    { "sweet_berry_bush_stage3.png", 93, "terrain2.png" },

    // 1.7+ Flowers (94..105)
    { "flower_allium.png", 94, "terrain2.png" },
    { "flower_blue_orchid.png", 95, "terrain2.png" },
    { "flower_houstonia.png", 96, "terrain2.png" },
    { "flower_tulip_red.png", 97, "terrain2.png" },
    { "flower_tulip_orange.png", 98, "terrain2.png" },
    { "flower_tulip_white.png", 99, "terrain2.png" },
    { "flower_tulip_pink.png", 100, "terrain2.png" },
    { "flower_oxeye_daisy.png", 101, "terrain2.png" },
    { "flower_paeonia.png", 102, "terrain2.png" },
    { "flower_rose_blue.png", 103, "terrain2.png" },
    { "flower_lily_of_the_valley.png", 104, "terrain2.png" },
    { "flower_wither_rose.png", 105, "terrain2.png" },

    // 3D Blocks, Torches & Workstations (FutureMC Authentic Textures)
    // Lanterns, Chains & Torches
    { "lantern.png", 160, "terrain2.png" },
    { "soul_lantern.png", 217, "terrain2.png" },
    { "soul_fire_lantern.png", 217, "terrain2.png" },
    { "soul_torch.png", 250, "terrain2.png" },
    { "soul_fire_torch.png", 250, "terrain2.png" },
    { "soul_fire_0.png", 216, "terrain2.png" },
    { "soul_fire_1.png", 216, "terrain2.png" },
    { "soul_fire.png", 216, "terrain2.png" },
    { "redstone_torch_on.png", 205, "terrain2.png" },
    { "redstone_torch_off.png", 231, "terrain2.png" },
    { "chain.png", 155, "terrain2.png" },

    // Stonecutter (251..254)
    { "stonecutter_top.png", 251, "terrain2.png" },
    { "stonecutter_bottom.png", 252, "terrain2.png" },
    { "stonecutter_side.png", 253, "terrain2.png" },
    { "stonecutter_saw.png", 254, "terrain2.png" },

    // Composter (131, 255, 14..16)
    { "composter_top.png", 131, "terrain2.png" },
    { "composter_bottom.png", 255, "terrain2.png" },
    { "composter_side.png", 14, "terrain2.png" },
    { "composter_compost.png", 15, "terrain2.png" },
    { "composter_ready.png", 16, "terrain2.png" },

    // Grindstone (151, 17, 21)
    { "grindstone_pivot.png", 151, "terrain2.png" },
    { "grindstone_round.png", 17, "terrain2.png" },
    { "grindstone_side.png", 21, "terrain2.png" },

    // Lectern (162, 22, 106, 107)
    { "lectern_base.png", 162, "terrain2.png" },
    { "lectern_front.png", 22, "terrain2.png" },
    { "lectern_sides.png", 106, "terrain2.png" },
    { "lectern_top.png", 107, "terrain2.png" },

    // Campfire (118, 108..112)
    { "campfire_log.png", 118, "terrain2.png" },
    { "campfire_side.png", 108, "terrain2.png" },
    { "campfire_fire.png", 109, "terrain2.png" },
    { "campfire_log_lit.png", 110, "terrain2.png" },
    { "soul_campfire_fire.png", 111, "terrain2.png" },
    { "soul_campfire_log_lit.png", 112, "terrain2.png" },

    // Blast Furnace (113..115)
    { "blast_furnace_top.png", 113, "terrain2.png" },
    { "blast_furnace_front.png", 114, "terrain2.png" },
    { "blast_furnace_side.png", 115, "terrain2.png" },

    // Smoker (212, 116, 117, 119)
    { "smoker_top.png", 212, "terrain2.png" },
    { "smoker_bottom.png", 116, "terrain2.png" },
    { "smoker_front.png", 117, "terrain2.png" },
    { "smoker_side.png", 119, "terrain2.png" },

    // Cartography Table (120..123)
    { "cartography_table_top.png", 120, "terrain2.png" },
    { "cartography_table_side1.png", 121, "terrain2.png" },
    { "cartography_table_side2.png", 122, "terrain2.png" },
    { "cartography_table_side3.png", 123, "terrain2.png" },

    // Fletching Table (145, 124, 125)
    { "fletching_table_top.png", 145, "terrain2.png" },
    { "fletching_table_front.png", 124, "terrain2.png" },
    { "fletching_table_side.png", 125, "terrain2.png" },

    // Smithing Table (211, 126..128)
    { "smithing_table_top.png", 211, "terrain2.png" },
    { "smithing_table_bottom.png", 126, "terrain2.png" },
    { "smithing_table_front.png", 127, "terrain2.png" },
    { "smithing_table_side.png", 128, "terrain2.png" },

    // Loom (169, 129, 130, 132)
    { "loom_top.png", 169, "terrain2.png" },
    { "loom_bottom.png", 129, "terrain2.png" },
    { "loom_front.png", 130, "terrain2.png" },
    { "loom_side.png", 132, "terrain2.png" },

    // Barrel (133..135)
    { "barrel_top.png", 133, "terrain2.png" },
    { "barrel_bottom.png", 134, "terrain2.png" },
    { "barrel_side.png", 135, "terrain2.png" },

    // Bell (136..138)
    { "bell_top.png", 136, "terrain2.png" },
    { "bell_bottom.png", 137, "terrain2.png" },
    { "bell_side.png", 138, "terrain2.png" },

    // Candles (139..141)
    { "candle.png", 139, "terrain2.png" },
    { "candle_lit.png", 139, "terrain2.png" },
    { "red_candle.png", 140, "terrain2.png" },
    { "red_candle_lit.png", 140, "terrain2.png" },
    { "white_candle.png", 141, "terrain2.png" },
    { "white_candle_lit.png", 141, "terrain2.png" },

    // Cauldron (149, 150, 163, 165)
    { "cauldron_top.png", 149, "terrain2.png" },
    { "cauldron_inner.png", 150, "terrain2.png" },
    { "cauldron_side.png", 163, "terrain2.png" },
    { "cauldron_bottom.png", 165, "terrain2.png" },

    // Anvil (166, 170, 180, 185)
    { "anvil_base.png", 166, "terrain2.png" },
    { "anvil_top_damaged_0.png", 170, "terrain2.png" },
    { "anvil_top_damaged_1.png", 180, "terrain2.png" },
    { "anvil_top_damaged_2.png", 185, "terrain2.png" },

    // Hopper (195, 199, 200)
    { "hopper_top.png", 195, "terrain2.png" },
    { "hopper_inside.png", 199, "terrain2.png" },
    { "hopper_outside.png", 200, "terrain2.png" }
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

    // Si la imagen es una tira vertical animada (ej: 16x48, 16x128), extraemos solo el primer cuadro cuadrado (16x16)
    int frameH = srcTile.w;
    if (frameH > srcTile.h) frameH = srcTile.h;

    for (int y = 0; y < cellH && (startY + y) < dstAtlas.h; ++y) {
        for (int x = 0; x < cellW && (startX + x) < dstAtlas.w; ++x) {
            int srcX = (x * srcTile.w) / cellW;
            int srcY = (y * frameH) / cellH;
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
    for (int i = 0; i < Tile::NUM_BLOCK_TYPES; i++) {
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

TextureCategory BlockAtlasStitcher::getTextureCategory(const std::string& filename) {
    std::string name = filename;
    for (auto& c : name) c = (char)tolower(c);
    if (name.find("leaves") != std::string::npos ||
        name.find("sapling") != std::string::npos ||
        name.find("flower") != std::string::npos ||
        name.find("grass") != std::string::npos ||
        name.find("crop") != std::string::npos ||
        name.find("wheat") != std::string::npos ||
        name.find("dandelion") != std::string::npos ||
        name.find("rose") != std::string::npos ||
        name.find("fern") != std::string::npos ||
        name.find("vine") != std::string::npos ||
        name.find("sugar_cane") != std::string::npos ||
        name.find("nether_wart") != std::string::npos ||
        name.find("reeds") != std::string::npos ||
        name.find("fire") != std::string::npos ||
        name.find("torch") != std::string::npos ||
        name.find("lantern") != std::string::npos ||
        name.find("campfire") != std::string::npos ||
        name.find("lectern") != std::string::npos ||
        name.find("composter") != std::string::npos ||
        name.find("grindstone") != std::string::npos ||
        name.find("scaffolding") != std::string::npos ||
        name.find("chain") != std::string::npos ||
        name.find("candle") != std::string::npos ||
        name.find("saw") != std::string::npos ||
        name.find("cauldron") != std::string::npos ||
        name.find("hopper") != std::string::npos ||
        name.find("cactus") != std::string::npos) {
        return CAT_CUTOUT;
    }
    if (name.find("water") != std::string::npos ||
        name.find("ice") != std::string::npos ||
        name.find("glass") != std::string::npos) {
        return CAT_TRANSLUCENT;
    }
    return CAT_OPAQUE;
}

void BlockAtlasStitcher::downsampleTile(const unsigned char* src, int srcW, int srcH, unsigned char* dst, TextureCategory cat) {
    int dstW = srcW / 2;
    int dstH = srcH / 2;
    if (dstW < 1) dstW = 1;
    if (dstH < 1) dstH = 1;

    const int alphaThreshold = 25; // 0.1 * 255 (PocketMC alpha test cutoff)

    float origCoverage = 0.0f;
    if (cat == CAT_CUTOUT) {
        int passCount = 0;
        int totalPix = srcW * srcH;
        for (int i = 0; i < totalPix; i++) {
            if (src[i * 4 + 3] >= alphaThreshold) {
                passCount++;
            }
        }
        origCoverage = (float)passCount / (float)totalPix;
    }

    struct RawTexel {
        unsigned char r, g, b, a;
        int idx;
    };
    std::vector<RawTexel> texels(dstW * dstH);

    for (int y = 0; y < dstH; y++) {
        for (int x = 0; x < dstW; x++) {
            int srcX0 = x * 2;
            int srcY0 = y * 2;
            int srcX1 = std::min(srcX0 + 1, srcW - 1);
            int srcY1 = std::min(srcY0 + 1, srcH - 1);

            int i00 = (srcY0 * srcW + srcX0) * 4;
            int i10 = (srcY0 * srcW + srcX1) * 4;
            int i01 = (srcY1 * srcW + srcX0) * 4;
            int i11 = (srcY1 * srcW + srcX1) * 4;

            int idxs[4] = { i00, i10, i01, i11 };

            if (cat == CAT_OPAQUE) {
                int r = 0, g = 0, b = 0, a = 0;
                for (int k = 0; k < 4; k++) {
                    r += src[idxs[k] + 0];
                    g += src[idxs[k] + 1];
                    b += src[idxs[k] + 2];
                    a += src[idxs[k] + 3];
                }
                int dstIdx = y * dstW + x;
                texels[dstIdx] = { (unsigned char)(r / 4), (unsigned char)(g / 4), (unsigned char)(b / 4), (unsigned char)(a / 4), dstIdx };
            } else {
                // Alpha-weighted RGB
                float rSum = 0.0f, gSum = 0.0f, bSum = 0.0f, wSum = 0.0f;
                int aSum = 0;
                for (int k = 0; k < 4; k++) {
                    float w = src[idxs[k] + 3] / 255.0f;
                    rSum += src[idxs[k] + 0] * w;
                    gSum += src[idxs[k] + 1] * w;
                    bSum += src[idxs[k] + 2] * w;
                    wSum += w;
                    aSum += src[idxs[k] + 3];
                }
                unsigned char r = 0, g = 0, b = 0;
                if (wSum > 0.0f) {
                    r = (unsigned char)std::min(255.0f, rSum / wSum);
                    g = (unsigned char)std::min(255.0f, gSum / wSum);
                    b = (unsigned char)std::min(255.0f, bSum / wSum);
                }
                unsigned char a = (unsigned char)(aSum / 4);
                int dstIdx = y * dstW + x;
                texels[dstIdx] = { r, g, b, a, dstIdx };
            }
        }
    }

    if (cat == CAT_CUTOUT && origCoverage > 0.0f) {
        int totalDstPix = dstW * dstH;
        int targetPassCount = (int)std::round(origCoverage * totalDstPix);
        if (targetPassCount < 1 && origCoverage > 0.0f) targetPassCount = 1;

        std::vector<RawTexel> sortedTexels = texels;
        std::sort(sortedTexels.begin(), sortedTexels.end(), [](const RawTexel& a, const RawTexel& b) {
            return a.a > b.a;
        });

        for (int i = 0; i < totalDstPix; i++) {
            int origIdx = sortedTexels[i].idx;
            if (i < targetPassCount) {
                if (texels[origIdx].a < alphaThreshold) {
                    texels[origIdx].a = 255;
                }
            } else {
                if (sortedTexels[i].a < alphaThreshold) {
                    texels[origIdx].a = 0;
                }
            }
        }
    }

    for (int i = 0; i < dstW * dstH; i++) {
        dst[i * 4 + 0] = texels[i].r;
        dst[i * 4 + 1] = texels[i].g;
        dst[i * 4 + 2] = texels[i].b;
        dst[i * 4 + 3] = texels[i].a;
    }
}

std::vector<TextureData> BlockAtlasStitcher::stitchAtlasMultiLevel(const std::string& atlasResourceName, TextureData& atlasL0, AppPlatform* platform, int maxLevels) {
    stitchAtlas(atlasResourceName, atlasL0, platform);

    int actualLevels = std::min(maxLevels, 4);
    std::vector<TextureData> atlasLevels(actualLevels + 1);
    atlasLevels[0] = atlasL0;

    int baseW = atlasL0.w > 0 ? atlasL0.w : 256;
    int baseH = atlasL0.h > 0 ? atlasL0.h : 256;

    for (int lvl = 1; lvl <= actualLevels; lvl++) {
        int w = baseW >> lvl;
        int h = baseH >> lvl;
        atlasLevels[lvl].w = w;
        atlasLevels[lvl].h = h;
        atlasLevels[lvl].format = TEXF_UNCOMPRESSED_8888;
        atlasLevels[lvl].memoryHandledExternally = false;
        atlasLevels[lvl].data = new unsigned char[w * h * 4];
        std::memset(atlasLevels[lvl].data, 0, w * h * 4);
    }

    std::map<int, TextureCategory> slotCategories;
    std::string cleanAtlasName = atlasResourceName;
    size_t lastSlash = cleanAtlasName.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        cleanAtlasName = cleanAtlasName.substr(lastSlash + 1);
    }

    for (const auto& entry : s_mappings) {
        if (entry.atlasName == cleanAtlasName) {
            slotCategories[entry.index] = getTextureCategory(entry.filename);
        }
    }

    for (int i = 0; i < Tile::NUM_BLOCK_TYPES; i++) {
        Tile* tile = Tile::tiles[i];
        if (!tile || !tile->hasMaterialInstances()) continue;
        for (int f = 0; f < 6; f++) {
            const Tile::MaterialInstance* matInst = tile->getMaterialInstance((Tile::BlockFace)f);
            if (!matInst || !matInst->usesSeparateTexture()) continue;
            int atlasIdx = matInst->textureIndex;
            bool isAlt = (atlasIdx & Tile::TEXTURE_ALT_FLAG) != 0;
            std::string targetAtlas = isAlt ? "terrain2.png" : "terrain.png";
            if (targetAtlas == cleanAtlasName) {
                int cleanIdx = atlasIdx & ~Tile::TEXTURE_ALT_FLAG;
                slotCategories[cleanIdx] = getTextureCategory(matInst->textureName);
            }
        }
    }

    for (int slot = 0; slot < 256; slot++) {
        TextureCategory cat = CAT_OPAQUE;
        auto catIt = slotCategories.find(slot);
        if (catIt != slotCategories.end()) {
            cat = catIt->second;
        }

        unsigned char tileL0[16 * 16 * 4];
        unsigned char tileL1[8 * 8 * 4];
        unsigned char tileL2[4 * 4 * 4];
        unsigned char tileL3[2 * 2 * 4];
        unsigned char tileL4[1 * 1 * 4];

        unsigned char* tileBufs[5] = { tileL0, tileL1, tileL2, tileL3, tileL4 };

        int tileX = slot & 0xf;
        int tileY = (slot >> 4) & 0xf;
        int cellW = baseW / 16;
        int cellH = baseH / 16;
        int startX = tileX * cellW;
        int startY = tileY * cellH;

        for (int y = 0; y < cellH; y++) {
            for (int x = 0; x < cellW; x++) {
                int srcIdx = ((startY + y) * baseW + (startX + x)) * 4;
                int dstIdx = (y * cellW + x) * 4;
                tileL0[dstIdx + 0] = atlasL0.data[srcIdx + 0];
                tileL0[dstIdx + 1] = atlasL0.data[srcIdx + 1];
                tileL0[dstIdx + 2] = atlasL0.data[srcIdx + 2];
                tileL0[dstIdx + 3] = atlasL0.data[srcIdx + 3];
            }
        }

        for (int lvl = 1; lvl <= actualLevels; lvl++) {
            int prevS = 16 >> (lvl - 1);
            int currS = 16 >> lvl;
            downsampleTile(tileBufs[lvl - 1], prevS, prevS, tileBufs[lvl], cat);

            int lvlAtlasW = baseW >> lvl;
            int lvlCellW = lvlAtlasW / 16;
            int lvlCellH = lvlAtlasW / 16;
            int lvlStartX = tileX * lvlCellW;
            int lvlStartY = tileY * lvlCellH;

            for (int y = 0; y < lvlCellH; y++) {
                for (int x = 0; x < lvlCellW; x++) {
                    int srcIdx = (y * lvlCellW + x) * 4;
                    int dstIdx = ((lvlStartY + y) * lvlAtlasW + (lvlStartX + x)) * 4;
                    atlasLevels[lvl].data[dstIdx + 0] = tileBufs[lvl][srcIdx + 0];
                    atlasLevels[lvl].data[dstIdx + 1] = tileBufs[lvl][srcIdx + 1];
                    atlasLevels[lvl].data[dstIdx + 2] = tileBufs[lvl][srcIdx + 2];
                    atlasLevels[lvl].data[dstIdx + 3] = tileBufs[lvl][srcIdx + 3];
                }
            }
        }
    }

    return atlasLevels;
}
