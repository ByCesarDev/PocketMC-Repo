#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_OBJECT_TREE__VanillaTrees_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_OBJECT_TREE__VanillaTrees_H__

#include "../../../feature/Feature.h"

class BigOakTreeFeature : public Feature
{
public:
	BigOakTreeFeature(bool doUpdate = false);
	bool place(Level* level, Random* random, int x, int y, int z) override;
};

class AcaciaTreeFeature : public Feature
{
public:
	AcaciaTreeFeature(bool doUpdate = false);
	bool place(Level* level, Random* random, int x, int y, int z) override;
};

class DarkOakTreeFeature : public Feature
{
public:
	DarkOakTreeFeature(bool doUpdate = false);
	bool place(Level* level, Random* random, int x, int y, int z) override;
};

class MegaPineTreeFeature : public Feature
{
public:
	bool usePodzol;
	MegaPineTreeFeature(bool usePodzol = true, bool doUpdate = false);
	bool place(Level* level, Random* random, int x, int y, int z) override;
};

class MegaJungleTreeFeature : public Feature
{
public:
	MegaJungleTreeFeature(bool doUpdate = false);
	bool place(Level* level, Random* random, int x, int y, int z) override;
};

class SwampTreeFeature : public Feature
{
public:
	SwampTreeFeature(bool doUpdate = false);
	bool place(Level* level, Random* random, int x, int y, int z) override;
};

class HugeMushroomFeature : public Feature
{
public:
	int mushroomType; // 0 = Brown, 1 = Red
	HugeMushroomFeature(int type = 0, bool doUpdate = false);
	bool place(Level* level, Random* random, int x, int y, int z) override;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN_VANILLA_OBJECT_TREE__VanillaTrees_H__*/
