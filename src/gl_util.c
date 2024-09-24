
#include "gl_util.h"
#include <stdlib.h>

#define bindArrayBuffer(vbo) glBindBuffer(GL_ARRAY_BUFFER, (vbo))
#define arrayBufferData(size, ptr)                                             \
  glBufferData(GL_ARRAY_BUFFER, size, ptr, GL_STATIC_DRAW)

void genGlIds(GlIdentifier* wrappers, int n) {
  // I. We create all VAOs, VBOs, and EBO
  GLuint* VAO     = malloc(n * sizeof(GLuint));
  VBOBuffers* VBO = malloc(n * sizeof(VBOBuffers));
  GLuint* EBO     = malloc(n * sizeof(GLuint));
  glGenVertexArrays(n, VAO);
  glGenBuffers(n * N_BUFFER_TYPES, (GLuint*)VBO);
  glGenBuffers(n, EBO);
  wrappers->vao = VAO;
  wrappers->vbo = VBO;
  wrappers->ebo = EBO;

  // II. then we bind them to each other
  for (int i = 0; i < n; i++) {
    glBindVertexArray(VAO[i]);
    GLuint* buffers = (GLuint*)&VBO[i];
    // bind vertex, normal, tangend texcoord buffers
    for (int j = 0; j < N_BUFFER_TYPES; j++) {
      glBindBuffer(GL_ARRAY_BUFFER, buffers[j]);
      glVertexAttribPointer(
          j,                 // id
          COMPONENT_SIZE[j], // number of floats per component
          GL_FLOAT,
          GL_FALSE,
          COMPONENT_SIZE[j] * sizeof(float), // stride
          (void*)0                           // offset
      );
    }
    // and bind index buffer
    glBindBuffer(GL_ARRAY_BUFFER, EBO[i]);
  }
}
// copy mesh contents to vram
void syncBuffers(Mesh* meshes, GlIdentifier* ids, int n_meshes) {
  for (int i = 0; i < n_meshes; i++) {
    GLuint vao   = ids->vao[i];
    GLuint* vbos = (GLuint*)&ids->vbo[i]; // struct is packed
    GLuint ebo   = ids->ebo[i];

    Mesh m           = meshes[i];
    float* buffer[4] = {m.vertices, m.normals, m.tangents, m.tex_coords};
    int n            = m.n_vertices;

    glBindVertexArray(vao);
    for (int j = 0; j < N_BUFFER_TYPES; j++) {
      if (buffer[i]) {
        glBindBuffer(GL_ARRAY_BUFFER, vbos[j]);
        glBufferData(
            GL_ARRAY_BUFFER,
            n * COMPONENT_SIZE[i] * sizeof(float),
            buffer[i],
            GL_STATIC_DRAW
        );
      }
    }
    if (m.indices) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(
          GL_ELEMENT_ARRAY_BUFFER,
          m.n_triangles * 3 * sizeof(float),
          m.indices,
          GL_STATIC_DRAW
      );
    }
  }
}
