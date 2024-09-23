
#include "gl_wrapper.h"
#include <stdlib.h>

#define bindArrayBuffer(vbo) glBindBuffer(GL_ARRAY_BUFFER, (vbo))
#define arrayBufferData(size, ptr)                                             \
  glBufferData(GL_ARRAY_BUFFER, size, ptr, GL_STATIC_DRAW)

void initMeshWrappers(ModelSlice models, GlMeshWrapperSlice wrappers) {
  int n_meshes  = wrappers.len;
  int n_buffers = n_meshes * BUFFER_CONTENT_MAX;

  // I. We allocate space for all VAOs, VBOs, and EBO
  // one VAO, and EBO for every mesh, 4 VBOs
  GLuint* VAO = calloc(n_meshes, sizeof(GLuint));
  GLuint* VBO = calloc(n_buffers, sizeof(GLuint));
  GLuint* EBO = calloc(n_meshes, sizeof(GLuint));
  glGenVertexArrays(n_meshes, VAO);
  glGenBuffers(n_buffers, VBO);
  glGenBuffers(n_meshes, EBO);

  // II. set values
  int wi = 0; // current wrapper being initialized
  for (int i = 0; i < models.len; i++) {
    for (int j = 0; j < models.ptr[i].n_meshes; j++, wi++) {
      Mesh* m          = &(models.ptr[i].meshes[j]);
      GlMeshWrapper* w = &(wrappers.ptr[wi]);
      w->mesh          = m;
      w->vao           = VAO[wi];
      w->vbo[VERTICES] = (m->vertices) ? VBO[wi * BUFFER_CONTENT_MAX + 0] : 0;
      w->vbo[NORMALS]  = (m->normals) ? VBO[wi * BUFFER_CONTENT_MAX + 1] : 0;
      w->vbo[TANGENT]  = (m->tangents) ? VBO[wi * BUFFER_CONTENT_MAX + 2] : 0;
      w->vbo[TEXCOORD] = (m->tex_coords) ? VBO[wi * BUFFER_CONTENT_MAX + 3] : 0;
      w->ebo           = (m->indices) ? EBO[wi] : 0;
    }
  }

  // III. copy mesh values to vram
  // TODO iterate over vbo ids and delete unused buffers
  for (int i = 0; i < n_meshes; i++) {
    GlMeshWrapper* w = &(wrappers.ptr[i]);
    Mesh* m          = w->mesh;
    glBindVertexArray(w->vao);

    float* b[4] = {m->vertices, m->normals, m->tangents, m->tex_coords};
    int n       = m->n_vertices;
    char s[4]   = {3, 3, 4, 2};

    for (int j = 0; j < BUFFER_CONTENT_MAX; j++) {
      if (b[i]) {
        glBindBuffer(GL_ARRAY_BUFFER, w->vbo[i]);
        glBufferData(
            GL_ARRAY_BUFFER, n * s[i] * sizeof(float), b[i], GL_STATIC_DRAW
        );
        glVertexAttribPointer(
            i, s[i], GL_FLOAT, GL_FALSE, s[i] * sizeof(float), (void*)0
        );
        glEnableVertexAttribArray(i);
      }
    }
  }

  free(VAO);
  free(VBO);
  free(EBO);
}
