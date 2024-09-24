#ifndef GL_HEADER_DEFINED
#define GL_HEADER_DEFINED

#include "glad/gl.h"
#include "models/model.h"
#include "util.h"

/*
* Abstraction layer over opengl, adapted for usage in this application
*/

#define PACKED __attribute__((packed))


typedef enum {
  VERTEX,
  NORMAL,
  TANGENT,
  TEXCOORD,
  N_BUFFER_TYPES
} BUFFER_TYPE;
static const char COMPONENT_SIZE[N_BUFFER_TYPES] = {3, 3, 4, 2};

// the number of floats in each buffer type
static const char BUFFER_TYPE_SIZE[N_BUFFER_TYPES] = {3, 3, 4, 2};

typedef struct {
  GLuint vertices;
  GLuint normals;
  GLuint tangent;
  GLuint texcoord;
} PACKED VBOBuffers;

// groupings of identifiers 
typedef struct {
  GLuint* vao;
  GLuint* ebo;
  VBOBuffers* vbo;
} GlIdentifier;


void genGlIds(GlIdentifier* ids, int n);

void syncBuffers(Mesh* m, GlIdentifier* ids, int n);

#endif
