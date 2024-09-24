#ifndef SHADER_HEADER
#define SHADER_HEADER

#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef const char* ShaderSrc;
typedef GLuint Shader;
typedef struct {
  GLint model;
  GLint view;
  GLint projection;
} ShaderVars;

GLuint loadShader(const char* v_path, const char* f_path);
ShaderVars loadShaderVars(GLuint shader);

#endif
