#include "WaterDropParticle.h"
#include "../../world/level/tile/LiquidTile.h"
#include "../../util/Mth.h"

WaterDropParticle::WaterDropParticle(Level* level, float x, float y, float z)
:	super(level, x, y, z, 0, 0, 0)
{
	gravity = 0.06f;
	xd *= 0.3f;
	yd = (float)(Mth::random()) * 0.2f + 0.1f;
	zd *= 0.3f;
	rCol = gCol = bCol = 1.0f;
	tex = 19 + sharedRandom.nextInt(4);
	setSize(0.01f, 0.01f);
	lifetime = (int)(8.0f / (Mth::random() * 0.8f + 0.2f));
}

void WaterDropParticle::tick()
{
	xo = x; yo = y; zo = z;

	yd -= gravity;
	move(xd, yd, zd);
	xd *= 0.98f;
	yd *= 0.98f;
	zd *= 0.98f;

	if (lifetime-- <= 0)
		remove();

	if (onGround) {
		if (Mth::random() < 0.5f)
			remove();
		xd *= 0.7f;
		zd *= 0.7f;
	}

	const Material* m = level->getMaterial(
		Mth::floor(x),
		Mth::floor(y),
		Mth::floor(z)
	);
	if (m && (m->isLiquid() || m->isSolid())) {
		float y0 = (float)(Mth::floor(y) + 1) - LiquidTile::getHeight(level->getData(Mth::floor(x), Mth::floor(y), Mth::floor(z)));
		if (yd < y0)
			remove();
	}
}
