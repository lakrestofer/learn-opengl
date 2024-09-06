#ifndef APP_CORE_HEADER
#define APP_CORE_HEADER

#include "glad/gl.h"
#include "shaders/shader.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct {
  const float* vertices;
  const unsigned int* indices;
  GLsizeiptr size;
  GLsizeiptr iSize;
} Vertices;


/// start the application loop
int runApp(GLFWwindow*, GLuint VAO, Shader);

#endif
