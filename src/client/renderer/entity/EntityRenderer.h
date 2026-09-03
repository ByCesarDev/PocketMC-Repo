#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EntityRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EntityRenderer_H__

//package net.minecraft.client.renderer.entity;

#include <string>
#include "../../model/HumanoidModel.h"

class Textures;
class Tesselator;
class EntityRenderDispatcher;
class Entity;
class AABB;
class Font;
class Tile;
class Level;

class EntityRenderer
{
protected:
	EntityRenderer();
public:
    virtual ~EntityRenderer() {}
	void init(EntityRenderDispatcher* entityRenderDispatcher);

	virtual void render(Entity* entity, float x, float y, float z, float rot, float a) = 0;
    static void render(const AABB& bb, float xo, float yo, float zo);
    static void renderFlat(const AABB& bb);
	void renderShadow(Entity* e, float x, float y, float z, float pow, float a);
	void renderTileShadow(Tile* tt, float x, float y, float z, int xt, int yt, int zt, float pow, float r, float xo, float yo, float zo);
	void renderFlame(Entity* e, float x, float y, float z, float a);
	void postRender(Entity* entity, float x, float y, float z, float rot, float a);
	Level* getLevel();

	Font* getFont();

	virtual void onGraphicsReset() {}

protected:
	void bindTexture(const std::string& resourceName);

	float shadowRadius;
	float shadowStrength;
	static EntityRenderDispatcher* entityRenderDispatcher;
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EntityRenderer_H__*/
