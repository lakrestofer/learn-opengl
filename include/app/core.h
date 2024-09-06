#ifndef APP_CORE_HEADER
#define APP_CORE_HEADER

#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct {
  const float* vertices;
  const unsigned int* indices;
  GLsizeiptr size;
  GLsizeiptr iSize;
} Vertices;

typedef const char* ShaderSrc;

/// start the application loop
int runApp(GLFWwindow*, Vertices, ShaderSrc, ShaderSrc);

#endif
