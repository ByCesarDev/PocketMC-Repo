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

    list.push_back({
        "craftmine",
        "Craftmine (April Fools)",
        "April Fools",
        "Official Craftmine (April Fools) panorama background.",
        "gui/panoramas/April_Fools/craftmine/textures/ui/",
        "gui/panoramas/April_Fools/craftmine/textures/ui/panorama_0.png"
    });

    list.push_back({
        "herdcraft",
        "Herdcraft (April Fools)",
        "April Fools",
        "Official Herdcraft (April Fools) panorama background.",
        "gui/panoramas/April_Fools/herdcraft/textures/ui/",
        "gui/panoramas/April_Fools/herdcraft/textures/ui/panorama_0.png"
    });

    list.push_back({
        "poisonous_potato",
        "Poisonous Potato (April Fools)",
        "April Fools",
        "Official Poisonous Potato (April Fools) panorama background.",
        "gui/panoramas/April_Fools/poisonous_potato/textures/ui/",
        "gui/panoramas/April_Fools/poisonous_potato/textures/ui/panorama_0.png"
    });

    list.push_back({
        "aquatic_bedrock",
        "Aquatic Bedrock (Bedrock)",
        "Bedrock",
        "Official Aquatic Bedrock (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/aquatic_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/aquatic_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "bedrock_beta",
        "Bedrock Beta (Bedrock)",
        "Bedrock",
        "Official Bedrock Beta (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/bedrock_beta/textures/ui/",
        "gui/panoramas/Bedrock/bedrock_beta/textures/ui/panorama_0.png"
    });

    list.push_back({
        "better_together",
        "Better Together (Bedrock)",
        "Bedrock",
        "Official Better Together (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/better_together/textures/ui/",
        "gui/panoramas/Bedrock/better_together/textures/ui/panorama_0.png"
    });

    list.push_back({
        "buzzy_bees_bedrock",
        "Buzzy Bees Bedrock (Bedrock)",
        "Bedrock",
        "Official Buzzy Bees Bedrock (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/buzzy_bees_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/buzzy_bees_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "cats_and_pandas",
        "Cats And Pandas (Bedrock)",
        "Bedrock",
        "Official Cats And Pandas (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/cats_and_pandas/textures/ui/",
        "gui/panoramas/Bedrock/cats_and_pandas/textures/ui/panorama_0.png"
    });

    list.push_back({
        "chase_the_skies_bedrock",
        "Chase The Skies Bedrock (Bedrock)",
        "Bedrock",
        "Official Chase The Skies Bedrock (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/chase_the_skies_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/chase_the_skies_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "chase_the_skies_bedrock_1.21.90.25",
        "Chase The Skies Bedrock 1.21.90.25 (Bedrock)",
        "Bedrock",
        "Official Chase The Skies Bedrock 1.21.90.25 (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/chase_the_skies_bedrock_1.21.90.25/textures/ui/",
        "gui/panoramas/Bedrock/chase_the_skies_bedrock_1.21.90.25/textures/ui/panorama_0.png"
    });

    list.push_back({
        "christmas_bedrock",
        "Christmas Bedrock (Bedrock)",
        "Bedrock",
        "Official Christmas Bedrock (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/christmas_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/christmas_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "halloween_2021_bedrock",
        "Halloween 2021 Bedrock (Bedrock)",
        "Bedrock",
        "Official Halloween 2021 Bedrock (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/halloween_2021_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/halloween_2021_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "preview_bedrock",
        "Preview Bedrock (Bedrock)",
        "Bedrock",
        "Official Preview Bedrock (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/preview_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/preview_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "tiny_takeover_bedrock",
        "Tiny Takeover Bedrock (Bedrock)",
        "Bedrock",
        "Official Tiny Takeover Bedrock (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/tiny_takeover_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/tiny_takeover_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "village_and_pillage_bedrock",
        "Village And Pillage Bedrock (Bedrock)",
        "Bedrock",
        "Official Village And Pillage Bedrock (Bedrock) panorama background.",
        "gui/panoramas/Bedrock/village_and_pillage_bedrock/textures/ui/",
        "gui/panoramas/Bedrock/village_and_pillage_bedrock/textures/ui/panorama_0.png"
    });

    list.push_back({
        "chase_the_skies_bedrock-vv",
        "Chase The Skies Bedrock-Vv (VV)",
        "Bedrock (Vibrant Visuals)",
        "Official Chase The Skies Bedrock-Vv (VV) panorama background.",
        "gui/panoramas/Bedrock_Vibrant_Visuals/chase_the_skies_bedrock-vv/textures/ui/",
        "gui/panoramas/Bedrock_Vibrant_Visuals/chase_the_skies_bedrock-vv/textures/ui/panorama_0.png"
    });

    list.push_back({
        "copper_age_bedrock-vv",
        "Copper Age Bedrock-Vv (VV)",
        "Bedrock (Vibrant Visuals)",
        "Official Copper Age Bedrock-Vv (VV) panorama background.",
        "gui/panoramas/Bedrock_Vibrant_Visuals/copper_age_bedrock-vv/textures/ui/",
        "gui/panoramas/Bedrock_Vibrant_Visuals/copper_age_bedrock-vv/textures/ui/panorama_0.png"
    });

    list.push_back({
        "mounts_of_mayhem_bedrock-vv",
        "Mounts Of Mayhem Bedrock-Vv (VV)",
        "Bedrock (Vibrant Visuals)",
        "Official Mounts Of Mayhem Bedrock-Vv (VV) panorama background.",
        "gui/panoramas/Bedrock_Vibrant_Visuals/mounts_of_mayhem_bedrock-vv/textures/ui/",
        "gui/panoramas/Bedrock_Vibrant_Visuals/mounts_of_mayhem_bedrock-vv/textures/ui/panorama_0.png"
    });

    list.push_back({
        "tiny_takeover_bedrock-vv",
        "Tiny Takeover Bedrock-Vv (VV)",
        "Bedrock (Vibrant Visuals)",
        "Official Tiny Takeover Bedrock-Vv (VV) panorama background.",
        "gui/panoramas/Bedrock_Vibrant_Visuals/tiny_takeover_bedrock-vv/textures/ui/",
        "gui/panoramas/Bedrock_Vibrant_Visuals/tiny_takeover_bedrock-vv/textures/ui/panorama_0.png"
    });

    list.push_back({
        "1.14_demo_education",
        "1.14 Demo Education (Edu)",
        "Education Edition",
        "Official 1.14 Demo Education (Edu) panorama background.",
        "gui/panoramas/Education/1.14_demo_education/textures/ui/",
        "gui/panoramas/Education/1.14_demo_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "1.14_education",
        "1.14 Education (Edu)",
        "Education Edition",
        "Official 1.14 Education (Edu) panorama background.",
        "gui/panoramas/Education/1.14_education/textures/ui/",
        "gui/panoramas/Education/1.14_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "chase_the_clouds_education",
        "Chase The Clouds Education (Edu)",
        "Education Edition",
        "Official Chase The Clouds Education (Edu) panorama background.",
        "gui/panoramas/Education/chase_the_clouds_education/textures/ui/",
        "gui/panoramas/Education/chase_the_clouds_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "cloud_education",
        "Cloud Education (Edu)",
        "Education Edition",
        "Official Cloud Education (Edu) panorama background.",
        "gui/panoramas/Education/cloud_education/textures/ui/",
        "gui/panoramas/Education/cloud_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "copper_collaborate_complete_education",
        "Copper Collaborate Complete Education (Edu)",
        "Education Edition",
        "Official Copper Collaborate Complete Education (Edu) panorama background.",
        "gui/panoramas/Education/copper_collaborate_complete_education/textures/ui/",
        "gui/panoramas/Education/copper_collaborate_complete_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "goat_education",
        "Goat Education (Edu)",
        "Education Edition",
        "Official Goat Education (Edu) panorama background.",
        "gui/panoramas/Education/goat_education/textures/ui/",
        "gui/panoramas/Education/goat_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "learn_to_code_education",
        "Learn To Code Education (Edu)",
        "Education Edition",
        "Official Learn To Code Education (Edu) panorama background.",
        "gui/panoramas/Education/learn_to_code_education/textures/ui/",
        "gui/panoramas/Education/learn_to_code_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "mobile_multiplayer_more_education",
        "Mobile Multiplayer More Education (Edu)",
        "Education Edition",
        "Official Mobile Multiplayer More Education (Edu) panorama background.",
        "gui/panoramas/Education/mobile_multiplayer_more_education/textures/ui/",
        "gui/panoramas/Education/mobile_multiplayer_more_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "school_demo_education",
        "School Demo Education (Edu)",
        "Education Edition",
        "Official School Demo Education (Edu) panorama background.",
        "gui/panoramas/Education/school_demo_education/textures/ui/",
        "gui/panoramas/Education/school_demo_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "school_education",
        "School Education (Edu)",
        "Education Edition",
        "Official School Education (Edu) panorama background.",
        "gui/panoramas/Education/school_education/textures/ui/",
        "gui/panoramas/Education/school_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "trails_and_tales_education",
        "Trails And Tales Education (Edu)",
        "Education Edition",
        "Official Trails And Tales Education (Edu) panorama background.",
        "gui/panoramas/Education/trails_and_tales_education/textures/ui/",
        "gui/panoramas/Education/trails_and_tales_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "wild_education",
        "Wild Education (Edu)",
        "Education Edition",
        "Official Wild Education (Edu) panorama background.",
        "gui/panoramas/Education/wild_education/textures/ui/",
        "gui/panoramas/Education/wild_education/textures/ui/panorama_0.png"
    });

    list.push_back({
        "buzzy_bees",
        "Buzzy Bees",
        "Java & Bedrock",
        "Official Buzzy Bees panorama background.",
        "gui/panoramas/Java_and_Bedrock/buzzy_bees/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/buzzy_bees/textures/ui/panorama_0.png"
    });

    list.push_back({
        "caves",
        "Caves",
        "Java & Bedrock",
        "Official Caves panorama background.",
        "gui/panoramas/Java_and_Bedrock/caves/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/caves/textures/ui/panorama_0.png"
    });

    list.push_back({
        "cliffs",
        "Cliffs",
        "Java & Bedrock",
        "Official Cliffs panorama background.",
        "gui/panoramas/Java_and_Bedrock/cliffs/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/cliffs/textures/ui/panorama_0.png"
    });

    list.push_back({
        "copper_age",
        "Copper Age",
        "Java & Bedrock",
        "Official Copper Age panorama background.",
        "gui/panoramas/Java_and_Bedrock/copper_age/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/copper_age/textures/ui/panorama_0.png"
    });

    list.push_back({
        "garden_awakens",
        "Garden Awakens",
        "Java & Bedrock",
        "Official Garden Awakens panorama background.",
        "gui/panoramas/Java_and_Bedrock/garden_awakens/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/garden_awakens/textures/ui/panorama_0.png"
    });

    list.push_back({
        "mounts_of_mayhem",
        "Mounts Of Mayhem",
        "Java & Bedrock",
        "Official Mounts Of Mayhem panorama background.",
        "gui/panoramas/Java_and_Bedrock/mounts_of_mayhem/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/mounts_of_mayhem/textures/ui/panorama_0.png"
    });

    list.push_back({
        "nether",
        "Nether",
        "Java & Bedrock",
        "Official Nether panorama background.",
        "gui/panoramas/Java_and_Bedrock/nether/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/nether/textures/ui/panorama_0.png"
    });

    list.push_back({
        "spring_to_life",
        "Spring To Life",
        "Java & Bedrock",
        "Official Spring To Life panorama background.",
        "gui/panoramas/Java_and_Bedrock/spring_to_life/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/spring_to_life/textures/ui/panorama_0.png"
    });

    list.push_back({
        "trails_and_tales",
        "Trails And Tales",
        "Java & Bedrock",
        "Official Trails And Tales panorama background.",
        "gui/panoramas/Java_and_Bedrock/trails_and_tales/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/trails_and_tales/textures/ui/panorama_0.png"
    });

    list.push_back({
        "tricky_trials",
        "Tricky Trials",
        "Java & Bedrock",
        "Official Tricky Trials panorama background.",
        "gui/panoramas/Java_and_Bedrock/tricky_trials/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/tricky_trials/textures/ui/panorama_0.png"
    });

    list.push_back({
        "wild",
        "Wild",
        "Java & Bedrock",
        "Official Wild panorama background.",
        "gui/panoramas/Java_and_Bedrock/wild/textures/ui/",
        "gui/panoramas/Java_and_Bedrock/wild/textures/ui/panorama_0.png"
    });

    list.push_back({
        "aquatic_java",
        "Aquatic Java (Java)",
        "Java Edition",
        "Official Aquatic Java (Java) panorama background.",
        "gui/panoramas/Java/aquatic_java/textures/ui/",
        "gui/panoramas/Java/aquatic_java/textures/ui/panorama_0.png"
    });

    list.push_back({
        "caves_java",
        "Caves Java (Java)",
        "Java Edition",
        "Official Caves Java (Java) panorama background.",
        "gui/panoramas/Java/caves_java/textures/ui/",
        "gui/panoramas/Java/caves_java/textures/ui/panorama_0.png"
    });

    list.push_back({
        "chase_the_skies_java",
        "Chase The Skies Java (Java)",
        "Java Edition",
        "Official Chase The Skies Java (Java) panorama background.",
        "gui/panoramas/Java/chase_the_skies_java/textures/ui/",
        "gui/panoramas/Java/chase_the_skies_java/textures/ui/panorama_0.png"
    });

    list.push_back({
        "classic",
        "Classic (Java)",
        "Java Edition",
        "Official Classic (Java) panorama background.",
        "gui/panoramas/Java/classic/textures/ui/",
        "gui/panoramas/Java/classic/textures/ui/panorama_0.png"
    });

    list.push_back({
        "classic_blurred",
        "Classic Blurred (Java)",
        "Java Edition",
        "Official Classic Blurred (Java) panorama background.",
        "gui/panoramas/Java/classic_blurred/textures/ui/",
        "gui/panoramas/Java/classic_blurred/textures/ui/panorama_0.png"
    });

    list.push_back({
        "indev",
        "Indev (Java)",
        "Java Edition",
        "Official Indev (Java) panorama background.",
        "gui/panoramas/Java/indev/textures/ui/",
        "gui/panoramas/Java/indev/textures/ui/panorama_0.png"
    });

    list.push_back({
        "tiny_takeover_java",
        "Tiny Takeover Java (Java)",
        "Java Edition",
        "Official Tiny Takeover Java (Java) panorama background.",
        "gui/panoramas/Java/tiny_takeover_java/textures/ui/",
        "gui/panoramas/Java/tiny_takeover_java/textures/ui/panorama_0.png"
    });

    list.push_back({
        "village_and_pillage_java",
        "Village And Pillage Java (Java)",
        "Java Edition",
        "Official Village And Pillage Java (Java) panorama background.",
        "gui/panoramas/Java/village_and_pillage_java/textures/ui/",
        "gui/panoramas/Java/village_and_pillage_java/textures/ui/panorama_0.png"
    });

    list.push_back({
        "nether_nintendo_3ds",
        "Nether Nintendo 3Ds",
        "Others",
        "Official Nether Nintendo 3Ds panorama background.",
        "gui/panoramas/Others/nether_nintendo_3ds/textures/ui/",
        "gui/panoramas/Others/nether_nintendo_3ds/textures/ui/panorama_0.png"
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

