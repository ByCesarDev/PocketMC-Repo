#ifndef NET_MINECRAFT_CLIENT_RENDERER__WeatherRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER__WeatherRenderer_H__

#include "gles.h"
#include "../../util/Random.h"
#include "../../world/phys/Vec3.h"

class Minecraft;
class Level;

class WeatherRenderer
{
public:
	WeatherRenderer();
	~WeatherRenderer();

	void tick();
	void render(float a);

public:
	void setLevel(Minecraft* minecraft) { mc = minecraft; }

private:
	Minecraft* mc;
	Random random;
	int _tick;
	int _rainSoundTime;
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER__WeatherRenderer_H__*/
