
#include "gl_wrapper.h"
#include <stdlib.h>

void initObjects(Model* models, GlObjects* wrappers, int n_models) {
  int total_n_meshes = 0;
  for (int i = 0; i < n_models; i++) {
    Model* m = &models[i];
    total_n_meshes += m->n_meshes;

    // one vao per mesh
    GLuint* VAO        = malloc(total_n_meshes * sizeof(GLuint));
    short* vbo_map     = malloc(total_n_meshes * sizeof(short));
    short* vbo_len_map = malloc(total_n_meshes * sizeof(short));
    GLuint* EBO        = calloc(total_n_meshes, sizeof(GLuint));
    glGenVertexArrays(total_n_meshes, VAO);

    // for every model
    for (int mi = 0; mi < total_n_meshes; mi++) {}

    free(VAO);
  }
