#include "game/game_state.h"

void init_game(GameState *state)
{
    state->ball.point = (Point3d){0.0f, 1.0f, 0.0f};
    state->ball.velocity = (Point3d){0.0f, 0.0f, 0.0f};
    state->ball.angle = 0;
}