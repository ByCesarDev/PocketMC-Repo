#ifndef NET_MINECRAFT_CLIENT_MODEL__HumanoidModel_H__
#define NET_MINECRAFT_CLIENT_MODEL__HumanoidModel_H__

//package net.minecraft.client.model;

#include "Model.h"
#include "geom/ModelPart.h"
class ItemInstance;
class HumanoidModel: public Model
{
public:
HumanoidModel(float g = 0, float yOffset = 0, int texW = 64, int texH = 32, bool slim = false);

	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale);

	void render(HumanoidModel* model, float scale);
    void render(Entity* e, float time, float r, float bob, float yRot, float xRot, float scale);
	void renderHorrible(float time, float r, float bob, float yRot, float xRot, float scale);
	void onGraphicsReset();
	virtual void prepareMobModel(Mob* mob, float time, float r, float a) override;

	ModelPart head, hair, body, arm0, arm1, leg0, leg1;//, ear;
	bool isSlim;
	bool holdingLeftHand;
	bool holdingRightHand;
	bool sneaking;
	bool bowAndArrow;
	bool swimming;
	float swimLean;
};

#endif /*NET_MINECRAFT_CLIENT_MODEL__HumanoidModel_H__*/
