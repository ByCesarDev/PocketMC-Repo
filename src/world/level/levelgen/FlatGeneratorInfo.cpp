#include "FlatGeneratorInfo.h"
#include "FlatLayerInfo.h"
#include "../tile/Tile.h"

FlatGeneratorInfo::FlatGeneratorInfo()
{
}

FlatGeneratorInfo::~FlatGeneratorInfo()
{
	for (std::vector<FlatLayerInfo*>::iterator it = layers.begin(); it != layers.end(); ++it)
		delete *it;
}

void FlatGeneratorInfo::updateLayers()
{
	int y = 0;
	for (std::vector<FlatLayerInfo*>::iterator it = layers.begin(); it != layers.end(); ++it) {
		(*it)->setStart(y);
		y += (*it)->getHeight();
	}
}

void FlatGeneratorInfo::getDefault(FlatGeneratorInfo& result)
{
	for (std::vector<FlatLayerInfo*>::iterator it = result.layers.begin(); it != result.layers.end(); ++it)
		delete *it;
	result.layers.clear();

	result.getLayers().push_back(new FlatLayerInfo(1, Tile::unbreakable->id));
	result.getLayers().push_back(new FlatLayerInfo(2, Tile::dirt->id));
	result.getLayers().push_back(new FlatLayerInfo(1, Tile::grass->id));
	result.updateLayers();

	const int seaLevel = 63;
	int height = 0;
	int topLayerHeight = 0;
	for (std::vector<FlatLayerInfo*>::iterator it = result.layers.begin(); it != result.layers.end(); ++it) {
		topLayerHeight = (*it)->getHeight();
		height += topLayerHeight;
	}
	if (height < seaLevel) {
		int belowTop = height - topLayerHeight;
		int filler = seaLevel - belowTop;
		FlatLayerInfo* dirtFiller = new FlatLayerInfo(filler, Tile::dirt->id);
		result.layers.insert(result.layers.end() - 1, dirtFiller);
		result.updateLayers();
	}
}
