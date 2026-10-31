#!/usr/bin/env python3
import os
import shutil
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("[-] Error: Pillow is not installed.\n    Run: pip install pillow (or 'pkg install python-pillow' in Termux)")

# ----------------- PATH CONFIGURATION -----------------
PROJECT_ROOT = os.path.abspath(os.path.dirname(__file__))
DATA_DIR = os.path.join(PROJECT_ROOT, "data")
MOD_ASSETS = os.path.join(PROJECT_ROOT, "src", "mods", "woodland", "assets")
ATLAS_HEADER = os.path.join(PROJECT_ROOT, "src", "mods", "woodland", "WoodlandAtlas.h")

TERRAIN_PNG = os.path.join(DATA_DIR, "images", "terrain.png")

# Locate items.png (in gui/ or images/)
ITEMS_PNG = os.path.join(DATA_DIR, "images", "gui", "items.png")
if not os.path.exists(ITEMS_PNG):
    ITEMS_PNG = os.path.join(DATA_DIR, "images", "items.png")

BLOCKS_OUT_DIR = os.path.join(DATA_DIR, "images", "blocks")
ENTITY_OUT_DIR = os.path.join(DATA_DIR, "images", "mods", "woodland", "textures", "entity")


def find_asset(filename, search_dir):
    """Recursively search for a file by name."""
    for root, _, files in os.walk(search_dir):
        for f in files:
            if f.lower() == filename.lower():
                return os.path.join(root, f)
    return None


def paste_tile(atlas, tile_path, col, row):
    """Resizes tile to atlas cell size and pastes at (col, row)."""
    cell_w = atlas.width // 16
    cell_h = atlas.height // 16
    tile = Image.open(tile_path).convert("RGBA")
    # In case of animated strips, take the first square frame
    tile = tile.crop((0, 0, tile.width, tile.width))
    tile = tile.resize((cell_w, cell_h), Image.NEAREST)
    atlas.paste(tile, (col * cell_w, row * cell_h), tile)


def find_free_item_slots(items_img, needed=3):
    """Scans for fully transparent cells starting from the bottom-left of items.png."""
    cell_w = items_img.width // 16
    cell_h = items_img.height // 16
    slots = []
    
    # Check bottom row first (row 15), then upwards
    for r in range(15, -1, -1):
        for c in range(16):
            box = (c * cell_w, r * cell_h, (c + 1) * cell_w, (r + 1) * cell_h)
            cell = items_img.crop(box)
            extrema = cell.getchannel("A").getextrema()
            if extrema[1] == 0:  # Fully transparent slot
                slots.append((c, r))
                if len(slots) == needed:
                    return slots
    return slots


