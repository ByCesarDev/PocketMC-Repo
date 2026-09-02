#include "NetherStalkTile.h"
#include "../Level.h"
#include "../../item/Item.h"
#include "../../item/ItemCategory.h"
#include "../../entity/item/ItemEntity.h"
#include "../../../util/Random.h"

NetherStalkTile::NetherStalkTile(int id) : Bush(id, 26 | Tile::TEXTURE_ALT_FLAG) {
	setShape(0, 0, 0, 1, 0.875f, 1);
}

bool NetherStalkTile::mayPlaceOn(int tile) {
	return Tile::soulSand && tile == Tile::soulSand->id;
}

bool NetherStalkTile::canSurvive(Level* level, int x, int y, int z) {
	return mayPlaceOn(level->getTile(x, y - 1, z));
}

void NetherStalkTile::tick(Level* level, int x, int y, int z, Random* random) {
	if (!canSurvive(level, x, y, z)) {
		spawnResources(level, x, y, z, level->getData(x, y, z), 1.0f);
		level->setTile(x, y, z, 0);
		return;
	}
	int age = level->getData(x, y, z);
	if (!level->isClientSide && age < 3 && random->nextInt(10) == 0) {
		level->setData(x, y, z, age + 1);
	}
}

int NetherStalkTile::getTexture(int face, int data) {
	if (data >= 3) return 28 | Tile::TEXTURE_ALT_FLAG;
	if (data >= 1) return 27 | Tile::TEXTURE_ALT_FLAG;
	return 26 | Tile::TEXTURE_ALT_FLAG;
}

int NetherStalkTile::getTexture(LevelSource* level, int x, int y, int z, int face) {
	return getTexture(face, level->getData(x, y, z));
}

int NetherStalkTile::getRenderShape() {
	return Tile::SHAPE_ROWS;
}

int NetherStalkTile::getResource(int data, Random* random) {
	if (Item::netherWart) return Item::netherWart->id;
	return id;
}

int NetherStalkTile::getResourceCount(Random* random) {
	return 1;
}

void NetherStalkTile::spawnResources(Level* level, int x, int y, int z, int data, float odds) {
	if (level->isClientSide) return;
	int count = 1;
	if (data >= 3) {
		count = 2 + level->random.nextInt(3);
	}
	for (int i = 0; i < count; i++) {
		if (level->random.nextFloat() > odds) continue;
		int type = getResource(data, &level->random);
		if (type <= 0) continue;
		const float s = 0.7f;
		float xo = level->random.nextFloat() * s + (1 - s) * 0.5f;
		float yo = level->random.nextFloat() * s + (1 - s) * 0.5f;
		float zo = level->random.nextFloat() * s + (1 - s) * 0.5f;
		ItemEntity* item = new ItemEntity(level, (float)x + xo, (float)y + yo, (float)z + zo, ItemInstance(type, 1, 0));
		item->throwTime = 10;
		level->addEntity(item);
	}
}
