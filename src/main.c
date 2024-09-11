// system dependencies
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <cglm/cam.h>
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <stdbool.h>
#include <string.h>
// local dependencies
#include "init.h"
#include "shaders/utils.h"
#include "shaders/shader.h"
#include "textures/texture.h"
#include "external/stb_image.h"

#define H 480
#define W 640
#define WT "Hello World"

// some aliases for functions to make them shorter
#define onScreenResize glfwSetFramebufferSizeCallback
#define onCursorEvent glfwSetCursorPosCallback

// and aliases for constants
#define VSHADER GL_VERTEX_SHADER
#define FSHADER GL_FRAGMENT_SHADER

typedef struct {
  float yaw;
  float pitch;
  vec3 pos;
  vec3 front;
} Camera;

Camera defaultCamera(void) {
  Camera c = {
      .yaw = -90.0f,
      .pitch = 0,
      .pos = {0, 0, 3},
      .front = {0, 0, -1},
  };
  return c;
}
/// generates the 'look at' matrix given current camera state
void cameraLookAt(Camera* c, mat4 to) {
  vec3 posFrontSum;
  glm_vec3_add(c->pos, c->front, posFrontSum);
  glm_lookat(c->pos, posFrontSum, (vec3){0, 1, 0}, to);
}

void moveCameraForward(Camera* c, float speed) {
  vec3 direction;
  glm_vec3_scale(c->front, speed, direction);
  glm_vec3_add(c->pos, direction, c->pos);
}
void moveCameraBackward(Camera* c, float speed) {
  vec3 direction;
  glm_vec3_scale(c->front, speed, direction);
  glm_vec3_sub(c->pos, direction, c->pos);
}

typedef struct {
  float x;
  float y;
} Cursor;

Cursor defaultCursorPos(void) {
  Cursor p = {.x = W / 2, .y = H / 2};
  return p;
}

typedef struct {
  float delta_time;
  float last_frame;
} FrameTime;

FrameTime defaultFrameTime(void) {
  FrameTime t = {
      .delta_time = 0.0f, // Time between current frame and last frame
      .last_frame = 0.0f, // Time of last frame
  };
  return t;
}
void updateFrameTime(FrameTime* time, float now) {
  time->delta_time = now - time->last_frame;
  time->last_frame = now;
}

typedef struct {
  Camera camera;
  Cursor cursor;
  FrameTime frame_t;
} GameState;

GameState defaultGameState(void) {
  GameState state = {
      .camera = defaultCamera(),
      .cursor = defaultCursorPos(),
      .frame_t = defaultFrameTime(),
  };
  return state;
}

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
const char *VSS = GLSL(
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
const char *FSS = GLSL(
    out vec4 FragColor;
    in vec2 TexCoord;
    uniform sampler2D texture1;
    void main() {
        FragColor = texture(texture1, TexCoord);
    }    
);
// clang-format on

#define closeWindow() glfwSetWindowShouldClose(w, true);
#define K(key) GLFW_KEY_##key
#define keyPressed(key) ((glfwGetKey(w, K(key))) == (GLFW_PRESS))
void handleInput(GLFWwindow* w, GameState* s) {
  float dt = s->frame_t.delta_time;

  if (keyPressed(ESCAPE)) closeWindow();
  if (keyPressed(UP)) moveCameraForward(&s->camera, 2.5f * dt);
  if (keyPressed(DOWN)) moveCameraBackward(&s->camera, 2.5f * dt);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* w, double cx, double cy) {
  GameState* s = glfwGetWindowUserPointer(w);
  if (!s) return;

  if (firstMouse) {
    s->cursor.x = cx;
    s->cursor.y = cy;
    firstMouse = false;
  }

  double dx = (cx - s->cursor.x);
  double dy = (s->cursor.y - cy);
  s->cursor.x = cx;
  s->cursor.y = cy;
  s->camera.yaw += dx * 0.2f;
  s->camera.pitch += dy * 0.2f;
  if (s->camera.pitch > 89.0f) s->camera.pitch = 89.0f;
  if (s->camera.pitch < -89.0f) s->camera.pitch = -89.0f;
  vec3 dir = {
      cos(glm_rad(s->camera.yaw)) * cos(glm_rad(s->camera.pitch)),
      sin(glm_rad(s->camera.pitch)),
      sin(glm_rad(s->camera.yaw)) * cos(glm_rad(s->camera.pitch)),
  };
  glm_normalize(dir);
  memcpy(s->camera.front, dir, sizeof(vec3));
}

int main(void) {
  // === Init glfw and gl context ===
  GLFWwindow* w = initAndCreateWindow();

  // === set glfw settings ===
  glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // === register callbacks ==
  // register callback to run  when screen resizes
  onScreenResize(w, onResizeScreen);
  onCursorEvent(w, mouse_callback);

  // === compile and link shaders ==
  GLuint shaders[2] = {initShader(VSHADER, VSS), initShader(FSHADER, FSS)};
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
  glBindTexture(GL_TEXTURE_2D, texture1);
  glBindBuffer(GL_ARRAY_BUFFER, VBO); // ram -> vram
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
  glUniform1i(glGetUniformLocation(shader, "texture1"), 0);

  // == we setup global game state ===
  GameState state = defaultGameState();

  // model view projection matrices
  mat4 model = GLM_MAT4_IDENTITY;
  mat4 view = GLM_MAT4_IDENTITY;
  mat4 projection = GLM_MAT4_IDENTITY;
  glm_rotate(model, glm_rad(-55.0f), (vec3){1, 0, 0});
  cameraLookAt(&state.camera, view);
  glm_perspective(glm_rad(45.0), W / H, 0.1, 100.0, projection);

  GLuint modelLoc = glGetUniformLocation(shader, "model");
  GLuint viewLoc = glGetUniformLocation(shader, "view");
  GLuint projectionLoc = glGetUniformLocation(shader, "projection");

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)model);
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)view);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (float*)projection);

  // we make game state available from everywhere
  glfwSetWindowUserPointer(w, &state);

  // === Application loop ==
  while (!glfwWindowShouldClose(w)) {
    // update
    updateFrameTime(&state.frame_t, glfwGetTime());

    // handle input
    handleInput(w, &state);

    // draw
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate the program for use
    glUseProgram(shader);

    glBindVertexArray(VAO);
    cameraLookAt(&state.camera, view); // update view matrix from camera
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)view);

    for (int i = 0; i < 10; i++) {
      float angle = 20.0f * i;
      mat4 localModel = GLM_MAT4_IDENTITY;
      glm_translate(localModel, cubePositions[i]);
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