def main():
    print("[*] Starting Woodland Asset Setup...")

    if not os.path.exists(MOD_ASSETS):
        sys.exit(f"[-] Could not find mod assets at: {MOD_ASSETS}")
    if not os.path.exists(TERRAIN_PNG):
        sys.exit(f"[-] Could not find terrain.png at: {TERRAIN_PNG}")
    if not os.path.exists(ITEMS_PNG):
        sys.exit(f"[-] Could not find items.png at: {ITEMS_PNG}")

    os.makedirs(BLOCKS_OUT_DIR, exist_ok=True)
    os.makedirs(ENTITY_OUT_DIR, exist_ok=True)

    # 1. MOB ENTITY TEXTURES
    entity_src_dir = os.path.join(MOD_ASSETS, "textures", "entity")
    if os.path.exists(entity_src_dir):
        copied_mobs = 0
        for f in os.listdir(entity_src_dir):
            if f.endswith(".png"):
                shutil.copy(os.path.join(entity_src_dir, f), os.path.join(ENTITY_OUT_DIR, f))
                copied_mobs += 1
        print(f"[+] Copied {copied_mobs} mob entity textures -> data/images/mods/woodland/textures/entity/")

    # 2. TERRAIN ATLAS (BLOCKS)
    print(f"[*] Patching {TERRAIN_PNG}...")
    terrain = Image.open(TERRAIN_PNG).convert("RGBA")

    block_files = [
        ("wood1.png", 7, 10),           # Index 167
        ("wood2.png", 8, 10),           # Index 168
        ("wood3.png", 9, 10),           # Index 169
        ("wood_key_block.png", 10, 10),  # Index 170
        ("wood_door.png", 11, 10),       # Index 171
        ("wood_portal.png", 12, 10),     # Index 172
    ]

    for filename, col, row in block_files:
        path = find_asset(filename, MOD_ASSETS)
        if not path:
            # Fallback if door/portal reuse other texture names
            if filename == "wood_door.png": path = find_asset("wood2.png", MOD_ASSETS)
            elif filename == "wood_portal.png": path = find_asset("wood1.png", MOD_ASSETS)

        if path:
            paste_tile(terrain, path, col, row)
            # Also copy to data/images/blocks/
            shutil.copy(path, os.path.join(BLOCKS_OUT_DIR, filename))
            print(f"    - Block '{filename}' placed at (Col {col}, Row {row}) [Index {col + row * 16}]")
        else:
            print(f"    ! Warning: Texture '{filename}' not found in assets")

    terrain.save(TERRAIN_PNG)
    print(f"[+] Saved updated {TERRAIN_PNG}")

    # 3. ITEMS ATLAS
    print(f"[*] Patching {ITEMS_PNG}...")
    items = Image.open(ITEMS_PNG).convert("RGBA")
    free_slots = find_free_item_slots(items, needed=3)

    if len(free_slots) < 3:
        # Fallback to (0, 15), (1, 15), (2, 15) if auto-detection fails
        free_slots = [(0, 15), (1, 15), (2, 15)]

    item_textures = [
        ("KEY", "wood_key.png", free_slots[0]),
        ("MAP", "door_map.png", free_slots[1]),
        ("EGG", "wooden_spawn_egg.png", free_slots[2]),
    ]

    for code, filename, (col, row) in item_textures:
        path = find_asset(filename, MOD_ASSETS)
        if not path and filename == "door_map.png":
            path = find_asset("wood_key.png", MOD_ASSETS)

        if path:
            paste_tile(items, path, col, row)
            print(f"    - Item '{filename}' placed at (Col {col}, Row {row})")
        else:
            print(f"    ! Warning: Texture '{filename}' not found in assets")

    items.save(ITEMS_PNG)
    print(f"[+] Saved updated {ITEMS_PNG}")

    # 4. WRITE WoodlandAtlas.h
    key_col, key_row = free_slots[0]
    map_col, map_row = free_slots[1]
    egg_col, egg_row = free_slots[2]

    header_content = f"""#ifndef POCKETMC_MODS_WOODLAND_ATLAS_H__
#define POCKETMC_MODS_WOODLAND_ATLAS_H__

// Automatically generated by setup_woodland_assets.py

// Block texture indices in terrain.png (Column + Row * 16)
#define WL_TEX_WOOD1       (7 + 10 * 16)  // 167
#define WL_TEX_WOOD2       (8 + 10 * 16)  // 168
#define WL_TEX_WOOD3       (9 + 10 * 16)  // 169
#define WL_TEX_KEY_BLOCK   (10 + 10 * 16) // 170
#define WL_TEX_DOOR        (11 + 10 * 16) // 171
#define WL_TEX_PORTAL      (12 + 10 * 16) // 172

// Item icon coordinates in items.png (Column X, Row Y)
#define WL_ICON_KEY_X      {key_col}
#define WL_ICON_KEY_Y      {key_row}

#define WL_ICON_MAP_X      {map_col}
#define WL_ICON_MAP_Y      {map_row}

#define WL_ICON_EGG_X      {egg_col}
#define WL_ICON_EGG_Y      {egg_row}

#endif
"""
    with open(ATLAS_HEADER, "w") as h:
        h.write(header_content)
    print(f"[+] Updated {ATLAS_HEADER}")

    # 5. LANGUAGE MERGE
    lang_src = find_asset("en_US_woodland.lang", MOD_ASSETS)
    # Search for en_US.lang in data/
    lang_dst = None
    for root, _, files in os.walk(DATA_DIR):
        for f in files:
            if f == "en_US.lang":
                lang_dst = os.path.join(root, f)
                break
        if lang_dst:
            break

    if lang_src and lang_dst:
        with open(lang_src, "r", encoding="utf-8", errors="ignore") as f:
            woodland_lines = f.read()

        with open(lang_dst, "r", encoding="utf-8", errors="ignore") as f:
            target_content = f.read()

        if "entity.woodland.wooden.name" not in target_content:
            with open(lang_dst, "a", encoding="utf-8") as f:
                f.write("\n\n# Woodland Mod Translations\n" + woodland_lines)
            print(f"[+] Appended Woodland translations to {lang_dst}")
        else:
            print(f"[*] Translations already present in {lang_dst}")

    print("\n[✓] Woodland asset configuration complete!")


if __name__ == "__main__":
    main()