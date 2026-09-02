#ifndef NET_MINECRAFT_WORLD_LEVEL__PortalForcer_H__
#define NET_MINECRAFT_WORLD_LEVEL__PortalForcer_H__

#include "Level.h"
#include "../../util/Random.h"

class Entity;

class PortalForcer
{
public:
    void force(Level* level, Entity* entity);
    bool findPortal(Level* level, Entity* entity);
    bool createPortal(Level* level, Entity* entity);

private:
    Random m_random;
};

#endif
