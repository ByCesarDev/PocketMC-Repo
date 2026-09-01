#include "ChestTile.h"
#include "Tile.h"
#include "entity/ChestTileEntity.h"
#include "../../entity/player/Player.h"
#include "../Level.h"

int ChestTile::getTexture(int face, int data) {
    // 0: Abajo, 1: Arriba
    if (face == 1 || face == 0) return 25; 
    
    // Si data es 0 (por ejemplo en inventario/GUI), el frente por defecto es la cara 2 (Norte)
    if (data == 0) {
        if (face == 2) return 27;
        return 26;
    }

    // Si la cara coincide con la dirección guardada en 'data', ponemos el frente (cerradura)
    if (face == data) return 27; 
    
    // En cualquier otro caso, la textura lateral
    return 26; 
}

void ChestTile::onPlace(Level* level, int x, int y, int z) {
    // Al colocar el bloque, recalculamos la dirección para ver si se une a otro cofre
    recalcLockDir(level, x, y, z);
}

void ChestTile::neighborChanged(Level* level, int x, int y, int z, int neighborTileId) {
    // Si el bloque que cambió es otro cofre, recalculamos la orientación
    // para mantener la integridad del cofre doble
    if (neighborTileId == this->id) {
        recalcLockDir(level, x, y, z);
    }
}

bool ChestTile::use(Level* level, int x, int y, int z, Player* player) {
    if (level->isClientSide) {
        return true;
    }

    // Obtenemos la TileEntity para abrir el inventario
    ChestTileEntity* tileEntity = (ChestTileEntity*)level->getTileEntity(x, y, z);
    if (tileEntity) {
        player->openContainer(tileEntity);
    }
    return true;
}

TileEntity* ChestTile::newTileEntity() {
    return new ChestTileEntity();
}

void ChestTile::recalcLockDir(Level* level, int x, int y, int z) {
    // Esta función es llamada por el renderizador. 
    // Si el bloque no tiene datos de orientación (meta 0), 
    // aquí se debería calcular hacia dónde mira el cofre.
    if (level->getData(x, y, z) == 0) {
        // Orientación por defecto (Sur)
        level->setDataNoUpdate(x, y, z, 3);
    }
}