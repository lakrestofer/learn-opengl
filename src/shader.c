#include "shaders/shader.h"
#include <stdio.h>

GLuint initShader(GLenum type, const char* shader_src) {
  GLuint obj = glCreateShader(type);
  glShaderSource(obj, 1, &shader_src, NULL);
  glCompileShader(obj);
  return obj;
}
bool shaderIsValid(GLuint shader) {
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    printf("ERROR::SHADER::COMPILATION_FAILED\n %s", infoLog);
    return false;
  }
  return true;
}

GLuint linkShaders(GLuint* shaders, int n) {
  GLuint program = glCreateProgram();
  for (int i = 0; i < n; i++) glAttachShader(program, shaders[i]);
  glLinkProgram(program);
  return program;
}

bool shaderProgramIsValid(GLuint program) {
  int success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    printf("ERROR::SHADER::LINKING_FAILED\n %s", infoLog);
    return false;
  }
  return true;
}
