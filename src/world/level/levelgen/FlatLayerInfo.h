#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatLayerInfo_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatLayerInfo_H__

#include <string>

class FlatLayerInfo
{
public:
	FlatLayerInfo(int height, int id);
	FlatLayerInfo(int height, int id, int data);

	int getHeight() const { return height; }
	void setHeight(int height) { this->height = height; }

	int getId() const { return id; }
	void setId(int id) { this->id = id; }

	int getData() const { return data; }
	void setData(int data) { this->data = data; }

	int getStart() const { return start; }
	void setStart(int start) { this->start = start; }

	std::string toString() const;

private:
	int height;
	int id;
	int data;
	int start;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__FlatLayerInfo_H__*/
