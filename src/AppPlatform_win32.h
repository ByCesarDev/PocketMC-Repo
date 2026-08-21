#ifndef APPPLATFORM_WIN32_H__
#define APPPLATFORM_WIN32_H__

#include "AppPlatform.h"
#include "platform/log.h"
#include "platform/HttpClient.h"
#include "platform/PngLoader.h"
#include "client/renderer/gles.h"
#include "world/level/storage/FolderMethods.h"
#include "util/StringUtils.h"
#include <png.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <shellapi.h>

static void png_funcReadFile(png_structp pngPtr, png_bytep data, png_size_t length) {
	((std::istream*)png_get_io_ptr(pngPtr))->read((char*)data, length);
}

class AppPlatform_win32: public AppPlatform
{
public:
    AppPlatform_win32()
    {
    }

	BinaryBlob readAssetFile(const std::string& filename) {
		FILE* fp = fopen(("data/" + filename).c_str(), "r");
		if (!fp)
			return BinaryBlob();

		int size = getRemainingFileSize(fp);

		BinaryBlob blob;
		blob.size = size;
		blob.data = new unsigned char[size];

		fread(blob.data, 1, size, fp);
		fclose(fp);

		return blob;
	}

    void saveScreenshot(const std::string& filename, int glWidth, int glHeight) {
        //@todo
    }

    __inline unsigned int rgbToBgr(unsigned int p) {
        return (p & 0xff00ff00) | ((p >> 16) & 0xff) | ((p << 16) & 0xff0000);
    }

    TextureData loadTexture(const std::string& filename_, bool textureFolder)
	{
		// Support fetching PNG textures via HTTP/HTTPS (for skins, etc).
		if (Util::startsWith(filename_, "http://") || Util::startsWith(filename_, "https://")) {
			std::vector<unsigned char> body;
			if (HttpClient::download(filename_, body) && !body.empty()) {
				return loadTextureFromMemory(body.data(), body.size());
			}
			return TextureData();
		}

		std::string filename = filename_;
		std::string checkFilename = filename_;
		for (char& c : checkFilename) {
			if (c == '\\') c = '/';
		}

		if (textureFolder) {
			if (checkFilename.rfind("games/", 0) != 0 && checkFilename.rfind("data/", 0) != 0) {
				filename = "data/images/" + filename_;
			}
		}

		for (char& c : filename) {
			if (c == '\\') c = '/';
		}

		std::ifstream source(filename.c_str(), std::ios::binary);
		if (!source) {
			LOGI("Couldn't find file: %s\n", filename.c_str());
			return TextureData();
		}

		std::vector<unsigned char> fileData((std::istreambuf_iterator<char>(source)), std::istreambuf_iterator<char>());
		source.close();

		if (fileData.empty()) {
			LOGI("Couldn't read file: %s\n", filename.c_str());
			return TextureData();
		}

		return loadTextureFromMemory(fileData.data(), fileData.size());
    }

	TextureData loadTextureFromMemory(const unsigned char* data, size_t size) override {
		return loadPngFromMemory(data, size);
	}

	virtual std::string getDateString(int s) {
		time_t tm = s;

		char mbstr[100];
		std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&tm));

		return std::string(mbstr);
	}

	virtual int getScreenWidth();
	virtual int getScreenHeight();

	virtual float getPixelsPerMillimeter();

	virtual bool supportsTouchscreen();

	virtual void openURL(const std::string& url) {
		ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

private:
};

#endif /*APPPLATFORM_WIN32_H__*/
