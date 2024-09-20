// system dependencies
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <cglm/cglm.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
// local dependencies
#include "init.h"
// #include "shaders/shader.h"
#include "external/stb_image.h"
// #include "textures/texture.h"
#include "models/model.h"

int H    = 480;
int W    = 640;
char* WT = "Hello World";
vec3 UP  = {0, 1, 0};

// aliases
#define shaderVar glGetUniformLocation

typedef struct {
  float yaw;
  float pitch;
  vec3 pos;
  vec3 front;
  vec3 right;
} Camera;

Camera defaultCamera(void) {
  Camera c = {
      .yaw   = -90.0f,
      .pitch = 0,
      .pos   = {0, 0, 3},
      .front = {0, 0, -1},
  };
  glm_cross(UP, c.front, c.right);
  glm_vec3_normalize(c.right);

  vec3 posFrontSum;
  glm_vec3_add(c.pos, c.front, posFrontSum);
  return c;
}
// /// generates the 'look at' matrix given current camera state
void cameraLookAt(Camera* c, mat4 to) {
  vec3 posFrontSum;
  glm_vec3_add(c->pos, c->front, posFrontSum);
  glm_lookat(c->pos, posFrontSum, UP, to);
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
void moveCameraLeft(Camera* c, float speed) {
  vec3 direction;
  glm_vec3_scale(c->right, speed, direction);
  glm_vec3_add(c->pos, direction, c->pos);
}
void moveCameraRight(Camera* c, float speed) {
  vec3 direction;
  glm_vec3_scale(c->right, speed, direction);
  glm_vec3_sub(c->pos, direction, c->pos);
}
void moveCameraUp(Camera* c, float speed) { c->pos[1] += speed; }
void moveCameraDown(Camera* c, float speed) { c->pos[1] -= speed; }

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
      .camera  = defaultCamera(),
      .cursor  = defaultCursorPos(),
      .frame_t = defaultFrameTime(),
  };
  return state;
}

// === callbacks ===

void onResizeScreen(GLFWwindow* _, int w, int h) {
  glViewport(0, 0, w, h);
  W = w;
  H = h;
}

// === application code ===

// shader paths
const char* CUBE_VSHADER  = "shaders/cube.vert";
const char* CUBE_FSHADER  = "shaders/cube.frag";
const char* LIGHT_VSHADER = "shaders/light.vert";
const char* LIGHT_FSHADER = "shaders/light.frag";

#define closeWindow() glfwSetWindowShouldClose(w, true);
#define K(key) GLFW_KEY_##key
#define keyPressed(key) ((glfwGetKey(w, K(key))) == (GLFW_PRESS))
void handleInput(GLFWwindow* w, GameState* s) {
  float dt = s->frame_t.delta_time;

  if (keyPressed(ESCAPE)) closeWindow();
  if (keyPressed(UP)) moveCameraForward(&s->camera, 2.5f * dt);
  if (keyPressed(DOWN)) moveCameraBackward(&s->camera, 2.5f * dt);
  if (keyPressed(LEFT)) moveCameraLeft(&s->camera, 2.5f * dt);
  if (keyPressed(RIGHT)) moveCameraRight(&s->camera, 2.5f * dt);
  if (keyPressed(PAGE_UP)) moveCameraUp(&s->camera, 2.5f * dt);
  if (keyPressed(PAGE_DOWN)) moveCameraDown(&s->camera, 2.5f * dt);
}

bool firstMouse = true;
void mouseCallback(GLFWwindow* w, double cx, double cy) {
  GameState* s = glfwGetWindowUserPointer(w);
  if (!s) return;

  if (firstMouse) {
    s->cursor.x = cx;
    s->cursor.y = cy;
    firstMouse  = false;
  }

  double dx   = (cx - s->cursor.x);
  double dy   = (s->cursor.y - cy);
  s->cursor.x = cx;
  s->cursor.y = cy;
  s->camera.yaw += dx * 0.2f;
  s->camera.pitch += dy * 0.2f;
  if (s->camera.pitch > 89.0f) s->camera.pitch = 89.0f;
  if (s->camera.pitch < -89.0f) s->camera.pitch = -89.0f;
  s->camera.front[0] =
      cos(glm_rad(s->camera.yaw)) * cos(glm_rad(s->camera.pitch));
  s->camera.front[1] = sin(glm_rad(s->camera.pitch));
  s->camera.front[2] =
      sin(glm_rad(s->camera.yaw)) * cos(glm_rad(s->camera.pitch));
  glm_normalize(s->camera.front);
  glm_cross(UP, s->camera.front, s->camera.right);
  glm_vec3_normalize(s->camera.right);
}

int main(void) {
  // setup default state
  GLFWwindow* w = NULL;
  // 3d models
  Model* earth_model = NULL;
  Model* sun_model   = NULL;

  // game state
  GameState state = defaultGameState();
  // default model view projection matrices
  mat4 m = GLM_MAT4_IDENTITY; // cube model
  mat4 v = GLM_MAT4_IDENTITY; // view
  mat4 p = GLM_MAT4_IDENTITY; // projection

  // === Init glfw and gl context ===
  w = initAndCreateWindow(W, H, WT);
  if (!w) goto clean;

  // === set glfw settings ===
  glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // === gl settings ===
  glEnable(GL_DEPTH_TEST);

  // === register callbacks ==
  // register callback to run  when screen resizes
  glfwSetFramebufferSizeCallback(w, onResizeScreen);
  glfwSetCursorPosCallback(w, mouseCallback);

  // === load 3d models ===
  earth_model = loadModelFromGltfFile("models/earth.glb");
  sun_model   = loadModelFromGltfFile("models/sun.glb");
  if (!earth_model || sun_model) goto clean;

  // === compile and link shaders ==

  // locations for uniform vars

  // === load textures ===

  // === setup gl objects ===

  // == we setup global game state ===

  // we make game state available from everywhere
  glfwSetWindowUserPointer(w, &state);

  // === mvp setup begin ===

  // === Application loop ==
  while (!glfwWindowShouldClose(w)) {
    // === update ===
    float time = glfwGetTime();
    updateFrameTime(&state.frame_t, time); // update frame time
    cameraLookAt(&state.camera, v);        // set v

    handleInput(w, &state); // handle input
    glm_perspective(glm_rad(45.0), (float)W / (float)H, 0.1, 100.0, p);

    // === draw ===
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glfw: swap buffers
    glfwSwapBuffers(w); // swap buffer
    glfwPollEvents();   // poll for more events
  }

  // === Cleanup ===
clean:
  glfwTerminate();
  if (earth_model) freeModel(earth_model);
  if (sun_model) freeModel(sun_model);
  if (!w) return -1; // glfw could not init window
  return 0;
}
