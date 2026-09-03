#ifndef NET_MINECRAFT_WORLD_LEVEL__WorldType_H__
#define NET_MINECRAFT_WORLD_LEVEL__WorldType_H__

class WorldType
{
public:
	enum Type {
		INFINITE_SIZE = 0, // Current PocketMC unbounded generation (DEFAULT)
		FLAT          = 1, // Superflat world
		POCKET        = 2, // 256x256 blocks (16x16 chunks)
		SMALL         = 3, // 512x512 blocks (32x32 chunks)
		LARGE         = 4, // 2560x2560 blocks (160x160 chunks)
		WORLD_TYPE_COUNT
	};

	static int getChunkWidth(int worldType) {
		switch (worldType) {
		case POCKET: return 16;
		case SMALL:  return 32;
		case LARGE:  return 160;
		default:     return -1; // Infinite or Flat
		}
	}

	static bool isInfinite(int worldType) {
		return worldType == INFINITE_SIZE || worldType == FLAT;
	}

	static bool isFlat(int worldType) {
		return worldType == FLAT;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL__WorldType_H__*/
