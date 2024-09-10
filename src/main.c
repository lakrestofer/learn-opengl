// system dependencies
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <cglm/cam.h>
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <stdbool.h>
// local dependencies
#include "init.h"
#include "shaders/utils.h"
#include "shaders/shader.h"
#include "textures/texture.h"
#include "external/stb_image.h"

#define H 480
#define W 640
#define WT "Hello World"

typedef struct {
  vec3 pos;
  vec3 up;
  vec3 front;
  vec3 left;
  vec4 right;
} Camera;

// === callbacks ===

void onResizeScreen(GLFWwindow* _, int width, int height) {
  glViewport(0, 0, width, height);
}

// === application code ===

// clang-format off
vec3 cubePositions[] = {
    { 0.0f,  0.0f,  0.0f},
    { 2.0f,  5.0f, -15.0f},
    {-1.5f, -2.2f, -2.5f}, 
    {-3.8f, -2.0f, -12.3f}, 
    { 2.4f, -0.4f, -3.5f}, 
    {-1.7f,  3.0f, -7.5f}, 
    { 1.3f, -2.0f, -2.5f}, 
    { 1.5f,  2.0f, -2.5f},
    { 1.5f,  0.2f, -1.5f},
    {-1.3f,  1.0f, -1.5f} 
};
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
// does no transformation on the vertices
const char *VERTEX_SHADER_SRC = GLSL(
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
);
// colors each pixel within the triangle red
const char *FRAGMENT_SHADER_SRC = GLSL(
    out vec4 FragColor;
    in vec2 TexCoord;
    uniform sampler2D texture1;
    void main() {
        FragColor = texture(texture1, TexCoord);
    }    
);
// clang-format on

#define CAMERA_SPEED 0.05f

void processInput(GLFWwindow* window, Camera* camera) {
  vec3 diff;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    glm_vec3_scale(camera->front, CAMERA_SPEED, diff);
    glm_vec3_add(camera->pos, diff, camera->pos);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    glm_vec3_scale(camera->front, -CAMERA_SPEED, diff);
    glm_vec3_add(camera->pos, diff, camera->pos);
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    glm_vec3_scale(camera->left, CAMERA_SPEED, diff);
    glm_vec3_add(camera->pos, diff, camera->pos);
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    glm_vec3_scale(camera->right, CAMERA_SPEED, diff);
    glm_vec3_add(camera->pos, diff, camera->pos);
  }
}

int main(void) {

  // === Init glfw and gl context ===
  GLFWwindow* w = initAndCreateWindow();

  // === register callbacks ==
  // register callback to run  when screen resizes
  glfwSetFramebufferSizeCallback(w, onResizeScreen);

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

  // === setup gl objects ===
  GLuint VAO; // vertex array object
  GLuint VBO; // vertex buffer object
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  // enable some gl settings
  glEnable(GL_DEPTH_TEST);

  // setup how to interpret the buffer data
  glBindVertexArray(VAO);
  // bind texture
  glBindTexture(GL_TEXTURE_2D, texture1);
  // copy vertex data from ram to vram
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // set vertex attribute pointers
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
      1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))
  );
  glEnableVertexAttribArray(1);

  // activate texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture1);

  // tell opengl which texture unit each shader sampler belongs to
  glUseProgram(shader);
  GLuint shaderTextureLoc = glGetUniformLocation(shader, "texture1");
  glUniform1i(shaderTextureLoc, 0);

  // == we setup our scaling,rotation,translation matrices

  // camera
  Camera camera = {.pos = {0, 0, 3}, .front = {0, 0, -1}, .up = {0, 1, 0}};
  glm_cross(camera.front, camera.up, camera.right);
  glm_vec3_normalize(camera.right);
  glm_vec3_negate_to(camera.right, camera.left);

  // model view projection matrices
  mat4 model = GLM_MAT4_IDENTITY;
  glm_rotate(model, glm_rad(-55.0f), (vec3){1, 0, 0});
  mat4 view;
  vec3 cameraPos2;
  glm_vec3_add(camera.pos, camera.front, cameraPos2);
  glm_lookat(camera.pos, cameraPos2, camera.up, view);

  mat4 projection = GLM_MAT4_IDENTITY;
  glm_perspective(glm_rad(45.0), W / H, 0.1, 100.0, projection);

  GLuint modelLoc = glGetUniformLocation(shader, "model");
  GLuint viewLoc = glGetUniformLocation(shader, "view");
  GLuint projectionLoc = glGetUniformLocation(shader, "projection");

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)view);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (float*)projection);

  // === Application loop ==
  while (!glfwWindowShouldClose(w)) {
    processInput(w, &camera);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate the program for use
    glUseProgram(shader);

    glBindVertexArray(VAO);

    mat4 localView;
    glm_vec3_add(camera.pos, camera.front, cameraPos2);
    glm_lookat(camera.pos, cameraPos2, camera.up, localView);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)localView);

    for (int i = 0; i < 10; i++) {
      mat4 localModel = GLM_MAT4_IDENTITY;
      glm_translate(localModel, cubePositions[i]);
      float angle = 20.0f * i;
      glm_rotate(model, glm_rad(angle), (vec3){1.0f, 0.3f, 0.5f});
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)localModel);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glfwSwapBuffers(w); // swap buffer
    glfwPollEvents();   // poll for more events
  }

  // === Cleanup ===
clean:
  glfwTerminate();
  if (!w) return -1; // glfw could not init window
  return 0;
}
