# Reporte Completo de Mapeo de Texturas - PocketMC

## Resumen Ejecutivo

Este reporte contiene el análisis exhaustivo de todos los bloques definidos en PocketMC, sus índices de textura en el atlas, posiciones en terrain.png/terrain2.png, y recomendaciones para el sistema material_instances.

## Sistema de Coordenadas del Atlas

**Cálculo de posición:**
- **Columna** = índice % 16 (0-15)
- **Fila** = índice / 16 (0-15)
- **Atlas principal**: terrain.png (sin flag TEXTURE_ALT_FLAG)
- **Atlas secundario**: terrain2.png (con flag TEXTURE_ALT_FLAG = 0x1000)

**Código de referencia (TileRenderer.cpp línea 2174-2175):**
```cpp
int xt = (tex & 0xf) << 4;  // Columna * 16
int yt = tex & 0xf0;        // Fila * 16
```

## Tabla Resumen de Bloques

| ID | Nombre | Tipo | Formato Recomendado | Atlas |
|----|--------|------|---------------------|-------|
| 1 | rock (StoneTile) | Simple | UNIFORME | terrain.png |
| 2 | grass (GrassTile) | Especial | SIMPLIFICADO | terrain.png |
| 3 | dirt (DirtTile) | Simple | UNIFORME | terrain.png |
| 4 | stoneBrick | Simple | UNIFORME | terrain.png |
| 5 | wood (planks) | Simple | UNIFORME | terrain.png |
| 6 | sapling | Planta | UNIFORME | terrain.png |
| 7 | unbreakable (bedrock) | Simple | UNIFORME | terrain.png |
| 8 | water (dinámico) | Líquido | UNIFORME | N/A |
| 9 | calmWater (estático) | Líquido | UNIFORME | N/A |
| 10 | lava (dinámico) | Líquido | UNIFORME | N/A |
| 11 | calmLava (estático) | Líquido | UNIFORME | N/A |
| 12 | sand (HeavyTile) | Simple | UNIFORME | terrain.png |
| 13 | gravel (GravelTile) | Simple | UNIFORME | terrain.png |
| 14 | goldOre | Simple | UNIFORME | terrain.png |
| 15 | ironOre | Simple | UNIFORME | terrain.png |
| 16 | coalOre | Simple | UNIFORME | terrain.png |
| 17 | treeTrunk (TreeTile) | Especial | SIMPLIFICADO | terrain.png |
| 18 | leaves (LeafTile) | Especial | UNIFORME | terrain.png |
| 19 | birchTrunk (TreeTile) | Especial | SIMPLIFICADO | terrain2.png |
| 20 | glass (GlassTile) | Simple | UNIFORME | terrain.png |
| 21 | lapisOre | Simple | UNIFORME | terrain.png |
| 22 | lapisBlock | Simple | UNIFORME | terrain.png |
| 23 | spruceTrunk (TreeTile) | Especial | SIMPLIFICADO | terrain2.png |
| 24 | sandStone (SandStoneTile) | Multi-textura | ESPECÍFICO | terrain.png |
| 26 | bed (BedTile) | Direccional | ESPECÍFICO | terrain.png |
| 30 | web (WebTile) | Simple | UNIFORME | terrain.png |
| 31 | tallgrass | Planta | UNIFORME | terrain.png |
| 35 | cloth (ClothTile) | Multi-textura | UNIFORME | terrain.png |
| 39 | mushroom1 | Planta | UNIFORME | terrain.png |
| 40 | mushroom2 | Planta | UNIFORME | terrain.png |
| 41 | goldBlock (MetalTile) | Simple | UNIFORME | terrain.png |
| 42 | ironBlock (MetalTile) | Simple | UNIFORME | terrain.png |
| 43 | stoneSlab | Slab | UNIFORME | terrain.png |
| 44 | stoneSlabHalf | Slab | UNIFORME | terrain.png |
| 45 | redBrick | Simple | UNIFORME | terrain.png |
| 46 | tnt (TntTile) | Simple | UNIFORME | terrain.png |
| 47 | bookshelf (BookshelfTile) | Especial | SIMPLIFICADO | terrain.png |
| 48 | mossStone | Simple | UNIFORME | terrain.png |
| 49 | obsidian (ObsidianTile) | Simple | UNIFORME | terrain.png |
| 50 | torch (TorchTile) | Direccional | UNIFORME | terrain.png |
| 51 | fire (FireTile) | Animado | UNIFORME | terrain.png |
| 53 | stairs_wood | Escalera | UNIFORME | terrain.png |
| 54 | chest (ChestTile) | Direccional | ESPECÍFICO | terrain.png |
| 56 | emeraldOre | Simple | UNIFORME | terrain.png |
| 57 | emeraldBlock (MetalTile) | Simple | UNIFORME | terrain.png |
| 58 | workbench (WorkbenchTile) | Especial | ESPECÍFICO | terrain.png |
| 59 | crops (CropTile) | Planta | UNIFORME | terrain.png |
| 60 | farmland (FarmTile) | Simple | UNIFORME | terrain.png |
| 61 | furnace (FurnaceTile) | Direccional | ESPECÍFICO | terrain.png |
| 62 | furnace_lit (FurnaceTile) | Direccional | ESPECÍFICO | terrain.png |
| 63 | sign (SignTile) | Direccional | ESPECÍFICO | terrain.png |
| 64 | door_wood (DoorTile) | Direccional | ESPECÍFICO | terrain.png |
| 65 | ladder (LadderTile) | Direccional | UNIFORME | terrain.png |
| 67 | stairs_stone | Escalera | UNIFORME | terrain.png |
| 68 | wallSign (SignTile) | Direccional | ESPECÍFICO | terrain.png |
| 71 | door_iron (DoorTile) | Direccional | ESPECÍFICO | terrain.png |
| 73 | redStoneOre | Simple | UNIFORME | terrain.png |
| 74 | redStoneOre_lit | Simple | UNIFORME | terrain.png |
| 78 | topSnow (TopSnowTile) | Simple | UNIFORME | terrain.png |
| 79 | ice (IceTile) | Simple | UNIFORME | terrain.png |
| 80 | snow (SnowTile) | Simple | UNIFORME | terrain.png |
| 81 | cactus (CactusTile) | Simple | UNIFORME | terrain.png |
| 82 | clay (ClayTile) | Simple | UNIFORME | terrain.png |
| 83 | reeds (ReedTile) | Planta | UNIFORME | terrain.png |
| 85 | fence (FenceTile) | Valla | UNIFORME | terrain.png |
| 87 | netherrack | Simple | UNIFORME | terrain.png |
| 89 | lightGem (LightGemTile) | Simple | UNIFORME | terrain.png |
| 90 | netherPortal (NetherPortalTile) | Portal | UNIFORME | terrain.png |
| 95 | invisible_bedrock | Invisible | N/A | N/A |
| 96 | trapdoor (TrapDoorTile) | Direccional | ESPECÍFICO | terrain.png |
| 98 | stoneBrickSmooth (MultiTextureTile) | Multi-textura | ESPECÍFICO | terrain.png |
| 102 | thinGlass (ThinFenceTile) | Valla | UNIFORME | terrain.png |
| 103 | melon (MelonTile) | Especial | SIMPLIFICADO | terrain.png |
| 105 | melonStem (StemTile) | Planta | UNIFORME | terrain.png |
| 107 | fenceGate (FenceGateTile) | Direccional | ESPECÍFICO | terrain.png |
| 108 | stairs_brick | Escalera | UNIFORME | terrain.png |
| 109 | stairs_stoneBrickSmooth | Escalera | UNIFORME | terrain.png |
| 112 | netherBrick | Simple | UNIFORME | terrain.png |
| 114 | stairs_netherBricks | Escalera | UNIFORME | terrain.png |
| 117 | netherQuartzOre | Simple | UNIFORME | terrain2.png |
| 118 | endStone | Simple | UNIFORME | terrain2.png |
| 119 | soulSand (SoulSandTile) | Simple | UNIFORME | terrain2.png |
| 128 | stairs_sandStone | Escalera | UNIFORME | terrain.png |
| 150 | sprucePlanks | Simple | UNIFORME | terrain2.png |
| 151 | birchPlanks | Simple | UNIFORME | terrain2.png |
| 155 | quartzBlock (QuartzBlockTile) | Multi-textura | ESPECÍFICO | terrain.png |
| 156 | stairs_quartz | Escalera | UNIFORME | terrain.png |
| 157 | spruceSlab | Slab | UNIFORME | terrain2.png |
| 158 | spruceSlabHalf | Slab | UNIFORME | terrain2.png |
| 159 | birchSlab | Slab | UNIFORME | terrain2.png |
| 160 | birchSlabHalf | Slab | UNIFORME | terrain2.png |
| 161 | stairs_spruce | Escalera | UNIFORME | terrain2.png |
| 162 | stairs_birch | Escalera | UNIFORME | terrain2.png |
| 163 | fenceBirch | Valla | UNIFORME | terrain2.png |
| 164 | fenceSpruce | Valla | UNIFORME | terrain2.png |
| 165 | fenceGateBirch | Direccional | ESPECÍFICO | terrain2.png |
| 166 | fenceGateSpruce | Direccional | ESPECÍFICO | terrain2.png |
| 170 | deepslate (DeepslateTile) | Simple | UNIFORME | terrain2.png |
| 171 | cobbledDeepslate | Simple | UNIFORME | terrain2.png |
| 172 | deepslateCoalOre | Simple | UNIFORME | terrain2.png |
| 173 | deepslateDiamondOre | Simple | UNIFORME | terrain2.png |
| 174 | deepslateGoldOre | Simple | UNIFORME | terrain2.png |
| 175 | deepslateIronOre | Simple | UNIFORME | terrain2.png |
| 176 | deepslateLapisOre | Simple | UNIFORME | terrain2.png |
| 177 | deepslateRedstoneOre | Simple | UNIFORME | terrain2.png |
| 178 | deepslateRedstoneOre_lit | Simple | UNIFORME | terrain2.png |
| 181 | deepslatePolished | Simple | UNIFORME | terrain2.png |
| 182 | deepslateTiles | Simple | UNIFORME | terrain2.png |
| 183 | deepslateBricks | Simple | UNIFORME | terrain2.png |
| 179 | dandelion (FlowerTile) | Planta | UNIFORME | terrain.png |
| 180 | cornflower (FlowerTile) | Planta | UNIFORME | terrain.png |
| 245 | stonecutterBench (StonecutterTile) | Especial | ESPECÍFICO | terrain.png |
| 246 | glowingObsidian (ObsidianTile) | Simple | UNIFORME | terrain.png |
| 247 | netherReactor (NetherReactor) | Direccional | ESPECÍFICO | terrain.png |
| 248 | info_updateGame1 | Info | UNIFORME | terrain.png |
| 249 | info_updateGame2 | Info | UNIFORME | terrain.png |
| 253 | grass_carried (CarriedTile) | Simple | UNIFORME | terrain.png |
| 254 | leaves_carried (LeafTile) | Especial | UNIFORME | terrain.png |

