#ifndef GS_HEADER_DEFINED
#define GS_HEADER_DEFINED

#include <cglm/cglm.h>

typedef struct {
  float yaw;
  float pitch;
  vec3 pos;
  vec3 front;
  vec3 right;
} Camera;

Camera defaultCamera(void);
void cameraLookAt(Camera* c, mat4 to);
void moveCameraForward(Camera* c, float speed);
void moveCameraBackward(Camera* c, float speed);

void moveCameraLeft(Camera* c, float speed);
void moveCameraRight(Camera* c, float speed);
void moveCameraUp(Camera* c, float speed);
void moveCameraDown(Camera* c, float speed);

typedef struct {
  float x;
  float y;
} Cursor;

Cursor defaultCursorPos(int w, int h);

typedef struct {
  float delta_time;
  float last_frame;
} FrameTime;

FrameTime defaultFrameTime(void);

void updateFrameTime(FrameTime* time, float now);

typedef struct {
  Camera camera;
  Cursor cursor;
  FrameTime frame_t;
} GameState;

GameState defaultGameState(int w, int h);


#endif // GS_HEADER_DEFINED
