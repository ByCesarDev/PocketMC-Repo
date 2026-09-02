#ifndef NET_MINECRAFT_CLIENT_RENDERER_PTEXTURE__ClassicPortalTexture_H__
#define NET_MINECRAFT_CLIENT_RENDERER_PTEXTURE__ClassicPortalTexture_H__

#include "DynamicTexture.h"

class AppPlatform;

class ClassicPortalTexture: public DynamicTexture
{
    typedef DynamicTexture super;
    int _frame;
    int _frameCount;
    unsigned char* _sheetData;

public:
    ClassicPortalTexture();
    ~ClassicPortalTexture();
    void loadSheet(AppPlatform* platform);
    virtual void tick() override;
};

#endif