---

## Lista Completa de Archivos de Textura a Crear

### Bloques Simples (UNIFORME)
```
stone.png
dirt.png
stone_brick.png
planks_oak.png
sand.png
gravel.png
gold_ore.png
iron_ore.png
coal_ore.png
glass.png
lapis_ore.png
lapis_block.png
mossy_cobblestone.png
obsidian.png
brick.png
tnt.png
gold_block.png
iron_block.png
diamond_ore.png
diamond_block.png
netherrack.png
glowstone.png
nether_brick.png
nether_quartz_ore.png
end_stone.png
soul_sand.png
glowing_obsidian.png
planks_spruce.png
planks_birch.png
deepslate.png
cobbled_deepslate.png
deepslate_coal_ore.png
deepslate_diamond_ore.png
deepslate_gold_ore.png
deepslate_iron_ore.png
deepslate_lapis_ore.png
deepslate_redstone_ore.png
deepslate_redstone_ore_lit.png
polished_deepslate.png
deepslate_tiles.png
deepslate_bricks.png
cobweb.png
cactus_side.png
clay.png
sugar_cane.png
ice.png
snow.png
snow_layer.png
redstone_ore.png
redstone_ore_lit.png
sapling.png
tall_grass.png
mushroom_red.png
mushroom_brown.png
flower_dandelion.png
flower_cornflower.png
fire.png
portal.png
glass_pane.png
torch.png
ladder.png
fence_oak.png
fence_birch.png
fence_spruce.png
wheat.png
```

