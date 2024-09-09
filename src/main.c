// system dependencies
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <stdbool.h>
#include <cglm/cglm.h>
// local dependencies
#include "init.h"
#include "shaders/utils.h"
#include "shaders/shader.h"
#include "textures/texture.h"
#include "external/stb_image.h"

// === callbacks ===

void onResizeScreen(GLFWwindow* _, int width, int height) {
  glViewport(0, 0, width, height);
}

// === application code ===

// clang-format off
float VERTICES[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   2.0f, 2.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   2.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 2.0f    // top left 
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
    uniform sampler2D texture1;
    uniform sampler2D texture2;
    uniform float mixRatio;
    void main() {
        FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(1.0 - TexCoord.x, TexCoord.y)), mixRatio);
    }    
);
// clang-format on

void processInput(GLFWwindow* window, float* mixRatio, int mixRatioLoc) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    *mixRatio -= 0.01;
    if (*mixRatio > 1.0) *mixRatio = 1.0;
    if (*mixRatio < 0.0) *mixRatio = 0;
    glUniform1f(mixRatioLoc, *mixRatio);
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    *mixRatio += 0.01;
    if (*mixRatio > 1.0) *mixRatio = 1.0;
    if (*mixRatio < 0.0) *mixRatio = 0;
    glUniform1f(mixRatioLoc, *mixRatio);
  }
}

int main(void) {

  // === Init glfw and gl context ===
  GLFWwindow* w = initAndCreateWindow();

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
  GLuint texture1 = createTexture("container.jpg", JPG);
  GLuint texture2 = createTexture("awesomeface.png", PNG);

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
  glBindTexture(GL_TEXTURE_2D, texture1);
  // copy vertex data from ram to vram
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
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

  // activate texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture2);

  // tell opengl which texture unit each shader sampler belongs to
  glUseProgram(shader);
  glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
  glUniform1i(glGetUniformLocation(shader, "texture2"), 1);

  int mixRatioLoc = glGetUniformLocation(shader, "mixRatio");
  float mixRatio = 0.5;
  glUniform1f(mixRatioLoc, mixRatio);

  // == we setup our scaling,rotation,translation matrices
  vec4 vec = {1.0f, 0.0f, 0.0f, 1.0f};

  mat4 transformation = GLM_MAT4_IDENTITY;
  glm_translate(transformation, (vec3){1.0, 1.0, 0.0});
  glm_rotate(transformation, CGLM_PI_2, (vec3){0.0, 0.0, 1.0});
  glm_scale(transformation, (vec3){0.5, 0.5, 0.5});

  vec4 dest;
  glm_mat4_mulv(transformation, vec, dest);

  // === Application loop ==
  while (!glfwWindowShouldClose(w)) {
    processInput(w, &mixRatio, mixRatioLoc);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // activate the program for use
    glUseProgram(shader);

    glBindVertexArray(VAO);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(w); // swap buffer
    glfwPollEvents();   // poll for more events
  }

  // === Cleanup ===
clean:
  glfwTerminate();
  if (!w) return -1; // glfw could not init window
  return 0;
}
