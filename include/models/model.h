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

void printGltfInfo(cgltf_data* data);

Model* loadModelFromGltfFile(const char* path);

void freeMesh(Mesh* mesh);
void freeModel(Model* model);

#endif
