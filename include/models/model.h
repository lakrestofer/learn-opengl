#ifndef MODEL_HEADER_DEFINED
#define MODEL_HEADER_DEFINED

#include <cgltf/cgltf.h>

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

void printGltfInfo(cgltf_data* data);
void printMesh(Mesh* m);

LoadModelRes loadModelFromGltfFile(const char* path, Model* model);

void freeMesh(Mesh* mesh);
void freeModel(Model* model);

#endif
