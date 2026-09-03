#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatGeneratorInfo_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatGeneratorInfo_H__

#include <string>
#include <vector>

class FlatLayerInfo;

class FlatGeneratorInfo
{
public:
	static const int SERIALIZATION_VERSION = 2;

	FlatGeneratorInfo();
	~FlatGeneratorInfo();

	std::vector<FlatLayerInfo*>& getLayers() { return layers; }
	const std::vector<FlatLayerInfo*>& getLayers() const { return layers; }

	void updateLayers();

	static void getDefault(FlatGeneratorInfo& out);

private:
	std::vector<FlatLayerInfo*> layers;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatGeneratorInfo_H__*/
