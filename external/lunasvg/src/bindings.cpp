#include "lunasvg.h"
#include <cstdio>
#include <cstring>

extern "C" {

unsigned char* load(const char* path, int width, int height) {
    std::unique_ptr<lunasvg::Document> document = lunasvg::Document::loadFromFile(path);
    if (document == nullptr) {
        return nullptr;
    }

    lunasvg::Bitmap bitmap = document->renderToBitmap(width, height);
    if (bitmap.isNull()) {
        return nullptr;
    }

    bitmap.convertToRGBA();

    const size_t size = width * height * 4;
    unsigned char* result = (unsigned char*)std::malloc(size);
    std::memcpy(result, bitmap.data(), size);

    return result;
}

}
