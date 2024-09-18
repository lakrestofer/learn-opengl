#include "shaders/shader.h"
#include <stdio.h>
#include <stdlib.h>

GLuint initShader(GLenum type, const char* shader_src) {
  // read file
  FILE* f = fopen(shader_src, "r"); // open file
  if (!f) return 0;
  fseek(f, 0, SEEK_END);
  int length = ftell(f); // find length of file
  fseek(f, 0, SEEK_SET);
  char* buffer = malloc(length + 1); // allocate memory for file
  if (!buffer) return 0;
  fread(buffer, 1, length, f); // copy file
  buffer[length] = 0;          // make sure to zero terminate it
  fclose(f);                   // close file object

  GLuint obj = glCreateShader(type);
  glShaderSource(obj, 1, &buffer, NULL);
  glCompileShader(obj);
  free(buffer); // free the buffer
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

GLuint linkShaders(GLuint v_shader, GLuint f_shader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, v_shader);
  glAttachShader(program, f_shader);
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
