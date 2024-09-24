#ifndef MODEL_HEADER_DEFINED
#define MODEL_HEADER_DEFINED

#include "glad/gl.h"
#include <cgltf/cgltf.h>
#include "util.h"


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
  int n_vertices;
  int n_triangles;
  float* vertices;
  float* normals;
  float* tangents;
  float* tex_coords;
  unsigned short* indices;
} Mesh;

typedef struct {
  int n_meshes;
  Mesh* meshes;
} Model;


typedef enum {
  SUCCESS,
  ERROR,
} LoadModelRes;

LoadModelRes loadModelFromGltfFile(const char* path, Model* model);

void freeMesh(Mesh* mesh);
void freeModel(Model* model);

#endif
