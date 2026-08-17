#include "ModRegistry.h"

#include "../world/item/Item.h"
#include "../world/item/ItemCategory.h"
#include "../world/level/material/Material.h"
#include "../world/level/tile/Tile.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

Tile* ModRegistry::exampleBlock = NULL;
Item* ModRegistry::exampleItem = NULL;

static bool pmcDirExists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && (st.st_mode & S_IFDIR);
}

static void pmcMakeDir(const std::string& path)
{
    if (path.empty() || pmcDirExists(path)) return;
#ifdef _WIN32
    _mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0755);
#endif
}

static void pmcMakeDirs(const std::string& path)
{
    std::string cur;
    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        cur += c;
        if (c == '/' || c == '\\') {
            if (cur.size() > 1) pmcMakeDir(cur);
        }
    }
    pmcMakeDir(path);
}

static std::string pmcJoin(const std::string& a, const std::string& b)
{
    if (a.empty() || a == ".") return b;
    char last = a[a.size() - 1];
    if (last == '/' || last == '\\') return a + b;
    return a + "/" + b;
}

static std::map<std::string, std::string> pmcReadManifest(const std::string& path)
{
    std::map<std::string, std::string> values;
    std::ifstream f(path.c_str());
    std::string line;
    while (std::getline(f, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        values[line.substr(0, eq)] = line.substr(eq + 1);
    }
    return values;
}

std::string ModRegistry::modsRoot(const std::string& storageRoot)
{
    return pmcJoin(storageRoot, "games/com.mojang/mods");
}

void ModRegistry::ensureFolders(const std::string& storageRoot)
{
    const std::string root = modsRoot(storageRoot);
    const std::string example = pmcJoin(root, "example_custom_content");
    pmcMakeDirs(pmcJoin(example, "resource_packs/textures/blocks"));
    pmcMakeDirs(pmcJoin(example, "resource_packs/textures/items"));
    pmcMakeDirs(pmcJoin(example, "resource_packs/models/blocks"));
    pmcMakeDirs(pmcJoin(example, "resource_packs/sounds"));
    pmcMakeDirs(pmcJoin(example, "resource_packs/animations"));
    pmcMakeDirs(pmcJoin(example, "behevior_packs/cpp"));
    pmcMakeDirs(pmcJoin(example, "behevior_packs/entities"));
    pmcMakeDirs(pmcJoin(example, "behevior_packs/blocks"));
    pmcMakeDirs(pmcJoin(example, "behevior_packs/items"));

    const std::string manifest = pmcJoin(example, "manifest.txt");
    std::ifstream test(manifest.c_str());
    if (!test.good()) {
        std::ofstream out(manifest.c_str());
        out << "id=example_custom_content\n";
        out << "name=Example Custom Content\n";
        out << "version=1.0.0\n";
        out << "author=PocketMC\n";
        out << "description=Example C++ mod that registers a custom block and item.\n";
    }

    const std::string cppNote = pmcJoin(example, "behevior_packs/cpp/example_custom_content.cpp.txt");
    std::ifstream cppTest(cppNote.c_str());
    if (!cppTest.good()) {
        std::ofstream out(cppNote.c_str());
        out << "// Example mod logic is compiled in src/mods/ModRegistry.cpp.\n";
        out << "// Future external C++ mods can mirror this registration API.\n";
    }
}

std::vector<ModInfo> ModRegistry::scanMods(const std::string& storageRoot)
{
    ensureFolders(storageRoot);

    std::vector<ModInfo> mods;
    ModInfo builtIn;
    builtIn.id = "builtin.example_custom_content";
    builtIn.name = "Example Custom Content";
    builtIn.version = "1.0.0";
    builtIn.author = "PocketMC";
    builtIn.description = "Built-in C++ example: adds a custom block and item.";
    builtIn.path = "compiled";
    builtIn.hasResourcePack = true;
    builtIn.hasBeheviorPack = true;
    builtIn.builtIn = true;
    mods.push_back(builtIn);

    const std::string root = modsRoot(storageRoot);

#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA((root + "/*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            std::string name = fd.cFileName;
            if (name == "." || name == "..") continue;
            std::string path = pmcJoin(root, name);
#else
    DIR* dir = opendir(root.c_str());
    if (dir) {
        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            std::string name = ent->d_name;
            if (name == "." || name == "..") continue;
            std::string path = pmcJoin(root, name);
            if (!pmcDirExists(path)) continue;
#endif
            std::map<std::string, std::string> manifest = pmcReadManifest(pmcJoin(path, "manifest.txt"));
            ModInfo info;
            info.id = manifest.count("id") ? manifest["id"] : name;
            info.name = manifest.count("name") ? manifest["name"] : name;
            info.version = manifest.count("version") ? manifest["version"] : "";
            info.author = manifest.count("author") ? manifest["author"] : "";
            info.description = manifest.count("description") ? manifest["description"] : "No description.";
            info.path = path;
            info.hasResourcePack = pmcDirExists(pmcJoin(path, "resource_packs"));
            info.hasBeheviorPack = pmcDirExists(pmcJoin(path, "behevior_packs"));
            mods.push_back(info);
#ifdef _WIN32
        } while (FindNextFileA(h, &fd));
        FindClose(h);
#else
        }
        closedir(dir);
#endif
    }

    return mods;
}

void ModRegistry::registerBuiltInTiles()
{
    if (exampleBlock || Tile::tiles[ExampleBlockId]) return;
    exampleBlock = (new Tile(ExampleBlockId, 1 + 14 * 16, Material::stone))->init()
        ->setDestroyTime(1.0f)
        ->setExplodeable(6.0f)
        ->setSoundType(Tile::SOUND_STONE)
        ->setCategory(ItemCategory::Decorations)
        ->setDescriptionId("exampleCustomBlock");
}

void ModRegistry::registerBuiltInItems()
{
    if (exampleItem || Item::items[256 + ExampleItemId]) return;
    exampleItem = (new Item(ExampleItemId))->setIcon(6, 10)
        ->setCategory(ItemCategory::Decorations)
        ->setDescriptionId("exampleCustomItem");
}
