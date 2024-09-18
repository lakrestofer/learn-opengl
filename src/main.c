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
#include "external/stb_image.h"
#include "textures/texture.h"

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

bool screenResized = false;
void onResizeScreen(GLFWwindow* _, int width, int height) {
  glViewport(0, 0, width, height);
  W             = width;
  H             = height;
  screenResized = true;
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
    {-1.3f,  1.0f, -1.5}
};
float vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};
const int stride =  (8 * sizeof(float));
const void* vertex_offset = (void*) 0;
const void* normal_offset = (void*) (3 * sizeof(float));
const void* texture_coord_offset = (void*) (6 * sizeof(float));
// does no transformation on the vertices
const char* CUBE_VSHADER = GLSL(
  layout (location = 0) in vec3 aPos;
  layout (location = 1) in vec3 aNormal;
  layout (location = 2) in vec2 aTexCoords;

  out vec3 FragPos;
  out vec3 Normal;
  out vec2 TexCoords;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 projection;

  void main() {
      FragPos = vec3(model * vec4(aPos, 1.0));
      Normal = mat3(transpose(inverse(model))) * aNormal;
      TexCoords = aTexCoords;
    
      gl_Position = projection * view * vec4(FragPos, 1.0);
  }
);
// colors each pixel within the triangle red
const char* CUBE_FSHADER = GLSL(
  struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shine;
  };

  struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
  };

  out vec4 FragColor;

  in vec2 TexCoords;
  in vec3 Normal;  
  in vec3 FragPos;  

  uniform Material material;
  uniform Light light;
  uniform vec3 viewPos;

  void main() {
    vec3 lightDir = normalize(light.position - FragPos);

    // cutoff
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // attenuation
    float light_constant = 1.2;
    float distance    = length(light.position - FragPos);
    float attenuation = light_constant * 1.0 / (light.constant + light.linear * distance +  light.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    // diffuse 
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));          

    diffuse *=  intensity;
    specular *=  intensity;
    FragColor = vec4(ambient + diffuse + specular, 1.0); 
	
  }
);
const char* LIGHT_VSHADER = GLSL(
  layout (location = 0) in vec3 aPos;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 projection;

  void main() {
  	gl_Position = projection * view * model * vec4(aPos, 1.0);
  }
);
const char* LIGHT_FSHADER = GLSL(
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
  GLuint light_vert_shader = initVShader(LIGHT_VSHADER);
  GLuint light_frag_shader = initFShader(LIGHT_FSHADER);
  if (!shaderIsValid(cube_vert_shader) || !shaderIsValid(cube_frag_shader))
    goto clean;
  GLuint c_shader     = linkShaders(cube_vert_shader, cube_frag_shader);
  GLuint light_shader = linkShaders(light_vert_shader, light_frag_shader);
  if (!shaderProgramIsValid(c_shader)) goto clean;
  glDeleteShader(cube_vert_shader);
  glDeleteShader(cube_frag_shader);
  glDeleteShader(light_frag_shader);

  // locations for uniform vars
  GLuint cube_model_loc              = shaderVar(c_shader, "model");
  GLuint cube_view_loc               = shaderVar(c_shader, "view");
  GLuint cube_proj_loc               = shaderVar(c_shader, "projection");
  GLuint cube_view_pos_loc           = shaderVar(c_shader, "viewPos");
  GLuint cube_material_diffuse_loc   = shaderVar(c_shader, "material.diffuse");
  GLuint cube_material_specular_loc  = shaderVar(c_shader, "material.specular");
  GLuint cube_material_shine_loc     = shaderVar(c_shader, "material.shine");
  GLuint cube_light_position_loc     = shaderVar(c_shader, "light.position");
  GLuint cube_light_direction_loc    = shaderVar(c_shader, "light.direction");
  GLuint cube_light_cutoff_loc       = shaderVar(c_shader, "light.cutOff");
  GLuint cube_light_outer_cutoff_loc = shaderVar(c_shader, "light.outerCutOff");
  GLuint cube_light_ambient_loc      = shaderVar(c_shader, "light.ambient");
  GLuint cube_light_diffuse_loc      = shaderVar(c_shader, "light.diffuse");
  GLuint cube_light_specular_loc     = shaderVar(c_shader, "light.specular");
  GLuint cube_light_constant_loc     = shaderVar(c_shader, "light.constant");
  GLuint cube_light_linear_loc       = shaderVar(c_shader, "light.linear");
  GLuint cube_light_quadratic_loc    = shaderVar(c_shader, "light.quadratic");
  GLuint light_model_loc             = shaderVar(light_shader, "model");
  GLuint light_view_loc              = shaderVar(light_shader, "view");
  GLuint light_proj_loc              = shaderVar(light_shader, "projection");

  // === load textures ===
  GLuint container_texture = createTexture("container2.png", PNG);
  GLuint container_texture_specular =
      createTexture("container2_specular.png", PNG);

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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, vertex_offset);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, normal_offset);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, texture_coord_offset);
  glEnableVertexAttribArray(2);
  // light cube
  GLuint light_VAO; // vertex array object
  glGenVertexArrays(1, &light_VAO);
  glBindVertexArray(light_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO); // ram -> vram
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, vertex_offset);
  glEnableVertexAttribArray(0);

  // == we setup global game state ===
  GameState state = defaultGameState();

  // we make game state available from everywhere
  glfwSetWindowUserPointer(w, &state);

  // === mvp setup begin ===

  mat4 cube_m    = GLM_MAT4_IDENTITY; // cube model
  mat4 light_m   = GLM_MAT4_IDENTITY; // light cube model
  mat4 v         = GLM_MAT4_IDENTITY; // view
  mat4 p         = GLM_MAT4_IDENTITY; // projection
  vec3 light_pos = {-0.2, -1.0, -0.3};
  glm_vec3_scale(light_pos, -10, light_pos);
  glm_translate(light_m, light_pos); // build m
  glm_mat4_scale(light_m, 0.1);      // build m

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

    // cube
    glUseProgram(c_shader);

    glUniformMatrix4fv(cube_view_loc, 1, GL_FALSE, (float*)v);
    glUniformMatrix4fv(cube_proj_loc, 1, GL_FALSE, (float*)p);
    glUniform3fv(cube_view_pos_loc, 1, state.camera.pos);
    glUniform1f(cube_material_diffuse_loc, 0);
    glUniform1f(cube_material_specular_loc, 1);
    glUniform1f(cube_material_shine_loc, 32);
    glUniform3fv(cube_light_position_loc, 1, state.camera.pos);
    glUniform3fv(cube_light_direction_loc, 1, state.camera.front);
    glUniform1f(cube_light_cutoff_loc, cos(glm_rad(12.5F)));
    glUniform1f(cube_light_outer_cutoff_loc, cos(glm_rad(17.5F)));
    glUniform3fv(cube_light_ambient_loc, 1, (vec3){0.2, 0.2, 0.2});
    glUniform3fv(cube_light_diffuse_loc, 1, (vec3){0.5, 0.5, 0.5});
    glUniform3fv(cube_light_specular_loc, 1, (vec3){1, 1, 1});
    glUniform1f(cube_light_constant_loc, 1.0f);
    glUniform1f(cube_light_linear_loc, 0.045f);
    glUniform1f(cube_light_quadratic_loc, 0.0075);
    glBindVertexArray(cube_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, container_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, container_texture_specular);

    for (int i = 0; i < 10; i++) {
      mat4 model_local = GLM_MAT4_IDENTITY;
      glm_translate(model_local, cubePositions[i]);
      float angle = 20.0f * i;
      glm_rotate(model_local, glm_rad(angle), (vec3){1.0f, 0.3f, 0.5f});
      glUniformMatrix4fv(cube_model_loc, 1, GL_FALSE, (float*)model_local);
      glDrawArrays(GL_TRIANGLES, 0, 36); // draw it
    }

    // light
    // glUseProgram(light_shader);
    // glUniformMatrix4fv(light_model_loc, 1, GL_FALSE, (float*)light_m);
    // glUniformMatrix4fv(light_view_loc, 1, GL_FALSE, (float*)v);
    // glUniformMatrix4fv(light_proj_loc, 1, GL_FALSE, (float*)p);
    // glBindVertexArray(light_VAO);
    // glDrawArrays(GL_TRIANGLES, 0, 36); // draw it

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
