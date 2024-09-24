
#include "game_state.h"

#define UP ((vec3){0, 1, 0})

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

Cursor defaultCursorPos(int w, int h) {
  Cursor p = {.x = w / 2, .y = h / 2};
  return p;
}

void updateFrameTime(FrameTime* time, float now) {
  time->delta_time = now - time->last_frame;
  time->last_frame = now;
}

FrameTime defaultFrameTime(void) {
  FrameTime t = {
      .delta_time = 0.0f, // Time between current frame and last frame
      .last_frame = 0.0f, // Time of last frame
  };
  return t;
}

GameState defaultGameState(int w, int h) {
  GameState state = {
      .camera  = defaultCamera(),
      .cursor  = defaultCursorPos(w, h),
      .frame_t = defaultFrameTime(),
  };
  return state;
}