### Bloques Simplificados (side, top, bottom)
```
grass_top.png
grass_side.png
grass_side_snow.png
log_oak_top.png
log_oak_side.png
log_birch_top.png
log_birch_side.png
log_spruce_top.png
log_spruce_side.png
bookshelf_side.png
melon_top.png
melon_side.png
```

### Bloques Específicos (6 caras)
```
furnace_top.png
furnace_bottom.png
furnace_front.png
furnace_front_lit.png
furnace_side.png
chest_top.png
chest_front.png
chest_side.png
crafting_table_top.png
crafting_table_front.png
crafting_table_side.png
crafting_table_back.png
stonecutter_top.png
stonecutter_bottom.png
stonecutter_front.png
stonecutter_side.png
sandstone_top.png
sandstone_bottom.png
sandstone_normal.png
sandstone_chiseled.png
sandstone_smooth.png
stonebrick_normal.png
stonebrick_mossy.png
stonebrick_cracked.png
quartz_top.png
quartz_bottom.png
quartz_side.png
quartz_chiseled_top.png
quartz_chiseled.png
quartz_lines_top.png
quartz_lines.png
nether_reactor_core.png
nether_reactor_active.png
nether_reactor_used.png
door_wood_top_bottom.png
door_wood_upper.png
door_wood_lower.png
door_iron_top_bottom.png
door_iron_upper.png
door_iron_lower.png
```

