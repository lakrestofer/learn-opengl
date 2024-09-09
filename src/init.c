
#include "init.h"
#include <stdio.h>

#define GLMA(v) GLAD_VERSION_MAJOR(v)
#define GLMI(v) GLAD_VERSION_MINOR(v)

/// prints the open gl version
void printGlVer(int v) { printf("Loaded OpenGL %d.%d\n", GLMA(v), GLMI(v)); }

GLFWwindow* initAndCreateWindow(void) {
  if (!glfwInit()) return NULL;

  // set some parameters
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(W, H, WT, NULL, NULL);
  if (!window) return NULL;

  // set the context to current context
  glfwMakeContextCurrent(window);

  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) return NULL;

  glViewport(0, 0, W, H);

  printGlVer(version);

  return window;
}
