
#ifndef SHADER_HEADER
#define SHADER_HEADER

#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef const char* ShaderSrc;
typedef GLuint Shader;

#define initVShader(src) initShader(GL_VERTEX_SHADER, src)
#define initFShader(src) initShader(GL_FRAGMENT_SHADER, src)
GLuint initShader(GLenum type, const char* shader_src);
bool shaderIsValid(GLuint shader);
GLuint linkShaders(GLuint v_shader, GLuint f_shader);
bool shaderProgramIsValid(GLuint program);


#endif
