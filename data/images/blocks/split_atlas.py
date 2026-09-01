#!/usr/bin/env python3
"""
Script para dividir los atlas terrain.png y terrain2.png en texturas individuales
basado en el reporte de mapeo TEXTURE_MAPPING_REPORT.md
"""

import re
from PIL import Image
import os

# Rutas
BLOCKS_DIR = r"C:\Users\Usuario\Desktop\Proyectos\PocketMC\data\images\blocks"
REPORT_FILE = os.path.join(BLOCKS_DIR, "TEXTURE_MAPPING_REPORT.md")
TERRAIN_PNG = os.path.join(BLOCKS_DIR, "terrain_original.png")
TERRAIN2_PNG = os.path.join(BLOCKS_DIR, "terrain2_original.png")

# Tamaño de cada celda en el atlas (16x16 píxeles)
CELL_SIZE = 16

# Diccionario para mapear nombres descriptivos a nombres de archivo
TEXTURE_MAPPING = {
    # Bloques Simples (UNIFORME)
    "stone.png": {"index": 1, "atlas": "terrain.png"},
    "dirt.png": {"index": 2, "atlas": "terrain.png"},
    "stone_brick.png": {"index": 16, "atlas": "terrain.png"},
    "planks_oak.png": {"index": 4, "atlas": "terrain.png"},
    "sand.png": {"index": 18, "atlas": "terrain.png"},
    "gravel.png": {"index": 19, "atlas": "terrain.png"},
    "gold_ore.png": {"index": 32, "atlas": "terrain.png"},
    "iron_ore.png": {"index": 33, "atlas": "terrain.png"},
    "coal_ore.png": {"index": 34, "atlas": "terrain.png"},
    "glass.png": {"index": 49, "atlas": "terrain.png"},
    "lapis_ore.png": {"index": 160, "atlas": "terrain.png"},
    "lapis_block.png": {"index": 144, "atlas": "terrain.png"},
    "mossy_cobblestone.png": {"index": 36, "atlas": "terrain.png"},
    "obsidian.png": {"index": 37, "atlas": "terrain.png"},
    "brick.png": {"index": 7, "atlas": "terrain.png"},
    "tnt.png": {"index": 8, "atlas": "terrain.png"},
    "gold_block.png": {"index": 23, "atlas": "terrain.png"},
    "iron_block.png": {"index": 22, "atlas": "terrain.png"},
    "diamond_ore.png": {"index": 50, "atlas": "terrain.png"},
    "diamond_block.png": {"index": 24, "atlas": "terrain.png"},
    "netherrack.png": {"index": 103, "atlas": "terrain.png"},
    "glowstone.png": {"index": 105, "atlas": "terrain.png"},
    "nether_brick.png": {"index": 224, "atlas": "terrain.png"},
    "nether_quartz_ore.png": {"index": 23, "atlas": "terrain2.png"},
    "end_stone.png": {"index": 24, "atlas": "terrain2.png"},
    "soul_sand.png": {"index": 25, "atlas": "terrain2.png"},
    "glowing_obsidian.png": {"index": 218, "atlas": "terrain.png"},
    "planks_spruce.png": {"index": 4, "atlas": "terrain2.png"},
    "planks_birch.png": {"index": 5, "atlas": "terrain2.png"},
    "deepslate.png": {"index": 6, "atlas": "terrain2.png"},
    "cobbled_deepslate.png": {"index": 7, "atlas": "terrain2.png"},
    "deepslate_coal_ore.png": {"index": 8, "atlas": "terrain2.png"},
    "deepslate_diamond_ore.png": {"index": 9, "atlas": "terrain2.png"},
    "deepslate_gold_ore.png": {"index": 10, "atlas": "terrain2.png"},
    "deepslate_iron_ore.png": {"index": 11, "atlas": "terrain2.png"},
    "deepslate_lapis_ore.png": {"index": 12, "atlas": "terrain2.png"},
    "deepslate_redstone_ore.png": {"index": 13, "atlas": "terrain2.png"},
    "deepslate_redstone_ore_lit.png": {"index": 13, "atlas": "terrain2.png"},
    "polished_deepslate.png": {"index": 18, "atlas": "terrain2.png"},
    "deepslate_tiles.png": {"index": 19, "atlas": "terrain2.png"},
    "deepslate_bricks.png": {"index": 20, "atlas": "terrain2.png"},
    "cobweb.png": {"index": 11, "atlas": "terrain.png"},
    "cactus_side.png": {"index": 70, "atlas": "terrain.png"},
    "clay.png": {"index": 72, "atlas": "terrain.png"},
    "sugar_cane.png": {"index": 73, "atlas": "terrain.png"},
    "ice.png": {"index": 67, "atlas": "terrain.png"},
    "snow.png": {"index": 66, "atlas": "terrain.png"},
    "snow_layer.png": {"index": 66, "atlas": "terrain.png"},
    "redstone_ore.png": {"index": 51, "atlas": "terrain.png"},
    "redstone_ore_lit.png": {"index": 51, "atlas": "terrain.png"},
    "sapling.png": {"index": 15, "atlas": "terrain.png"},
    "tall_grass.png": {"index": 39, "atlas": "terrain.png"},
    "mushroom_red.png": {"index": 29, "atlas": "terrain.png"},
    "mushroom_brown.png": {"index": 28, "atlas": "terrain.png"},
    "flower_dandelion.png": {"index": 13, "atlas": "terrain.png"},
    "flower_cornflower.png": {"index": 12, "atlas": "terrain.png"},
    "fire.png": {"index": 31, "atlas": "terrain.png"},
    "portal.png": {"index": 250, "atlas": "terrain.png"},
    "glass_pane.png": {"index": 49, "atlas": "terrain.png"},
    "torch.png": {"index": 80, "atlas": "terrain.png"},
    "ladder.png": {"index": 83, "atlas": "terrain.png"},
    "fence_oak.png": {"index": 4, "atlas": "terrain.png"},
    "fence_birch.png": {"index": 5, "atlas": "terrain2.png"},
    "fence_spruce.png": {"index": 4, "atlas": "terrain2.png"},
    "wheat.png": {"index": 88, "atlas": "terrain.png"},
    
    # Bloques Simplificados (side, top, bottom)
    "grass_top.png": {"index": 0, "atlas": "terrain.png"},
    "grass_side.png": {"index": 3, "atlas": "terrain.png"},
    "grass_side_snow.png": {"index": 68, "atlas": "terrain.png"},
    "log_oak_top.png": {"index": 21, "atlas": "terrain.png"},
    "log_oak_side.png": {"index": 20, "atlas": "terrain.png"},
    "log_birch_top.png": {"index": 3, "atlas": "terrain2.png"},
    "log_birch_side.png": {"index": 2, "atlas": "terrain2.png"},
    "log_spruce_top.png": {"index": 1, "atlas": "terrain2.png"},
    "log_spruce_side.png": {"index": 0, "atlas": "terrain2.png"},
    "bookshelf_side.png": {"index": 35, "atlas": "terrain.png"},
    "melon_top.png": {"index": 137, "atlas": "terrain.png"},
    "melon_side.png": {"index": 136, "atlas": "terrain.png"},
    
    # Bloques Específicos (6 caras)
    "furnace_top.png": {"index": 62, "atlas": "terrain.png"},
    "furnace_bottom.png": {"index": 62, "atlas": "terrain.png"},
    "furnace_front.png": {"index": 44, "atlas": "terrain.png"},
    "furnace_front_lit.png": {"index": 61, "atlas": "terrain.png"},
    "furnace_side.png": {"index": 45, "atlas": "terrain.png"},
    "chest_top.png": {"index": 25, "atlas": "terrain.png"},
    "chest_front.png": {"index": 27, "atlas": "terrain.png"},
    "chest_side.png": {"index": 26, "atlas": "terrain.png"},
    "crafting_table_top.png": {"index": 43, "atlas": "terrain.png"},
    "crafting_table_front.png": {"index": 60, "atlas": "terrain.png"},
    "crafting_table_side.png": {"index": 60, "atlas": "terrain.png"},
    "crafting_table_back.png": {"index": 59, "atlas": "terrain.png"},
    "stonecutter_top.png": {"index": 169, "atlas": "terrain.png"},
    "stonecutter_bottom.png": {"index": 62, "atlas": "terrain.png"},
    "stonecutter_front.png": {"index": 168, "atlas": "terrain.png"},
    "stonecutter_side.png": {"index": 45, "atlas": "terrain.png"},
    "sandstone_top.png": {"index": 176, "atlas": "terrain.png"},
    "sandstone_bottom.png": {"index": 208, "atlas": "terrain.png"},
    "sandstone_normal.png": {"index": 192, "atlas": "terrain.png"},
    "sandstone_chiseled.png": {"index": 229, "atlas": "terrain.png"},
    "sandstone_smooth.png": {"index": 230, "atlas": "terrain.png"},
    "stonebrick_normal.png": {"index": 54, "atlas": "terrain.png"},
    "stonebrick_mossy.png": {"index": 100, "atlas": "terrain.png"},
    "stonebrick_cracked.png": {"index": 101, "atlas": "terrain.png"},
    "quartz_top.png": {"index": 196, "atlas": "terrain.png"},
    "quartz_bottom.png": {"index": 211, "atlas": "terrain.png"},
    "quartz_side.png": {"index": 212, "atlas": "terrain.png"},
    "quartz_chiseled_top.png": {"index": 198, "atlas": "terrain.png"},
    "quartz_chiseled.png": {"index": 214, "atlas": "terrain.png"},
    "quartz_lines_top.png": {"index": 197, "atlas": "terrain.png"},
    "quartz_lines.png": {"index": 213, "atlas": "terrain.png"},
    "nether_reactor_core.png": {"index": 234, "atlas": "terrain.png"},
    "nether_reactor_active.png": {"index": 234, "atlas": "terrain.png"},
    "nether_reactor_used.png": {"index": 234, "atlas": "terrain.png"},
    "door_wood_top_bottom.png": {"index": 97, "atlas": "terrain.png"},
    "door_wood_upper.png": {"index": 81, "atlas": "terrain.png"},
    "door_wood_lower.png": {"index": 97, "atlas": "terrain.png"},
    "door_iron_top_bottom.png": {"index": 98, "atlas": "terrain.png"},
    "door_iron_upper.png": {"index": 82, "atlas": "terrain.png"},
    "door_iron_lower.png": {"index": 98, "atlas": "terrain.png"},
    
    # Bloques con Variantes
    "leaves_oak.png": {"index": 52, "atlas": "terrain.png"},
    "leaves_spruce.png": {"index": 132, "atlas": "terrain.png"},
    "leaves_birch.png": {"index": 52, "atlas": "terrain.png"},
    "leaves_carried.png": {"index": 235, "atlas": "terrain.png"},
}

