#include "PanoramaRegistry.h"

std::vector<PanoramaInfo> PanoramaRegistry::getAllPanoramas() {
    std::vector<PanoramaInfo> list;

    // Default PocketMC Panorama
    list.push_back({
        "default",
        "PocketMC Classic",
        "Classic",
        "The classic original PocketMC menu panorama.",
        "gui/panorama/",
        "gui/panorama/panorama_0.png"
    });

    // Java & Bedrock
    list.push_back({
        "cherry_grove",
        "Cherry Grove",
        "Java & Bedrock",
        "A serene forest of cherry blossom trees.",
        "gui/panoramas/Java_and_Bedrock/garden_awakens/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/garden_awakens/textures/ui/panorama_0.png"
    });

    list.push_back({
        "buzzy_bees",
        "Buzzy Bees",
        "Java & Bedrock",
        "Flowery meadows filled with friendly bees and honeycombs.",
        "gui/panoramas/Java_and_Bedrock/buzzy_bees/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/buzzy_bees/textures/ui/panorama_0.png"
    });

    list.push_back({
        "caves",
        "Caves & Cliffs Part I",
        "Java & Bedrock",
        "Deep underground caverns and lush cave flora.",
        "gui/panoramas/Java_and_Bedrock/caves/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/caves/textures/ui/panorama_0.png"
    });

    list.push_back({
        "cliffs",
        "Caves & Cliffs Part II",
        "Java & Bedrock",
        "Majestic snow-capped mountain peaks and high cliffs.",
        "gui/panoramas/Java_and_Bedrock/cliffs/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/cliffs/textures/ui/panorama_0.png"
    });

    list.push_back({
        "nether",
        "Nether Wastes",
        "Java & Bedrock",
        "Fiery depths of the Nether with crimson and warped forests.",
        "gui/panoramas/Java_and_Bedrock/nether/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/nether/textures/ui/panorama_0.png"
    });

    list.push_back({
        "tricky_trials",
        "Tricky Trials",
        "Java & Bedrock",
        "Mysterious trial chambers with copper and breeze spawners.",
        "gui/panoramas/Java_and_Bedrock/tricky_trials/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/tricky_trials/textures/ui/panorama_0.png"
    });

    list.push_back({
        "wild",
        "The Wild Update",
        "Java & Bedrock",
        "Ancient cities, deep dark sculk, and mangrove swamps.",
        "gui/panoramas/Java_and_Bedrock/wild/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/wild/textures/ui/panorama_0.png"
    });

    list.push_back({
        "trails_and_tales",
        "Trails & Tales",
        "Java & Bedrock",
        "Archaeology, armor trims, camels, and cherry blossoms.",
        "gui/panoramas/Java_and_Bedrock/trails_and_tales/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/trails_and_tales/textures/ui/panorama_0.png"
    });

    // Bedrock
    list.push_back({
        "aquatic_bedrock",
        "Update Aquatic",
        "Bedrock",
        "Vibrant ocean reefs, shipwrecks, and sea life.",
        "gui/panoramas/Bedrock/aquatic_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/aquatic_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "village_and_pillage_bedrock",
        "Village & Pillage",
        "Bedrock",
        "Bustling villages with new villager professions and pillager outposts.",
        "gui/panoramas/Bedrock/village_and_pillage_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/village_and_pillage_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "cats_and_pandas",
        "Cats & Pandas",
        "Bedrock",
        "Bamboo jungles with playful pandas and stray cats.",
        "gui/panoramas/Bedrock/cats_and_pandas/textures/ui/",
        "gui/panoramas/Bedrock/cats_and_pandas/textures/ui/panorama_0.png"
    });

    list.push_back({
        "better_together",
        "Better Together",
        "Bedrock",
        "The landmark cross-platform Bedrock update panorama.",
        "gui/panoramas/Bedrock/better_together/textures/ui/",
        "gui/panoramas/Bedrock/better_together/textures/ui/panorama_0.png"
    });

    // Java
    list.push_back({
        "classic_java",
        "Classic Java (Beta 1.8)",
        "Java",
        "The nostalgic classic panorama from Minecraft Beta 1.8.",
        "gui/panoramas/Java/classic/textures/ui/",
        "gui/panoramas/Java/classic/textures/ui/panorama_0.png"
    });

    list.push_back({
        "indev_java",
        "Indev",
        "Java",
        "Retro panorama from early Indev development.",
        "gui/panoramas/Java/indev/textures/ui/",
        "gui/panoramas/Java/indev/textures/ui/panorama_0.png"
    });

    return list;
}

PanoramaInfo PanoramaRegistry::getPanoramaById(const std::string& id) {
    auto all = getAllPanoramas();
    for (const auto& p : all) {
        if (p.id == id || p.folderPath == id) return p;
    }
    return getDefaultPanorama();
}

PanoramaInfo PanoramaRegistry::getDefaultPanorama() {
    return {
        "default",
        "PocketMC Classic",
        "Classic",
        "The classic original PocketMC menu panorama.",
        "gui/panorama/",
        "gui/panorama/panorama_0.png"
    };
}
