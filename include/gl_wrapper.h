#ifndef GL_HEADER_DEFINED
#define GL_HEADER_DEFINED

#include "glad/gl.h"
#include "models/model.h"
#include <stdlib.h>
#include <string.h>

/*
* Abstraction layer over opengl, adapted for usage in this application
*/

#define PACKED __attribute__((packed))

typedef struct {
  short i;
  short n;
} VboSlice;

/// a container for the VAO, VBOs, EBOs needed to render several models
/// uses SOA layout
typedef struct {
  GLuint* vao;
  short* vbo_map;
  short* vbo_len_map;
  GLuint* vbo;
  GLuint* ebo;
} GlObjects;

void initObjects(Model* models, GlObjects* wrappers, int n_models);

#endif
