#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <stdio.h>

#define H 480
#define W 640
#define WT "Hello World"

#define GLMA(v) GLAD_VERSION_MAJOR(v)
#define GLMI(v) GLAD_VERSION_MINOR(v)

typedef enum { UNKNOWN, GLFW_CREATE_WINDOW_ERROR, GLAD_INIT_ERROR } ErrorCode;

/// prints the open gl version
void printGlVer(int v) { printf("Loaded OpenGL %d.%d\n", GLMA(v), GLMI(v)); }

int runApp(GLFWwindow *w) {
  while (!glfwWindowShouldClose(w)) {
    glClear(GL_COLOR_BUFFER_BIT); // clear buffer
    glfwSwapBuffers(w);           // swap
    glfwPollEvents();             // poll and process events
  }
  return 0;
}

int main(void) {
  /// window
  GLFWwindow *w = NULL;
  /// version
  int v = 0;

  // === Init begin ===
  if (!glfwInit()) return -1;
  w = glfwCreateWindow(W, H, WT, NULL, NULL); // create window and context
  if (!w) goto clean;                         // if unsuccessful goto cleanup
  glfwMakeContextCurrent(w);          // set the context to current context
  v = gladLoadGL(glfwGetProcAddress); // setup opengl function pointers
  if (v == 0) goto clean;             // if unsuccsesfull goto cleanup
  printGlVer(v);                      // print version

  // === Application loop ==
  runApp(w);

  // === cleanup loop ==
clean:
  glfwTerminate();
  if (!w) return -1; // glfw could not init window
  if (!v) return -1; // glad could not init opengl
  return 0;
}
