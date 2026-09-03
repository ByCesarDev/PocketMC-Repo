#include "GrassColor.h"

bool GrassColor::useTint = true;
bool GrassColor::useAlpha = false;
int* GrassColor::pixels = nullptr;

int GrassColor::get(float temp, float rain) {
	if (useAlpha) {
		return getAlphaColor();
	}
	if (!pixels) {
		return 0x339933;
	}
	rain *= temp;
	int x = (int) ((1.0f - temp) * 255.0f);
	int y = (int) ((1.0f - rain) * 255.0f);
	if (x < 0) x = 0; if (x > 255) x = 255;
	if (y < 0) y = 0; if (y > 255) y = 255;
	return pixels[y << 8 | x];
}
