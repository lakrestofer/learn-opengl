#include "app/core.h"
#include <GL/glext.h>
#include <stdio.h>

#define VERTICES {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f}

#define GLSL(src) "#version 330 core\n" #src

// clang-format off
const char *TRIANGLE_VERTEX_SHADER_SRC = GLSL(
    layout(location = 0) in vec3 aPos;
    void main() {
      gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
);
// clang-format on

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

typedef struct {
  unsigned int vbo;
} GlBuffers;

// void initVertices(GLFWwindow *w) {}

int run_app(GLFWwindow *w) {
  unsigned int VBO;
  float vs[] = VERTICES;

  // generate a buffer object and write id to VBO
  // bind buffer object to the GL_ARRAY_TARGET
  // copy data from cpu ram to vram
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vs), vs, GL_STATIC_DRAW);

  // generate a vertex shader object
  unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vShader, 1, &TRIANGLE_VERTEX_SHADER_SRC, NULL);
  glCompileShader(vShader);

  int success;
  char infoLog[512];
  glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(vShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s", infoLog);
    return 1;
  }

  while (!glfwWindowShouldClose(w)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // set the clear color
    glClear(GL_COLOR_BUFFER_BIT);         // clear buffer

    processInput(w);                      // handle any events

    glfwSwapBuffers(w);                   // swap buffer
    glfwPollEvents();                     // poll for more events
  }
  return 0;
}
