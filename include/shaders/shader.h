
#ifndef SHADER_HEADER
#define SHADER_HEADER

#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef const char* ShaderSrc;
typedef GLuint Shader;


GLuint initShader(GLenum type, const char* shader_src);
bool shaderIsValid(GLuint shader);
GLuint linkShaders(GLuint* shaders, int n);
bool shaderProgramIsValid(GLuint program);


#endif
