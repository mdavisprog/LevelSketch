#include <stdlib.h>

void* (*zMallocPtr)(size_t size) = NULL;
void* (*zReallocPtr)(void* ptr, size_t size) = NULL;
void (*zFreePtr)(void* ptr) = NULL;

#define STBI_MALLOC(size) zMallocPtr(size)
#define STBI_REALLOC(ptr, size) zReallocPtr(ptr, size)
#define STBI_FREE(ptr) zFreePtr(ptr)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "stb_image.h"

#define STBTT_malloc(x, u) zMallocPtr(x)
#define STBTT_free(x, u) zFreePtr(x)

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
