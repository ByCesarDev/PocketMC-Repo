#ifndef NET_MINECRAFT_WORLD_LEVEL__FoliageColor_H__
#define NET_MINECRAFT_WORLD_LEVEL__FoliageColor_H__

class FoliageColor
{
public:
	static bool useTint;

	static void setUseTint(bool value) {
		useTint = value;
	}

	static void init(int* p) {
		pixels = p;
	}

	static int get(float temp, float rain);

	static int getEvergreenColor() {
		return 0x619961;
	}

	static int getBirchColor() {
		return 0x80a755;
	}

	static int getDefaultColor() {
		return 0x48b518;
	}

private:
	static int* pixels;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL__FoliageColor_H__*/
