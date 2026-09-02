#include "AncientDebrisTile.h"
#include "../material/Material.h"
#include "../../item/ItemCategory.h"

AncientDebrisTile::AncientDebrisTile(int id) 
    : Tile(id, 30 | Tile::TEXTURE_ALT_FLAG, Material::metal) {
    setDestroyTime(30.0f);
    setExplodeable(1200.0f);
    setSoundType(SOUND_STONE);
    setCategory(ItemCategory::Decorations);
    setDescriptionId("ancientDebris");
    setFaceTexture(FACE_UP, "ancient_debris_top", 31 | Tile::TEXTURE_ALT_FLAG);
    setFaceTexture(FACE_DOWN, "ancient_debris_top", 31 | Tile::TEXTURE_ALT_FLAG);
    setFaceTexture(FACE_NORTH, "ancient_debris_side", 30 | Tile::TEXTURE_ALT_FLAG);
    setFaceTexture(FACE_SOUTH, "ancient_debris_side", 30 | Tile::TEXTURE_ALT_FLAG);
    setFaceTexture(FACE_WEST, "ancient_debris_side", 30 | Tile::TEXTURE_ALT_FLAG);
    setFaceTexture(FACE_EAST, "ancient_debris_side", 30 | Tile::TEXTURE_ALT_FLAG);
}

int AncientDebrisTile::getTexture(int face) {
    if (face == 0 || face == 1) { // DOWN or UP
        return 31 | Tile::TEXTURE_ALT_FLAG;
    }
    return 30 | Tile::TEXTURE_ALT_FLAG;
}

int AncientDebrisTile::getTexture(int face, int data) {
    return getTexture(face);
}

int AncientDebrisTile::getTexture(LevelSource* level, int x, int y, int z, int face) {
    return getTexture(face);
}
