#include "FlatLayerInfo.h"

#include <string>

FlatLayerInfo::FlatLayerInfo(int height, int id)
:	height(height),
	id(id),
	data(0),
	start(0)
{
}

FlatLayerInfo::FlatLayerInfo(int height, int id, int data)
:	height(height),
	id(id),
	data(data),
	start(0)
{
}

std::string FlatLayerInfo::toString() const
{
	std::string result = std::to_string(id);

	if (height > 1)
		result = std::to_string(height) + "x" + result;
	if (data > 0)
		result += ":" + std::to_string(data);

	return result;
}
