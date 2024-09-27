// system dependencies
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <cglm/cglm.h>
#include <stdbool.h>
#include <sys/stat.h>
// local dependencies
#include "shaders/shader.h"
#include "external/stb_image.h"
#include "textures/texture.h"
#include "models/model.h"
#include "gl_util.h"
#include "game_state.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_glfw_gl4.h"

int H    = 480;
int W    = 640;
char* WT = "Hello World";
vec3 UP  = {0, 1, 0};
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

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
  GLFWwindow* w = NULL;
  if (!glfwInit()) goto clean;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  w = glfwCreateWindow(W, H, WT, NULL, NULL);
  if (!w) goto clean;
  glfwMakeContextCurrent(w);
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) goto clean;
  glViewport(0, 0, W, H);
  glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

  // === register callbacks ==
  // register callback to run  when screen resizes
  glfwSetFramebufferSizeCallback(w, onResizeScreen);
  glfwSetCursorPosCallback(w, mouseCallback);
  glfwGetWindowSize(w, &W, &H);

  struct nk_context* ctx = nk_glfw3_init(
      w, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER
  );
  {
    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();
  }

  // === load 3d models ===
  Model model          = {0};
  int model_load_error = loadModelFromGltfFile("models/earth.glb", &model);
  if (model_load_error) {
    printf("could not load model, exiting\n");
    goto clean;
  }

  // printf("n vertices: %d\n", model.meshes[0].n_vertices);
  // printf("n triangles: %d\n", model.meshes[0].n_triangles);
  // for (int i = 0; i < 20; i++) {
  //   unsigned int index = model.meshes[0].indices[i];
  //   float x            = model.meshes[0].vertices[index * 3 + 0];
  //   float y            = model.meshes[0].vertices[index * 3 + 1];
  //   float z            = model.meshes[0].vertices[index * 3 + 2];
  //   printf("index: %d, vertex: [%f,%f,%f]\n", index, x, y, z);
  // }

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
  unsigned int texture;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(N_BUFFER_TYPES, VBO);
  glGenBuffers(1, &EBO);
  glGenTextures(1, &texture);

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
  // texcoord
  glBindBuffer(GL_ARRAY_BUFFER, VBO[TEXCOORD]);
  glBufferData(
      GL_ARRAY_BUFFER,
      model.meshes[0].n_vertices * 2 * sizeof(float),
      model.meshes[0].tex_coords,
      GL_STATIC_DRAW
  );
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(3);
  //indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      model.meshes[0].n_triangles * 3 * sizeof(unsigned int),
      model.meshes[0].indices,
      GL_STATIC_DRAW
  );
  ImageData* image = &model.materials[0].textures[BASE].image;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB,
      image->w,
      image->h,
      0,
      GL_RGB,
      GL_UNSIGNED_BYTE,
      image->data
  );
  glGenerateMipmap(GL_TEXTURE_2D);

  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    printf("OpenGL error: %d\n", err);
  }

  // === setup before Application loop ===
  glfwSetWindowUserPointer(w, &state);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  // === Application loop ==
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(w)) {
    // === update ===
    float time = glfwGetTime();
    updateFrameTime(&state.frame_t, time); // update frame time
    handleInput(w, &state);                // handle input
    cameraLookAt(&state.camera, v);        // update v
    glm_perspective(glm_rad(45.0), (float)W / (float)H, 0.1, 100.0, p);

    nk_glfw3_new_frame();
    if (nk_begin(
            ctx,
            "Nuklear Window",
            nk_rect(0, 0, 500, 500),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE |
                NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE
        )) {

      nk_end(ctx);
    }

    // === draw ===
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(VAO);
    glUseProgram(shader);

    glUniformMatrix4fv(vars.model, 1, false, (float*)m);
    glUniformMatrix4fv(vars.view, 1, false, (float*)v);
    glUniformMatrix4fv(vars.projection, 1, false, (float*)p);
    glDrawElements(
        GL_TRIANGLES, model.meshes[0].n_triangles * 3, GL_UNSIGNED_INT, 0
    );

    nk_glfw3_render(NK_ANTI_ALIASING_ON);

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