def index_to_coords(index):
    """Convierte un índice de atlas a coordenadas (x, y) en píxeles"""
    column = index % 16
    row = index // 16
    x = column * CELL_SIZE
    y = row * CELL_SIZE
    return x, y

def extract_texture(atlas_image, index, output_path):
    """Extrae una textura del atlas y la guarda"""
    x, y = index_to_coords(index)
    
    # Extraer la región de 16x16 píxeles
    texture = atlas_image.crop((x, y, x + CELL_SIZE, y + CELL_SIZE))
    
    # Guardar la textura
    texture.save(output_path)
    print(f"[OK] Extraido: {output_path} (indice {index}, coords {x},{y})")

def main():
    print("=== División de Atlas de Texturas PocketMC ===\n")
    
    # Verificar que los archivos existan
    if not os.path.exists(TERRAIN_PNG):
        print(f"[ERROR] No se encuentra {TERRAIN_PNG}")
        return
    
    if not os.path.exists(TERRAIN2_PNG):
        print(f"[ERROR] No se encuentra {TERRAIN2_PNG}")
        return
    
    # Cargar los atlas
    print("Cargando atlas...")
    terrain_atlas = Image.open(TERRAIN_PNG)
    terrain2_atlas = Image.open(TERRAIN2_PNG)
    
    print(f"[OK] terrain.png: {terrain_atlas.size}")
    print(f"[OK] terrain2.png: {terrain2_atlas.size}\n")
    
    # Crear directorio de salida si no existe
    output_dir = os.path.join(BLOCKS_DIR, "textures")
    os.makedirs(output_dir, exist_ok=True)
    
    # Extraer texturas
    print("Extrayendo texturas...\n")
    
    count = 0
    for filename, info in TEXTURE_MAPPING.items():
        index = info["index"]
        atlas_name = info["atlas"]
        
        # Seleccionar el atlas correcto
        if atlas_name == "terrain.png":
            atlas = terrain_atlas
        elif atlas_name == "terrain2.png":
            atlas = terrain2_atlas
        else:
            print(f"[WARN] Atlas desconocido '{atlas_name}' para {filename}")
            continue
        
        # Ruta de salida
        output_path = os.path.join(output_dir, filename)
        
        # Extraer y guardar
        try:
            extract_texture(atlas, index, output_path)
            count += 1
        except Exception as e:
            print(f"[ERROR] Error extrayendo {filename}: {e}")
    
    print(f"\n=== Completado ===")
    print(f"Total de texturas extraídas: {count}")
    print(f"Directorio de salida: {output_dir}")

if __name__ == "__main__":
    main()
