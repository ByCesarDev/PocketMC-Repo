#ifndef NET_MINECRAFT_CLIENT_RENDERER_PTEXTURE__DynamicTexture_H__
#define NET_MINECRAFT_CLIENT_RENDERER_PTEXTURE__DynamicTexture_H__

#include <string>
#include <vector>

class Textures;
class AppPlatform;

class DynamicTexture
{
public:
    int tex;
	int replicate;
    unsigned char pixels[16*16*4];

    DynamicTexture(int tex_);
	virtual ~DynamicTexture() {}

	virtual void tick() = 0;
    virtual void bindTexture(Textures* tex);
};

class WaterTexture: public DynamicTexture
{
    typedef DynamicTexture super;
    int _tick;
	int _frame;

	float* current;
	float* next;
	float* heat;
	float* heata;

public:
    WaterTexture();
	~WaterTexture();

    void tick();
};

class WaterSideTexture: public DynamicTexture
{
	typedef DynamicTexture super;
	int _tick;
	int _frame;
	int _tickCount;

	float* current;
	float* next;
	float* heat;
	float* heata;

public:
	WaterSideTexture();
	~WaterSideTexture();

	void tick();
};

class LavaTexture: public DynamicTexture
{
	typedef DynamicTexture super;
	int _frame;
	int _frameCount;
	unsigned char* _sheetData;
public:
	LavaTexture();
	~LavaTexture();
	void tick() override;
	void loadSheet(AppPlatform* platform);
};

class LavaSideTexture: public DynamicTexture
{
	typedef DynamicTexture super;
	int _frame;
	int _frameCount;
	unsigned char* _sheetData;
public:
	LavaSideTexture();
	~LavaSideTexture();
	void tick() override;
	void loadSheet(AppPlatform* platform);
};

class PortalTexture: public DynamicTexture
{
	typedef DynamicTexture super;
	int _frame;
	int _frameCount;
	unsigned char* _sheetData;
public:
	PortalTexture();
	~PortalTexture();
	void tick() override;
	void loadSheet(AppPlatform* platform);
};

class FireTexture: public DynamicTexture
{
    typedef DynamicTexture super;
    int _tick;
	int _frame;

	float* current;
	float* next;
	float* heat;
	float* heata;
public:
    FireTexture(int id);
	~FireTexture();

    void tick() override;
};

class SoulFireTexture: public DynamicTexture
{
	typedef DynamicTexture super;
	int _frame;
	int _frameCount;
	unsigned char* _sheetData;

public:
	SoulFireTexture(int textureSlot = 216);
	~SoulFireTexture();

	void loadSheet(AppPlatform* platform);
	void tick() override;
	void bindTexture(Textures* tex) override;
};

class AnimatedBlockTexture : public DynamicTexture
{
	typedef DynamicTexture super;
	int _frame;
	int _frameCount;
	int _ticksPerFrame;
	int _tickCounter;
	unsigned char* _sheetData;
	std::string _imagePath;
	std::string _atlasName;

public:
	AnimatedBlockTexture(int textureSlot, const std::string& imagePath, const std::string& atlasName = "terrain2.png", int ticksPerFrame = 2);
	virtual ~AnimatedBlockTexture();

	void loadSheet(AppPlatform* platform);
	void tick() override;
	void bindTexture(Textures* tex) override;
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_PTEXTURE__DynamicTexture_H__*/
