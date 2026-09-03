#ifndef NET_MINECRAFT_WORLD_LEVEL__GrassColor_H__
#define NET_MINECRAFT_WORLD_LEVEL__GrassColor_H__

class GrassColor
{
public:
	static bool useTint;
	static bool useAlpha;

	static void setUseTint(bool value) {
		useTint = value;
	}

	static void setUseAlpha(bool value) {
		useAlpha = value;
	}

	static int getAlphaColor() {
		return 0x95D46C;
	}

	static void init(int* p) {
		pixels = p;
	}

	static int get(float temp, float rain);

private:
	static int* pixels;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL__GrassColor_H__*/
