#ifndef TEXTUER_HEADER_DEFINED
#define TEXTUER_HEADER_DEFINED
#include "glad/gl.h"

typedef enum { PNG, JPG } ImageType;
GLuint createTexture(const char* fileName, ImageType type);

#endif
