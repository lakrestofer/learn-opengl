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

int H    = 480;
int W    = 640;
char* WT = "Hello World";
vec3 UP  = {0, 1, 0};

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

bool screenResized = false;
void onResizeScreen(GLFWwindow* _, int width, int height) {
  glViewport(0, 0, width, height);
  W             = width;
  H             = height;
  screenResized = true;
}

// === application code ===

// clang-format off
float vertices[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f
};
const int vertex_stride =  (3 * sizeof(float));
const void* vertex_offset = (void*) 0;
// does no transformation on the vertices
const char* CUBE_VSHADER = GLSL(
    layout (location = 0) in vec3 aPos;

    uniform mat4 m;
    uniform mat4 v;
    uniform mat4 p;
   
    void main() {
        gl_Position = p * v * m * vec4(aPos, 1.0);
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
  void main() {
      FragColor = vec4(1.0);
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
  // === Init glfw and gl context ===
  GLFWwindow* w = initAndCreateWindow(W, H, WT);

  // === set glfw settings ===
  glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // === gl settings ===
  glEnable(GL_DEPTH_TEST);

  // === register callbacks ==
  // register callback to run  when screen resizes
  glfwSetFramebufferSizeCallback(w, onResizeScreen);
  glfwSetCursorPosCallback(w, mouseCallback);

  // === compile and link shaders ==
  GLuint cube_vert_shader  = initVShader(CUBE_VSHADER);
  GLuint cube_frag_shader  = initFShader(CUBE_FSHADER);
  GLuint light_frag_shader = initFShader(LIGHT_CUBE_FSHADER);
  if (!shaderIsValid(cube_vert_shader) || !shaderIsValid(cube_frag_shader))
    goto clean;
  GLuint cube_shader  = linkShaders(cube_vert_shader, cube_frag_shader);
  GLuint light_shader = linkShaders(cube_vert_shader, light_frag_shader);
  if (!shaderProgramIsValid(cube_shader)) goto clean;
  glDeleteShader(cube_vert_shader);
  glDeleteShader(cube_frag_shader);
  glDeleteShader(light_frag_shader);

  // locations for uniform vars
  GLuint cube_model_loc  = glGetUniformLocation(cube_shader, "m");
  GLuint cube_view_loc   = glGetUniformLocation(cube_shader, "v");
  GLuint cube_proj_loc   = glGetUniformLocation(cube_shader, "p");
  GLuint cube_object_loc = glGetUniformLocation(cube_shader, "objectColor");
  GLuint cube_light_loc  = glGetUniformLocation(cube_shader, "lightColor");
  GLuint light_model_loc = glGetUniformLocation(light_shader, "m");
  GLuint light_view_loc  = glGetUniformLocation(light_shader, "v");
  GLuint light_proj_loc  = glGetUniformLocation(light_shader, "p");

  // === setup gl objects ===
  // copy vertices to vram
  GLuint VBO; // vertex buffer object
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO); // set VBO as currently active
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // cube
  GLuint cube_VAO; // vertex array object
  glGenVertexArrays(1, &cube_VAO);
  glBindVertexArray(cube_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, vertex_offset);
  glEnableVertexAttribArray(0);
  // light cube
  GLuint light_VAO; // vertex array object
  glGenVertexArrays(1, &light_VAO);
  glBindVertexArray(light_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO); // ram -> vram
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, vertex_offset);
  glEnableVertexAttribArray(0);

  // == we setup global game state ===
  vec3 lightPos   = {1.2, 1, -2.0};
  GameState state = defaultGameState();

  // === mvp setup begin ===
  mat4 cube_m  = GLM_MAT4_IDENTITY; // cube model
  mat4 light_m = GLM_MAT4_IDENTITY; // light cube model
  mat4 v       = GLM_MAT4_IDENTITY; // view
  mat4 p       = GLM_MAT4_IDENTITY; // projection

  // first set mvp for light cube
  glm_translate(light_m, lightPos);                                   // build m
  glm_mat4_scale(light_m, 0.1);                                       // build m
  cameraLookAt(&state.camera, v);                                     // set v
  glm_perspective(glm_rad(45.0), (float)W / (float)H, 0.1, 100.0, p); // set p

  // then set mvp for ordinary cube
  // here we do nothing...

  // we make game state available from everywhere
  glfwSetWindowUserPointer(w, &state);

  // === Application loop ==
  while (!glfwWindowShouldClose(w)) {
    // === update ===
    updateFrameTime(&state.frame_t, glfwGetTime()); // update frame time
    handleInput(w, &state);                         // handle input
    cameraLookAt(&state.camera, v);                 // update view matrix
    glm_perspective(glm_rad(45.0), (float)W / (float)H, 0.1, 100.0, p);

    // === draw ===
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // cube
    glUseProgram(cube_shader);
    glUniformMatrix4fv(cube_model_loc, 1, GL_FALSE, (float*)cube_m);
    glUniformMatrix4fv(cube_view_loc, 1, GL_FALSE, (float*)v);
    glUniformMatrix4fv(cube_proj_loc, 1, GL_FALSE, (float*)p);
    glUniform3fv(cube_object_loc, 1, (vec3){1.0, 0.5, 0.31});
    glUniform3fv(cube_light_loc, 1, (vec3){1.0, 1.0, 1.0});
    glBindVertexArray(cube_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36); // draw it
    // light
    glUseProgram(light_shader);
    glUniformMatrix4fv(light_model_loc, 1, GL_FALSE, (float*)light_m);
    glUniformMatrix4fv(light_view_loc, 1, GL_FALSE, (float*)v);
    glUniformMatrix4fv(light_proj_loc, 1, GL_FALSE, (float*)p);
    glBindVertexArray(light_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36); // draw it

    // glfw: swap buffers
    glfwSwapBuffers(w); // swap buffer
    glfwPollEvents();   // poll for more events
  }

  // === Cleanup ===
clean:
  glfwTerminate();
  if (!w) return -1; // glfw could not init window
  return 0;
}
