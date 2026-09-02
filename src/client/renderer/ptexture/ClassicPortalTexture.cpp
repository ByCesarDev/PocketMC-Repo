#include "ClassicPortalTexture.h"
#include "../../../world/level/tile/Tile.h"
#include "../../../AppPlatform.h"
#include <cstring>

ClassicPortalTexture::ClassicPortalTexture()
:   super(Tile::classicPortal->tex),
    _frame(0),
    _frameCount(32),
    _sheetData(nullptr)
{
}

ClassicPortalTexture::~ClassicPortalTexture() {
    delete[] _sheetData;
}

void ClassicPortalTexture::loadSheet(AppPlatform* platform) {
    TextureData td = platform->loadTexture("blocks/classic_portal.png", true);
    if (td.data && td.format == TEXF_UNCOMPRESSED_8888) {
        _frameCount = td.h / 16;
        if (_frameCount < 1) _frameCount = 1;
        const int totalBytes = 16 * 16 * _frameCount * 4;
        _sheetData = new unsigned char[totalBytes];
        memcpy(_sheetData, td.data, totalBytes);
    }
    if (td.data && !td.memoryHandledExternally)
        delete[] td.data;

    if (_sheetData)
        memcpy(pixels, _sheetData, 16 * 16 * 4);
}

void ClassicPortalTexture::tick() {
    if (!_sheetData) return;

    int currentFrame = _frame % _frameCount;
    const int frameOffset = currentFrame * 16 * 16 * 4;
    memcpy(pixels, _sheetData + frameOffset, 16 * 16 * 4);

    _frame++;
}
