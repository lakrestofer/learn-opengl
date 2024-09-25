#ifndef MODEL_HEADER_DEFINED
#define MODEL_HEADER_DEFINED

#include "glad/gl.h"
#include <cgltf/cgltf.h>
#include "util.h"

typedef struct {
  int n_vertices;
  int n_triangles;
  float* vertices;
  float* normals;
  float* tangents;
  float* tex_coords;
  unsigned int* indices;
} Mesh;

typedef enum {
  BASE,
  N_TEXTURE_TYPES,
} TextureType;

typedef enum {
  GRAY,
  GRAY_ALPHA,
  RED_GREEN_BLUE,
  RED_GEEN_BLUE_ALPHA,
  N_IMAGE_FORMATS
} ImageFormat;


typedef struct {
  int h;
  int w;
  ImageFormat format;
  unsigned char* data;
} ImageData;

typedef struct {
  ImageData image;
} Texture;

typedef struct {
  Texture textures[N_TEXTURE_TYPES];
} Material;


typedef struct {
  int n_meshes;
  int n_materials;
  Mesh* meshes;
  Material* materials;
} Model;


typedef enum {
  SUCCESS,
  ERROR,
} LoadModelRes;

int format_to_gl_const(ImageFormat format);

LoadModelRes loadModelFromGltfFile(const char* path, Model* model);

void freeMesh(Mesh* mesh);
void freeModel(Model* model);

#endif
