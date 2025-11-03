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
