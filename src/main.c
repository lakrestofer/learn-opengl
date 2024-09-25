// system dependencies
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <cglm/cglm.h>
#include <stdbool.h>
#include <sys/stat.h>
// local dependencies
#include "init.h"
#include "shaders/shader.h"
#include "external/stb_image.h"
// #include "textures/texture.h"
#include "models/model.h"
#include "gl_util.h"
#include "game_state.h"

int H    = 480;
int W    = 640;
char* WT = "Hello World";
vec3 UP  = {0, 1, 0};

// aliases
#define shaderVar glGetUniformLocation

// === callbacks ===

void onResizeScreen(GLFWwindow* _, int w, int h) {
  glViewport(0, 0, w, h);
  W = w;
  H = h;
}

typedef struct {
  GlIdentifier ids;
  GLuint shader;
  GLuint texture;
} RenderModel;

// === application code ===

// shader paths
const char* SUN_VERT_SRC = "shaders/sun.vert";
const char* SUN_FRAG_SRC = "shaders/sun.frag";

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
  // printf("pitch: %f, yaw: %f\n", s->camera.pitch, s->camera.yaw);
  s->camera.front[0] =
      cos(glm_rad(s->camera.yaw)) * cos(glm_rad(s->camera.pitch));
  s->camera.front[1] = sin(glm_rad(s->camera.pitch));
  s->camera.front[2] =
      sin(glm_rad(s->camera.yaw)) * cos(glm_rad(s->camera.pitch));
  glm_normalize(s->camera.front);
  glm_cross(UP, s->camera.front, s->camera.right);
  glm_vec3_normalize(s->camera.right);
}

#define N_MODELS 2

float vertices[] = {
    -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
    0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

    -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
    0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
    0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

    -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f,
};

int main(void) {
  // === Init glfw and gl context ===
  GLFWwindow* w = initAndCreateWindow(W, H, WT);
  if (!w) goto clean;
  glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

  // === register callbacks ==
  // register callback to run  when screen resizes
  glfwSetFramebufferSizeCallback(w, onResizeScreen);
  glfwSetCursorPosCallback(w, mouseCallback);

  // === load 3d models ===
  Model model          = {0};
  int model_load_error = loadModelFromGltfFile("models/sun.glb", &model);
  if (model_load_error) {
    printf("could not load model, exiting\n");
    goto clean;
  }

  printf("n vertices: %d\n", model.meshes[0].n_vertices);
  printf("n triangles: %d\n", model.meshes[0].n_triangles);
  for (int i = 0; i < 20; i++) {
    unsigned int index = model.meshes[0].indices[i];
    float x            = model.meshes[0].vertices[index * 3 + 0];
    float y            = model.meshes[0].vertices[index * 3 + 1];
    float z            = model.meshes[0].vertices[index * 3 + 2];
    printf("index: %d, vertex: [%f,%f,%f]\n", index, x, y, z);
  }

  // === generate VAOs, VBOs etc... ===
  // GlIdentifier ids = {0};
  // genGlIds(&ids, model.n_meshes);
  // syncBuffers(model.meshes, &ids, model.n_meshes);

  // // === load textures ===
  // GLuint texture =
  //     loadTexture("container2.png", PNG); // TODO load texture form gltf file
  // if (!texture) {
  //   printf("could not load texture, exiting\n");
  //   goto clean;
  // }

  // === load, compile and link shaders ==
  GLuint shader = loadShader(SUN_VERT_SRC, SUN_FRAG_SRC);
  if (!shader) {
    printf("could not load shader, exiting\n");
    goto clean;
  }
  ShaderVars vars = loadShaderVars(shader);
  if (vars.model == -1 || vars.view == -1 || vars.projection == -1) {
    printf("could not load uniform variables\n");
    goto clean;
  }

  // === mvp setup begin ===
  mat4 m = GLM_MAT4_IDENTITY;
  mat4 v = GLM_MAT4_IDENTITY;
  mat4 p = GLM_MAT4_IDENTITY;

  // === game state setup begin ===
  GameState state = defaultGameState(W, H);

  unsigned int VAO, VBO[N_BUFFER_TYPES], EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(N_BUFFER_TYPES, VBO);
  glGenBuffers(1, &EBO);

  // bind object ids
  glBindVertexArray(VAO);
  // vertex
  glBindBuffer(GL_ARRAY_BUFFER, VBO[VERTEX]);
  glBufferData(
      GL_ARRAY_BUFFER,
      model.meshes[0].n_vertices * 3 * sizeof(float),
      model.meshes[0].vertices,
      GL_STATIC_DRAW
  );
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  //indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      model.meshes[0].n_triangles * 3 * sizeof(unsigned int),
      model.meshes[0].indices,
      GL_STATIC_DRAW
  );

  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    printf("OpenGL error: %d\n", err);
  }

  // === setup before Application loop ===
  glfwSetWindowUserPointer(w, &state);
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

  // === Application loop ==
  while (!glfwWindowShouldClose(w)) {
    // === update ===
    float time = glfwGetTime();
    updateFrameTime(&state.frame_t, time); // update frame time
    handleInput(w, &state);                // handle input
    cameraLookAt(&state.camera, v);        // update v
    glm_perspective(glm_rad(45.0), (float)W / (float)H, 0.1, 100.0, p);

    // === draw ===
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(VAO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO[VERTEX]);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glUseProgram(shader);

    glUniformMatrix4fv(vars.model, 1, false, (float*)m);
    glUniformMatrix4fv(vars.view, 1, false, (float*)v);
    glUniformMatrix4fv(vars.projection, 1, false, (float*)p);
    glDrawElements(
        GL_TRIANGLES, model.meshes[0].n_triangles * 3, GL_UNSIGNED_INT, 0
    );

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
