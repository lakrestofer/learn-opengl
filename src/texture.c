#include "textures/texture.h"
#include "external/stb_image.h"
#include <stdbool.h>

GLuint createTexture(const char* fileName, ImageType type) {
  int iw, ih, nbrChnls;
  if (type == PNG) stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(fileName, &iw, &ih, &nbrChnls, 0);
  if (!data) return 0;
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB,
      iw,
      ih,
      0,
      type == JPG ? GL_RGB : GL_RGBA,
      GL_UNSIGNED_BYTE,
      data
  );
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);
  if (type == PNG) stbi_set_flip_vertically_on_load(false);
  return texture;
}
