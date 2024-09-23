#ifndef GL_HEADER_DEFINED
#define GL_HEADER_DEFINED

#include "glad/gl.h"
#include "models/model.h"
#include "util.h"

/*
* Abstraction layer over opengl, adapted for usage in this application
*/

#define PACKED __attribute__((packed))

/// enum mapping to buffer
typedef enum {
  VERTICES,
  NORMALS,
  TANGENT,
  TEXCOORD,
  BUFFER_CONTENT_MAX
} BufferContent;

/// a container for the VAO, VBOs, EBOs needed to render a model
typedef struct {
  Mesh* mesh;
  GLuint vao;
  GLuint ebo;
  GLuint vbo[BUFFER_CONTENT_MAX];
} GlMeshWrapper;

DEFINE_SLICE_STRUCT(GlMeshWrapper)

void initMeshWrappers(ModelSlice models, GlMeshWrapperSlice wrappers);

#endif
