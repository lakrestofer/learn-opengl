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
#include "external/stb_image.h"

#define H 480
#define W 640
#define WT "Hello World"

// some aliases for functions to make them shorter
#define onScreenResize glfwSetFramebufferSizeCallback
#define onCursorEvent glfwSetCursorPosCallback

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
const int vertex_stride =  (5 * sizeof(float));
const void* vertex_offset = (void*) 0;
// does no transformation on the vertices
const char* CUBE_VSHADER = GLSL(
    layout (location = 0) in vec3 aPos;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
   
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
);
// colors each pixel within the triangle red
const char* CUBE_FSHADER = GLSL(
  out vec4 FragColor;
  uniform vec3 objectColor;
  uniform vec3 lightColor;
  void main() {
      FragColor = vec4(lightColor * objectColor, 1.0);
  }
);
const char* LIGHT_CUBE_FSHADER = GLSL(
  out vec4 FragColor;
  uniform vec3 objectColor;
  uniform vec3 lightColor;
  void main() {
      FragColor = vec4(lightColor * objectColor, 1.0);
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
  GLuint cvs = initVShader(CUBE_VSHADER);
  GLuint cfs = initFShader(CUBE_FSHADER);
  GLuint lcfs = initFShader(LIGHT_CUBE_FSHADER);
  if (!shaderIsValid(cvs) || !shaderIsValid(cfs)) goto clean;
  GLuint cube_shader = linkShaders(cvs, cfs);
  GLuint light_cube_shader = linkShaders(cvs, lcfs);
  if (!shaderProgramIsValid(cube_shader)) goto clean;
  glDeleteShader(cvs);
  glDeleteShader(cfs);
  glDeleteShader(lcfs);

  // === load textures ===

  // === setup gl objects ===
  GLuint light_VAO; // vertex array object
  GLuint VAO;       // vertex array object
  GLuint VBO;       // vertex buffer object
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &light_VAO);

  // enable some gl settings
  glEnable(GL_DEPTH_TEST);

  // cube
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO); // ram -> vram
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, vertex_offset);
  glEnableVertexAttribArray(0);
  // cube
  glBindVertexArray(light_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO); // ram -> vram
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, vertex_offset);
  glEnableVertexAttribArray(0);

  // activate texture

  // tell opengl which texture unit each shader sampler belongs to
  glUseProgram(cube_shader);

  // == we setup global game state ===
  GameState state = defaultGameState();

  // model view projection matrices
  mat4 m = GLM_MAT4_IDENTITY;
  mat4 v = GLM_MAT4_IDENTITY;
  mat4 p = GLM_MAT4_IDENTITY;
  glm_rotate(m, glm_rad(-55.0f), (vec3){1, 0, 0});
  cameraLookAt(&state.camera, v);
  glm_perspective(glm_rad(45.0), W / H, 0.1, 100.0, p);

  GLuint ml = glGetUniformLocation(cube_shader, "model");
  GLuint vl = glGetUniformLocation(cube_shader, "view");
  GLuint pl = glGetUniformLocation(cube_shader, "projection");
  GLuint ol = glGetUniformLocation(cube_shader, "objectColor");
  GLuint ll = glGetUniformLocation(cube_shader, "lightColor");

  glUniformMatrix4fv(ml, 1, GL_FALSE, (float*)m);
  glUniformMatrix4fv(vl, 1, GL_FALSE, (float*)v);
  glUniformMatrix4fv(pl, 1, GL_FALSE, (float*)p);
  glUniform3fv(ol, 1, (vec3){1.0, 0.5, 0.31});
  glUniform3fv(ll, 1, (vec3){1.0, 1.0, 1.0});

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
    glUseProgram(cube_shader);

    glBindVertexArray(VAO);
    cameraLookAt(&state.camera, v);
    glUniformMatrix4fv(vl, 1, GL_FALSE, (float*)v);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glfwSwapBuffers(w); // swap buffer
    glfwPollEvents();   // poll for more events
  }

  // === Cleanup ===
clean:
  glfwTerminate();
  if (!w) return -1; // glfw could not init window
  return 0;
}
