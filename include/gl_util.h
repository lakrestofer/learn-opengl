#ifndef GL_HEADER_DEFINED
#define GL_HEADER_DEFINED

#include "glad/gl.h"
#include "models/model.h"

/*
* Abstraction layer over opengl, adapted for usage in this application.
* We assume a convention that will be used for all shaders
*/

#define PACKED __attribute__((packed))

typedef enum {
  VERTEX,
  NORMAL,
  TANGENT,
  TEXCOORD,
  N_BUFFER_TYPES
} BUFFER_TYPE;

// the number of floats in each buffer type
static const char COMPONENT_SIZE[N_BUFFER_TYPES] = {3, 3, 4, 2};

typedef struct {
  GLuint vertices;
  GLuint normals;
  GLuint tangent;
  GLuint texcoord;
} PACKED VBOBuffers;

// SOA. groupings of identifiers 
typedef struct {
  GLuint* vao; // vertex array ids
  GLuint* ebo; // vertex buffer ids for indices
  VBOBuffers* vbo; // vertex buffer ids
} GlIdentifier;


void genGlIds(GlIdentifier* ids, int n);
void syncBuffers(Mesh* m, GlIdentifier* ids, int n);

#endif
