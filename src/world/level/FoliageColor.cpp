#include "FoliageColor.h"

bool FoliageColor::useTint = true;
int* FoliageColor::pixels = nullptr;

int FoliageColor::get(float temp, float rain) {
	if (!pixels) {
		return getDefaultColor();
	}
	rain *= temp;
	int x = (int) ((1.0f - temp) * 255.0f);
	int y = (int) ((1.0f - rain) * 255.0f);
	if (x < 0) x = 0; if (x > 255) x = 255;
	if (y < 0) y = 0; if (y > 255) y = 255;
	return pixels[y << 8 | x];
}
