// system dependencies
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <stdio.h>
#include <stdbool.h>
// local dependencies
#include "app/core.h"
#include "shaders/utils.h"

#define H 480
#define W 640
#define WT "Hello World"

#define GLMA(v) GLAD_VERSION_MAJOR(v)
#define GLMI(v) GLAD_VERSION_MINOR(v)

typedef enum { UNKNOWN, GLFW_CREATE_WINDOW_ERROR, GLAD_INIT_ERROR } ErrorCode;

/// prints the open gl version
void printGlVer(int v) { printf("Loaded OpenGL %d.%d\n", GLMA(v), GLMI(v)); }

// === callbacks ===

void onResizeScreen(GLFWwindow* _, int width, int height) {
  glViewport(0, 0, width, height);
}

// === application code ===
float VERTICES[] = {
    0.5f,
    0.5f,
    0.0f, // top right
    0.5f,
    -0.5f,
    0.0f, // bottom right
    -0.5f,
    -0.5f,
    0.0f, // bottom left
    -0.5f,
    0.5f,
    0.0f // top left
};
unsigned int INDICES[] = {
    // note that we start from 0!
    0,
    1,
    3,
    1,
    2,
    3
};

// clang-format off
// does no transformation on the vertices
const char *VERTEX_SHADER_SRC = GLSL(
    layout(location = 0) in vec3 aPos;
    void main() {
      gl_Position = vec4(aPos, 1.0);
    }
);
// colors each pixel within the triangle red
const char *FRAGMENT_SHADER_SRC = GLSL(
  out vec4 FragColor;
  uniform vec4 ourColor;
  void main() {
    FragColor = ourColor;
  }
);
// clang-format on

int main(void) {
  /// window
  GLFWwindow* w = NULL;
  /// version
  int v = 0;
  Vertices vertices = {
      .vertices = VERTICES,
      .indices = INDICES,
      .size = sizeof(VERTICES),
      .iSize = sizeof(INDICES)
  };

  // === Init begin ===
  if (!glfwInit()) return -1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  w = glfwCreateWindow(W, H, WT, NULL, NULL); // create window and context
  if (!w) goto clean;                         // if unsuccessful goto cleanup
  glfwMakeContextCurrent(w);          // set the context to current context
  v = gladLoadGL(glfwGetProcAddress); // setup opengl function pointers
  if (v == 0) goto clean;             // if unsuccsesfull goto cleanup
  glViewport(0, 0, W, H);             // set the viewport
  printGlVer(v);                      // print version

  glfwSetFramebufferSizeCallback(w, onResizeScreen); // register callback to run
                                                     // when screen resizes

  // === Application loop ==
  runApp(w, vertices, VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);

  // === Cleanup ===
clean:
  glfwTerminate();
  if (!w) return -1; // glfw could not init window
  if (!v) return -1; // glad could not init opengl
  return 0;
}