### Bloques con Variantes
```
leaves_oak.png
leaves_spruce.png
leaves_birch.png
leaves_carried.png
```

---

## Resumen de Formatos

### FORMATO UNIFORME (60+ bloques)
Bloques simples con una sola textura para todas las caras.

### FORMATO SIMPLIFICADO (5 bloques)
Bloques con texturas laterales iguales y top/bottom diferentes:
- grass
- treeTrunk (oak)
- birchTrunk
- spruceTrunk
- bookshelf
- melon

### FORMATO ESPECÍFICO (15+ bloques)
Bloques con texturas únicas en cada cara o con variantes complejas:
- furnace (unlit + lit)
- chest
- workbench
- stonecutter
- sandStone (3 variantes)
- stoneBrickSmooth (3 variantes)
- quartzBlock (3 variantes)
- bed
- door_wood
- door_iron
- trapdoor
- fenceGate
- netherReactor
- sign
- wallSign

---

## Notas Importantes

1. **TEXTURE_ALT_FLAG**: Los bloques con este flag (0x1000) usan terrain2.png en lugar de terrain.png
2. **Bloques líquidos** (water, lava) no necesitan texturas separadas (us shaders)
3. **Bloques invisibles** (invisible_bedrock) no necesitan texturas
4. **Bloques animados** (fire) pueden necesitar spritesheets
5. **Bloques direccionales** (furnace, chest, doors) requieren lógica especial para determinar qué cara es el "frente"
6. **Bloques con variantes** (leaves, cloth, sandstone) usan el campo data para seleccionar la textura
