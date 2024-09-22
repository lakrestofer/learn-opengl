#ifndef MODEL_HEADER_DEFINED
#define MODEL_HEADER_DEFINED

#include <cgltf/cgltf.h>

typedef struct {
  int n_vertices;
  float* vertices;
  float* indices;
} Mesh;

typedef struct {
  int n_meshes;
  Mesh* meshes;
} Model;

typedef enum {
  SUCCESS,
  ERROR,
} LoadModelRes;

void printGltfInfo(cgltf_data* data);

LoadModelRes loadModelFromGltfFile(const char* path, Model* model);

void freeMesh(Mesh* mesh);
void freeModel(Model* model);

#endif
