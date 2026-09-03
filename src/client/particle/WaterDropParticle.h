#ifndef NET_MINECRAFT_CLIENT_PARTICLE__WaterDropParticle_H__
#define NET_MINECRAFT_CLIENT_PARTICLE__WaterDropParticle_H__

//package net.minecraft.client.particle;

#include "Particle.h"
#include "../../world/level/Level.h"

class WaterDropParticle: public Particle
{
	typedef Particle super;
public:
    WaterDropParticle(Level* level, float x, float y, float z);

    virtual void tick();
};

#endif /*NET_MINECRAFT_CLIENT_PARTICLE__WaterDropParticle_H__*/
