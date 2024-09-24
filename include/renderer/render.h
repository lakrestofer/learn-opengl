#ifndef RENDER_HEADER_DEFINED
#define RENDER_HEADER_DEFINED

#include "gl_util.h"
#include "glad/gl.h"
#include "game_state.h"


// given the current gamestate, the id of an already prepared mesh, shader and texture
// activate them, and render them to the screen
void render(GameState* state, GLuint mesh , GLuint shader, GLuint texture);

#endif // RENDER_HEADER_DEFINED
