// system dependencies
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <stdio.h>
#include <stdbool.h>
// local dependencies
#include "app/core.h"
#include "shaders/utils.h"
#include "external/stb_image.h"

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

// clang-format off
float VERTICES[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
};
unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
}; 
// does no transformation on the vertices
const char *VERTEX_SHADER_SRC = GLSL(
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    layout (location = 2) in vec2 aTexCoord;
    out vec3 ourColor;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor;
        TexCoord = aTexCoord;
    }
);
// colors each pixel within the triangle red
const char *FRAGMENT_SHADER_SRC = GLSL(
    out vec4 FragColor;
    in vec3 ourColor;
    in vec2 TexCoord;
    uniform sampler2D ourTexture;
    void main() {
        // FragColor = texture(ourTexture, TexCoord);
        FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);  
    }
);
// clang-format on

void setWindowContext(void) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

int main(void) {
  /// window
  GLFWwindow* w = NULL;
  /// version
  int v = 0;
  Vertices vertices = {
      .vertices = VERTICES,
      .size = sizeof(VERTICES),
  };

  // === Init glfw and gl context ===
  if (!glfwInit()) return -1;
  setWindowContext();
  w = glfwCreateWindow(W, H, WT, NULL, NULL); // create window and context
  if (!w) goto clean;                         // if unsuccessful goto cleanup
  glfwMakeContextCurrent(w);          // set the context to current context
  v = gladLoadGL(glfwGetProcAddress); // setup opengl function pointers
  if (v == 0) goto clean;             // if unsuccsesfull goto cleanup
  glViewport(0, 0, W, H);             // set the viewport
  printGlVer(v);                      // print version

  // === register callbacks ==
  glfwSetFramebufferSizeCallback(w, onResizeScreen); // register callback to run
                                                     // when screen resizes
  // === compile and link shaders ==
  GLuint shaders[] = {
      initShader(GL_VERTEX_SHADER, VERTEX_SHADER_SRC),
      initShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SRC),
  };
  if (!shaderIsValid(shaders[0]) || !shaderIsValid(shaders[1])) goto clean;
  GLuint shader = linkShaders(shaders, 2);
  if (!shaderProgramIsValid(shader)) goto clean;
  for (int i = 0; i < 2; i++) glDeleteShader(shaders[i]);

  // === load textures ===
  int iw, ih, nbrChnls;
  unsigned char* data = stbi_load("container.jpg", &iw, &ih, &nbrChnls, 0);
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGB, iw, ih, 0, GL_RGB, GL_UNSIGNED_BYTE, data
  );
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);

  // === setup gl objects ===
  GLuint VAO;       // vertex array object
  GLuint VBO;       // vertex buffer object
  unsigned int EBO; // element buffer object
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  // setup how to interpret the buffer data
  glBindVertexArray(VAO);
  // bind texture
  glBindTexture(GL_TEXTURE_2D, texture);
  // copy vertex data from ram to vram
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
      GL_ARRAY_BUFFER, vertices.size, vertices.vertices, GL_STATIC_DRAW
  );
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW
  );
  // set vertex attribute pointers
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))
  );
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))
  );
  glEnableVertexAttribArray(2);

  // === Application loop ==
  runApp(w, VAO, shader);

  // === Cleanup ===
clean:
  glfwTerminate();
  if (!w) return -1; // glfw could not init window
  if (!v) return -1; // glad could not init opengl
  return 0;
}
